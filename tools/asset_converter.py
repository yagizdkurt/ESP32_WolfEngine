"""
WolfEngine asset converter.
Usage: asset_converter.py <images_dir> <output_dir> <palettes_dir>

Scans <images_dir> for:
  *.png              → Sprite
  *.gif              → WE_AnimationRaw
  *.ase / *.aseprite → WE_AnimationRaw (one per tag, or one for whole file)
  *.json (+ PNG named in meta.image) → WE_AnimationRaw (Aseprite JSON export)

Pipeline
--------
  Parse     — aseprite_parser.py  (ASE binary decode)
            — PIL                 (PNG / GIF / JSON-sheet decode)
  Normalize — this file           (dedup, clip validation, palette quantize)
  Generate  — codegen.py          (pure C++ formatting, no decisions)

Converts each and writes .cpp + WE_Assets.hpp into <output_dir>.
"""

import sys
import pathlib
import json as _json
import re

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib

from PIL import Image, ImageSequence

from aseprite_parser import extract_aseprite_frames
from codegen import (
    emit_auto_cpp, emit_named_cpp,
    emit_auto_gif_cpp, emit_named_gif_cpp,
    emit_auto_ase_cpp, emit_named_ase_cpp,
    emit_assets_header,
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def nearest_idx(r: int, g: int, b: int, palette_rgb: list) -> int:
    """Return 1-based palette index of the nearest color in palette_rgb (31 entries)."""
    best_i, best_d = 0, float('inf')
    for i, (pr, pg, pb) in enumerate(palette_rgb):
        d = (r - pr) ** 2 + (g - pg) ** 2 + (b - pb) ** 2
        if d < best_d:
            best_d, best_i = d, i
    return best_i + 1  # 1-based


def name_to_symbol(image_path, images_dir) -> str:
    relative = pathlib.Path(image_path).relative_to(images_dir)
    parts = relative.with_suffix("").parts
    return "_".join(parts).upper().replace("-", "_")


# ---------------------------------------------------------------------------
# Palette loading
# ---------------------------------------------------------------------------

def _load_toml(path) -> dict:
    """Load a TOML file, transparently stripping a UTF-8 BOM if present."""
    raw = pathlib.Path(path).read_bytes()
    if raw.startswith(b'\xef\xbb\xbf'):
        raw = raw[3:]
    return tomllib.loads(raw.decode('utf-8'))


def load_named_palette(palettes_dir: str, palette_name: str):
    """
    Scan all .toml files in palettes_dir for one whose meta.name matches.
    Returns the parsed TOML dict or None if not found.
    """
    for p in pathlib.Path(palettes_dir).glob('*.toml'):
        data = _load_toml(p)
        if data.get('meta', {}).get('name') == palette_name:
            return data
    return None


def palette_rgb_from_toml(data: dict) -> list:
    """
    Extract 31-entry list of (r,g,b) tuples from a palette TOML dict.
    Index 0 of the returned list corresponds to palette slot 1.
    Missing slots are filled with (0, 0, 0).
    """
    palette_rgb = [(0, 0, 0)] * 31
    for c in sorted(data.get('color', []), key=lambda x: x['index']):
        i = c['index']
        if 1 <= i <= 31:
            palette_rgb[i - 1] = tuple(c['rgb'])
    return palette_rgb


# ---------------------------------------------------------------------------
# Image conversion — PNG
# ---------------------------------------------------------------------------

def auto_palette_convert(img_rgba: Image.Image):
    """
    Auto-quantize to at most 31 colors.
    Returns (indices_2d, palette565_32) where:
      indices_2d    — list of rows, each a list of uint8 palette indices
      palette565_32 — list of 32 uint16 RGB565 values (index 0 always 0x0000)
    """
    w, h = img_rgba.size
    pix_ = img_rgba.load()
    pixels = [pix_[x, y] for y in range(h) for x in range(w)]

    opaque_rgb = [(r, g, b) for r, g, b, a in pixels if a >= 128]

    seen = {}
    for c in opaque_rgb:
        if c not in seen:
            seen[c] = len(seen)
    unique = list(seen.keys())

    if len(unique) <= 31:
        palette_rgb = unique + [(0, 0, 0)] * (31 - len(unique))
    else:
        temp = Image.new('RGB', (len(opaque_rgb), 1))
        temp.putdata(opaque_rgb)
        q = temp.quantize(colors=31, dither=0)
        raw = q.getpalette()[:93]  # 31 * 3
        palette_rgb = [(raw[i], raw[i + 1], raw[i + 2]) for i in range(0, 93, 3)]

    palette565 = [0x0000] + [rgb888_to_rgb565(r, g, b) for r, g, b in palette_rgb]

    indices_2d = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = pixels[y * w + x]
            row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
        indices_2d.append(row)

    return indices_2d, palette565


def named_palette_convert(img_rgba: Image.Image, palette_rgb: list):
    """
    Map each pixel to the nearest color in palette_rgb (31 entries).
    Returns indices_2d only (the palette array is the named constant).
    """
    w, h = img_rgba.size
    pix_ = img_rgba.load()
    pixels = [pix_[x, y] for y in range(h) for x in range(w)]

    indices_2d = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = pixels[y * w + x]
            row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
        indices_2d.append(row)

    return indices_2d


# ---------------------------------------------------------------------------
# Image conversion — GIF / multi-frame
# ---------------------------------------------------------------------------

def extract_gif_frames(path):
    """
    Open a GIF and return a list of RGBA PIL Images, one per frame.
    Returns None and prints an error if the GIF fails validation.
    """
    name = pathlib.Path(path).name
    try:
        gif = Image.open(path)
    except Exception as e:
        print(f"ERROR: Could not open {name}: {e}")
        return None

    frames = [frame.copy().convert('RGBA') for frame in ImageSequence.Iterator(gif)]

    if len(frames) < 2:
        print(f"ERROR: {name} has only {len(frames)} frame — use a PNG for single sprites. Skipping.")
        return None

    w0, h0 = frames[0].size
    for f in frames[1:]:
        if f.size != (w0, h0):
            print(f"ERROR: {name} has inconsistent frame sizes — all frames must be the same dimensions. Skipping.")
            return None

    if w0 > 96 or h0 > 96:
        print(f"ERROR: {name} frame size {w0}x{h0} exceeds max dimension 96. Skipping.")
        return None

    return frames


def gif_auto_palette_convert(frames: list):
    """
    Build a shared palette from all frames combined, then map each frame to indices.
    Returns (all_indices_2d, palette565).
    """
    all_opaque = []
    for frame in frames:
        all_opaque += [(r, g, b) for r, g, b, a in frame.getdata() if a >= 128]

    if len(set(all_opaque)) <= 31:
        palette_rgb = list(dict.fromkeys(all_opaque))
        palette_rgb += [(0, 0, 0)] * (31 - len(palette_rgb))
    else:
        if not all_opaque:
            palette_rgb = [(0, 0, 0)] * 31
        else:
            temp = Image.new('RGB', (len(all_opaque), 1))
            temp.putdata(all_opaque)
            q = temp.quantize(colors=31, dither=0)
            raw = q.getpalette()[:93]
            palette_rgb = [(raw[i], raw[i + 1], raw[i + 2]) for i in range(0, 93, 3)]

    palette565 = [0x0000] + [rgb888_to_rgb565(*c) for c in palette_rgb]

    all_indices_2d = []
    for frame in frames:
        w, h = frame.size
        pixels = list(frame.getdata())
        indices_2d = []
        for y in range(h):
            row = []
            for x in range(w):
                r, g, b, a = pixels[y * w + x]
                row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
            indices_2d.append(row)
        all_indices_2d.append(indices_2d)

    return all_indices_2d, palette565


def gif_named_palette_convert(frames: list, palette_rgb: list):
    """
    Map each frame's pixels to the nearest color in palette_rgb.
    Returns all_indices_2d (list of per-frame indices_2d).
    """
    all_indices_2d = []
    for frame in frames:
        w, h = frame.size
        pixels = list(frame.getdata())
        indices_2d = []
        for y in range(h):
            row = []
            for x in range(w):
                r, g, b, a = pixels[y * w + x]
                row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
            indices_2d.append(row)
        all_indices_2d.append(indices_2d)
    return all_indices_2d


# ---------------------------------------------------------------------------
# Normalize — frame deduplication
# ---------------------------------------------------------------------------

def deduplicate_frames(all_indices_2d: list):
    """
    Returns (unique_frames, seq) where:
      unique_frames — list of unique indices_2d (one entry per distinct bitmap)
      seq           — per-original-frame index into unique_frames
    Comparison is byte-for-byte (full memcmp equivalent).
    """
    unique_frames = []
    unique_flat   = []
    seq           = []

    for indices_2d in all_indices_2d:
        flat = bytes(pixel for row in indices_2d for pixel in row)
        found = -1
        for i, existing in enumerate(unique_flat):
            if flat == existing:
                found = i
                break
        if found >= 0:
            seq.append(found)
        else:
            seq.append(len(unique_frames))
            unique_frames.append(indices_2d)
            unique_flat.append(flat)

    return unique_frames, seq


# ---------------------------------------------------------------------------
# Aseprite JSON + PNG spritesheet parser (Path B)
# ---------------------------------------------------------------------------

def extract_json_sheet_frames(json_path):
    """
    Parse an Aseprite 'Export Sprite Sheet' JSON file.
    The companion PNG is located via meta.image (relative to the JSON file).
    Returns (frame_images, frame_durations_ms, tags) or None on error.
    """
    json_path = pathlib.Path(json_path)
    name = json_path.name

    try:
        meta_data = _json.loads(json_path.read_text(encoding='utf-8'))
    except Exception as e:
        print(f"ERROR: Could not parse {name}: {e}")
        return None

    meta      = meta_data.get('meta', {})
    image_rel = meta.get('image', '')
    if not image_rel:
        print(f"ERROR: {name}: 'meta.image' field missing. Skipping.")
        return None

    png_path = json_path.parent / image_rel
    if not png_path.exists():
        print(f"ERROR: {name}: spritesheet '{image_rel}' not found at {png_path}. Skipping.")
        return None

    try:
        sheet = Image.open(png_path).convert('RGBA')
    except Exception as e:
        print(f"ERROR: Could not open {png_path.name}: {e}")
        return None

    # frames can be a dict (keyed by filename) or an array
    raw_frames = meta_data.get('frames', {})
    if isinstance(raw_frames, dict):
        frame_list = list(raw_frames.values())
    elif isinstance(raw_frames, list):
        frame_list = raw_frames
    else:
        print(f"ERROR: {name}: unrecognized 'frames' format. Skipping.")
        return None

    if not frame_list:
        print(f"ERROR: {name}: no frames found. Skipping.")
        return None

    frame_images        = []
    frame_durations_ms  = []
    for entry in frame_list:
        rect = entry['frame']
        x, y, fw, fh = rect['x'], rect['y'], rect['w'], rect['h']
        frame_images.append(sheet.crop((x, y, x + fw, y + fh)))
        frame_durations_ms.append(entry.get('duration', 100))

    w0, h0 = frame_images[0].size
    for f in frame_images[1:]:
        if f.size != (w0, h0):
            print(f"ERROR: {name}: inconsistent frame sizes. Skipping.")
            return None
    if w0 > 96 or h0 > 96:
        print(f"ERROR: {name}: frame size {w0}x{h0} exceeds max dimension 96. Skipping.")
        return None

    tags = [{'name': t['name'], 'from': t['from'], 'to': t['to']}
            for t in meta.get('frameTags', [])]

    return frame_images, frame_durations_ms, tags


# ---------------------------------------------------------------------------
# Normalize — clip helpers
# ---------------------------------------------------------------------------

def _make_clips(tags, num_frames, base_symbol, frame_durations_ms):
    """
    Convert Aseprite tags to a clip list.
    If no tags exist, returns a single clip covering all frames.
    Each clip: {'symbol', 'clip_name', 'frame_indices', 'durations_ms'}
    """
    if not tags:
        return [{
            'symbol':        base_symbol,
            'clip_name':     '',
            'frame_indices': list(range(num_frames)),
            'durations_ms':  list(frame_durations_ms),
        }]

    clips = []
    for tag in tags:
        raw_name = tag['name']
        tag_sym  = re.sub(r'[^A-Z0-9_]', '',
                          raw_name.upper().replace(' ', '_').replace('-', '_'))
        if not tag_sym:
            tag_sym = f"CLIP{len(clips)}"
        fi_list = list(range(tag['from'], min(tag['to'] + 1, num_frames)))
        if not fi_list:
            print(f"  WARNING: tag '{raw_name}' has no valid frames "
                  f"(from={tag['from']}, to={tag['to']}, file has {num_frames} frames) — skipping clip.")
            continue
        durs = [frame_durations_ms[fi] for fi in fi_list]
        clips.append({
            'symbol':        f"{base_symbol}_{tag_sym}",
            'clip_name':     raw_name,
            'frame_indices': fi_list,
            'durations_ms':  durs,
        })
    return clips


def _warn_nonuniform_durations(clip, source_rel):
    durations = clip['durations_ms']
    if durations and len(set(durations)) > 1:
        label = clip['clip_name'] or clip['symbol']
        print(f"  WARNING: {source_rel} clip '{label}' has non-uniform frame durations "
              f"{durations} — WE_AnimationRaw has no per-frame slot; "
              f"set WE_Animation::frameDuration in game code.")


# ---------------------------------------------------------------------------
# Phase 1 — asset classification
# ---------------------------------------------------------------------------

def _classify_assets(images_path: pathlib.Path):
    """
    Scan images_path and partition all files into typed buckets.

    JSON exports are identified by having both 'frames' and 'meta' keys,
    plus a resolvable 'meta.image' PNG path. Their spritesheets are excluded
    from standalone PNG processing so they don't produce a duplicate Sprite asset.

    Returns:
      sprite_pngs   — PNG files that are standalone sprites
      gif_files     — GIF animation files
      ase_files     — .ase / .aseprite files
      json_files    — validated Aseprite JSON export files
      excluded_pngs — PNG files claimed as spritesheets (logged, not converted)
    """
    all_pngs  = sorted(images_path.rglob('*.png'),  key=lambda p: str(p).lower())
    gif_files = sorted(images_path.rglob('*.gif'),  key=lambda p: str(p).lower())
    ase_files = sorted(
        list(images_path.rglob('*.ase')) + list(images_path.rglob('*.aseprite')),
        key=lambda p: str(p).lower()
    )
    all_jsons = sorted(images_path.rglob('*.json'), key=lambda p: str(p).lower())

    json_files        = []
    claimed_png_paths = set()   # resolved absolute paths

    for json_path in all_jsons:
        try:
            data = _json.loads(json_path.read_text(encoding='utf-8'))
        except Exception:
            continue
        if 'frames' not in data or 'meta' not in data:
            continue
        image_rel = data.get('meta', {}).get('image', '')
        if not image_rel:
            continue
        png_candidate = (json_path.parent / image_rel).resolve()
        json_files.append(json_path)
        if png_candidate.exists():
            claimed_png_paths.add(png_candidate)

    sprite_pngs   = []
    excluded_pngs = []
    for p in all_pngs:
        if p.resolve() in claimed_png_paths:
            excluded_pngs.append(p)
        else:
            sprite_pngs.append(p)

    return sprite_pngs, gif_files, ase_files, json_files, excluded_pngs


# ---------------------------------------------------------------------------
# Shared normalize + generate for ASE / JSON paths
# ---------------------------------------------------------------------------

def _process_ase_or_json(path, images_path, output_dir, palettes_dir, asset_config,
                         conflicts, anim_symbols, written_cpps, extract_fn):
    """
    Normalize and generate one .cpp for an ASE or JSON Aseprite source.
    extract_fn(path) → (frame_images, frame_durations_ms, tags) or None
    Returns the number of clips emitted (0 on error or conflict).
    """
    base_symbol = name_to_symbol(path, images_path)

    if base_symbol in conflicts:
        return 0

    stem       = base_symbol.lower()
    relative   = pathlib.Path(path).relative_to(images_path)
    source_rel = str(relative).replace("\\", "/")
    config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

    # --- Parse ---
    result = extract_fn(path)
    if result is None:
        return 0
    frame_images, frame_durations_ms, tags = result

    # --- Normalize ---
    w, h = frame_images[0].size
    n    = len(frame_images)
    clips = _make_clips(tags, n, base_symbol, frame_durations_ms)
    for clip in clips:
        _warn_nonuniform_durations(clip, source_rel)

    cfg         = asset_config.get(config_key, {})
    palette_key = cfg.get('palette') if cfg else None

    if palette_key:
        palette_data = load_named_palette(palettes_dir, palette_key)
        if palette_data is None:
            available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
            print(f"ERROR: Palette '{palette_key}' not found for {pathlib.Path(path).name}. "
                  f"Available: {available}. Skipping.")
            return 0
        palette_rgb    = palette_rgb_from_toml(palette_data)
        palette_name   = palette_data['meta']['name']
        palette_header = palette_data['meta']['header']
        all_indices_2d = gif_named_palette_convert(frame_images, palette_rgb)
        unique_frames, global_seq = deduplicate_frames(all_indices_2d)
        # --- Generate ---
        cpp_path = emit_named_ase_cpp(output_dir, stem, clips,
                                      unique_frames, global_seq,
                                      w, h, source_rel, palette_name, palette_header)
    else:
        all_indices_2d, palette565 = gif_auto_palette_convert(frame_images)
        unique_frames, global_seq  = deduplicate_frames(all_indices_2d)
        # --- Generate ---
        cpp_path = emit_auto_ase_cpp(output_dir, stem, clips,
                                     unique_frames, global_seq,
                                     palette565, w, h, source_rel)

    written_cpps.add(pathlib.Path(cpp_path))
    for clip in clips:
        anim_symbols.append(clip['symbol'])

    clip_names = ', '.join(c['symbol'] for c in clips)
    print(f"  Converted: {source_rel}  [{w}W x {h}H]  {n} frames  →  {stem}.cpp  [{clip_names}]")
    return len(clips)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(images_dir: str, output_dir: str, palettes_dir: str):
    images_path = pathlib.Path(images_dir)
    output_path = pathlib.Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # --- Load assets.toml ---
    assets_toml_path = images_path / 'assets.toml'
    asset_config = {}
    if assets_toml_path.exists():
        asset_config = _load_toml(assets_toml_path)

    # --- Phase 1: classify assets ---
    sprite_pngs, gif_files, ase_files, json_files, excluded_pngs = \
        _classify_assets(images_path)

    for p in excluded_pngs:
        rel = str(p.relative_to(images_path)).replace("\\", "/")
        print(f"  Skipping {rel} — claimed as spritesheet by a JSON export")

    # --- Validate assets.toml keys against classified assets ---
    all_asset_keys = {
        "_".join(p.relative_to(images_path).with_suffix("").parts).lower().replace("-", "_")
        for p in sprite_pngs + gif_files + ase_files + json_files
    }
    for key in asset_config:
        if key.lower() not in all_asset_keys:
            print(f"WARNING: assets.toml entry '[{key}]' has no matching asset — skipping.")

    # --- Track existing .cpp files for stale cleanup ---
    existing_cpps = {p for p in output_path.glob('*.cpp')}
    written_cpps  = set()

    # --- Detect symbol conflicts across all asset types ---
    symbol_map = {}
    conflicts  = set()
    for path in sprite_pngs + gif_files + ase_files + json_files:
        sym = name_to_symbol(path, images_path)
        if sym in symbol_map:
            if sym not in conflicts:
                print(f"ERROR: Symbol conflict — Assets::{sym} would be produced by both:\n"
                      f"  {symbol_map[sym]}\n  {path}\n"
                      f"Skipping both. Rename one of these files.")
            conflicts.add(sym)
        else:
            symbol_map[sym] = path

    converted      = 0
    skipped        = 0
    errors         = 0
    sprite_symbols = []
    anim_symbols   = []

    # --- Process PNGs → Sprite ---
    for png_path in sprite_pngs:
        symbol = name_to_symbol(png_path, images_path)

        if symbol in conflicts:
            errors += 1
            continue

        stem       = symbol.lower()
        relative   = png_path.relative_to(images_path)
        source_rel = str(relative).replace("\\", "/")
        config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

        try:
            img = Image.open(png_path).convert('RGBA')
        except Exception as e:
            print(f"ERROR: Could not open {source_rel}: {e}")
            errors += 1
            continue

        w, h = img.size
        if w > 96 or h > 96:
            print(f"ERROR: {source_rel} is {w}x{h}, max dimension is 96. Skipping.")
            errors += 1
            continue

        cfg         = asset_config.get(config_key, {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            palette_data = load_named_palette(palettes_dir, palette_key)
            if palette_data is None:
                available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
                print(f"ERROR: Palette '{palette_key}' not found for {png_path.name}. "
                      f"Available palettes: {available}. Skipping.")
                errors += 1
                continue
            palette_rgb    = palette_rgb_from_toml(palette_data)
            palette_name   = palette_data['meta']['name']
            palette_header = palette_data['meta']['header']
            indices_2d     = named_palette_convert(img, palette_rgb)
            cpp_path = emit_named_cpp(
                output_dir, stem, symbol, source_rel, indices_2d, w, h, palette_name, palette_header
            )
        else:
            indices_2d, palette565 = auto_palette_convert(img)
            cpp_path = emit_auto_cpp(
                output_dir, stem, symbol, source_rel, indices_2d, palette565, w, h
            )

        written_cpps.add(pathlib.Path(cpp_path))
        sprite_symbols.append(symbol)
        converted += 1
        print(f"  Converted: {source_rel}  [{w}W x {h}H]  →  {stem}.cpp")

    # --- Process GIFs → WE_AnimationRaw ---
    for gif_path in gif_files:
        symbol = name_to_symbol(gif_path, images_path)

        if symbol in conflicts:
            errors += 1
            continue

        stem       = symbol.lower()
        relative   = gif_path.relative_to(images_path)
        source_rel = str(relative).replace("\\", "/")
        config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

        frames = extract_gif_frames(gif_path)
        if frames is None:
            errors += 1
            continue

        w, h = frames[0].size
        n    = len(frames)
        cfg         = asset_config.get(config_key, {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            palette_data = load_named_palette(palettes_dir, palette_key)
            if palette_data is None:
                available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
                print(f"ERROR: Palette '{palette_key}' not found for {gif_path.name}. "
                      f"Available palettes: {available}. Skipping.")
                errors += 1
                continue
            palette_rgb    = palette_rgb_from_toml(palette_data)
            palette_name   = palette_data['meta']['name']
            palette_header = palette_data['meta']['header']
            all_indices_2d = gif_named_palette_convert(frames, palette_rgb)
            unique_frames, seq = deduplicate_frames(all_indices_2d)
            cpp_path = emit_named_gif_cpp(
                output_dir, stem, symbol, source_rel, unique_frames, seq, w, h, palette_name, palette_header
            )
        else:
            all_indices_2d, palette565 = gif_auto_palette_convert(frames)
            unique_frames, seq         = deduplicate_frames(all_indices_2d)
            cpp_path = emit_auto_gif_cpp(
                output_dir, stem, symbol, source_rel, unique_frames, seq, palette565, w, h
            )

        written_cpps.add(pathlib.Path(cpp_path))
        anim_symbols.append(symbol)
        converted += 1
        print(f"  Converted: {source_rel}  [{w}W x {h}H]  {n} frames  →  {stem}.cpp")

    # --- Process .ase / .aseprite → WE_AnimationRaw (one per tag) ---
    for ase_path in ase_files:
        n_clips = _process_ase_or_json(
            ase_path, images_path, output_dir, palettes_dir, asset_config,
            conflicts, anim_symbols, written_cpps, extract_aseprite_frames
        )
        if n_clips > 0:
            converted += 1
        else:
            errors += 1

    # --- Process Aseprite JSON exports → WE_AnimationRaw (one per tag) ---
    for json_path in json_files:
        n_clips = _process_ase_or_json(
            json_path, images_path, output_dir, palettes_dir, asset_config,
            conflicts, anim_symbols, written_cpps, extract_json_sheet_frames
        )
        if n_clips > 0:
            converted += 1
        else:
            errors += 1

    # --- Clean up stale .cpp files ---
    for stale in existing_cpps - written_cpps:
        stale.unlink()
        print(f"  Removed stale: {stale.name}")

    # --- Generate WE_Assets.hpp ---
    emit_assets_header(output_dir, sprite_symbols, anim_symbols)

    print(f"\nAsset conversion complete: {converted} converted, {skipped} skipped, {errors} errors.")
    print(f"Generated: WE_Assets.hpp")


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <images_dir> <output_dir> <palettes_dir>")
        sys.exit(1)

    images_dir, output_dir, palettes_dir = sys.argv[1], sys.argv[2], sys.argv[3]
    main(images_dir, output_dir, palettes_dir)
