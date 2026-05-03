"""
WolfEngine asset converter.
Usage: asset_converter.py <images_dir> <output_dir> <palettes_dir>

Scans <images_dir> for:
  *.png              → Sprite
  *.gif              → WE_AnimationRaw
  *.ase / *.aseprite → WE_AnimationRaw (one per tag, or one for whole file)
  *.json (+ same-dir PNG named in meta.image) → WE_AnimationRaw (Aseprite JSON export)

Converts each and writes .cpp + WE_Assets.hpp into <output_dir>.
"""

import sys
import pathlib
import struct
import zlib
import json as _json
import re

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib

from PIL import Image, ImageSequence


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


def fmt_hex(v: int) -> str:
    return f"0x{v:04X}"


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
      indices_2d    — list of rows, each row a list of uint8 palette indices
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
        if not opaque_rgb:
            palette_rgb = [(0, 0, 0)] * 31
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
# Image conversion — GIF
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
    Returns (all_indices_2d, palette565) where all_indices_2d is a list of
    per-frame indices_2d arrays.
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


def deduplicate_frames(all_indices_2d: list):
    """
    Returns (unique_frames, seq) where:
      unique_frames — list of unique indices_2d (one entry per distinct bitmap)
      seq           — per-original-frame index into unique_frames
    Comparison is byte-for-byte (full memcmp equivalent).
    """
    unique_frames = []
    unique_flat = []
    seq = []

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
# Aseprite binary parser (Path A: .ase / .aseprite)
# ---------------------------------------------------------------------------

def _ase_decode_cel(raw_bytes, w, h, color_depth, palette, transparent_index=None):
    """Convert raw cel pixel bytes to a PIL RGBA Image."""
    if color_depth == 32:
        return Image.frombytes('RGBA', (w, h), bytes(raw_bytes))
    if color_depth == 16:
        px = bytearray()
        for i in range(0, len(raw_bytes), 2):
            v, a = raw_bytes[i], raw_bytes[i + 1]
            px += bytes([v, v, v, a])
        return Image.frombytes('RGBA', (w, h), bytes(px))
    if color_depth == 8:
        # transparent_index is checked before palette lookup so a palette chunk
        # that later overwrites that slot cannot clobber transparency.
        px = bytearray()
        for idx in raw_bytes:
            if transparent_index is not None and idx == transparent_index:
                px += bytes([0, 0, 0, 0])
            else:
                r, g, b, a = palette[idx] if idx < len(palette) else (0, 0, 0, 0)
                px += bytes([r, g, b, a])
        return Image.frombytes('RGBA', (w, h), bytes(px))
    raise ValueError(f"Unsupported color depth {color_depth}")


def _ase_parse(path):
    """
    Parse an .ase/.aseprite binary file.
    Returns (width, height, frame_images, frame_durations_ms, tags) where:
      frame_images       — list of RGBA PIL Images (visible layers composited)
      frame_durations_ms — list of int (ms per frame)
      tags               — list of {'name': str, 'from': int, 'to': int}

    Targets the modern Aseprite format (1.3+). Handles RGBA, grayscale, and
    indexed color modes. Resolves linked cels correctly.
    """
    data = pathlib.Path(path).read_bytes()
    pos = 0

    def rd(fmt):
        nonlocal pos
        sz = struct.calcsize('<' + fmt)
        vals = struct.unpack_from('<' + fmt, data, pos)
        pos += sz
        return vals[0] if len(vals) == 1 else vals

    def rd_str():
        nonlocal pos
        n = rd('H')
        s = data[pos:pos + n].decode('utf-8', errors='replace')
        pos += n
        return s

    # ---- File header (128 bytes total) ----
    rd('I')                      # file size
    magic = rd('H')
    if magic != 0xA5E0:
        raise ValueError(f"Not an Aseprite file (magic=0x{magic:04X})")
    num_frames  = rd('H')
    width       = rd('H')
    height      = rd('H')
    color_depth = rd('H')        # 32=RGBA, 16=grayscale, 8=indexed
    pos += 4                     # flags (DWORD)
    pos += 2                     # speed, deprecated (WORD)
    pos += 8                     # two reserved DWORDs
    transparent_index = rd('B')  # transparent color index (indexed mode only)
    pos += 3                     # ignore (BYTE[3])
    pos += 2                     # num_colors (WORD)
    pos += 2                     # pixel_width, pixel_height (BYTE each)
    pos += 4                     # grid_x, grid_y (SHORT each)
    pos += 4                     # grid_width, grid_height (WORD each)
    pos += 84                    # future (BYTE[84])
    # pos == 128 here

    layers = []   # list of {visible, type, blend, opacity}
    tags   = []
    # RGBA palette for indexed mode. transparent_index is NOT pre-applied here
    # because palette chunks (0x2019) are processed later and would overwrite it.
    # Instead it is passed to _ase_decode_cel which checks it before palette lookup.
    palette = [(0, 0, 0, 255)] * 256

    frame_durations_ms = []
    # raw_cels[fi][li] = PIL Image | ('linked', target_fi)
    raw_cels = [{} for _ in range(num_frames)]
    raw_xy   = [{} for _ in range(num_frames)]

    for fi in range(num_frames):
        rd('I')                      # bytes_in_frame (unused; chunk_end used per-chunk)
        frame_magic = rd('H')
        if frame_magic != 0xF1FA:
            raise ValueError(f"Bad frame magic at frame {fi}")
        old_chunks  = rd('H')
        duration_ms = rd('H')
        pos += 2                 # reserved (BYTE[2])
        num_chunks  = rd('I') or old_chunks

        frame_durations_ms.append(duration_ms)

        for _ in range(num_chunks):
            chunk_start = pos
            chunk_size  = rd('I')
            chunk_type  = rd('H')
            chunk_end   = chunk_start + chunk_size

            if chunk_type == 0x2004:  # Layer
                layer_flags = rd('H')
                layer_type  = rd('H')
                pos += 2             # child_level (WORD)
                pos += 4             # default w, h (WORD each)
                blend   = rd('H')
                opacity = rd('B')
                pos += 3             # future (BYTE[3])
                rd_str()             # name
                layers.append({
                    'visible': bool(layer_flags & 1),
                    'type':    layer_type,   # 0=normal, 1=group, 2=tilemap
                    'blend':   blend,
                    'opacity': opacity,
                })

            elif chunk_type == 0x2005:  # Cel
                li      = rd('H')
                cx      = rd('h')
                cy      = rd('h')
                pos += 1             # cel opacity (BYTE)
                cel_type = rd('H')
                pos += 2             # z_index (SHORT)
                pos += 5             # future (BYTE[5])

                if cel_type == 0:    # raw uncompressed
                    cw, ch = rd('H'), rd('H')
                    bpp = color_depth // 8
                    raw = data[pos:pos + cw * ch * bpp]
                    pos += len(raw)
                    raw_cels[fi][li] = _ase_decode_cel(raw, cw, ch, color_depth, palette, transparent_index)
                    raw_xy[fi][li]   = (cx, cy)

                elif cel_type == 1:  # linked cel — uses another frame's cel for same layer
                    linked_fi = rd('H')
                    raw_cels[fi][li] = ('linked', linked_fi)
                    raw_xy[fi][li]   = (cx, cy)

                elif cel_type == 2:  # zlib-compressed image
                    cw, ch = rd('H'), rd('H')
                    raw = zlib.decompress(data[pos:chunk_end])
                    raw_cels[fi][li] = _ase_decode_cel(raw, cw, ch, color_depth, palette, transparent_index)
                    raw_xy[fi][li]   = (cx, cy)
                # cel_type 3 = compressed tilemap — not supported, falls through to chunk_end

            elif chunk_type == 0x2018:  # Tags
                num_tags = rd('H')
                pos += 8             # future (BYTE[8])
                for _ in range(num_tags):
                    from_f = rd('H')
                    to_f   = rd('H')
                    pos += 1         # loop direction (BYTE)
                    pos += 2         # repeat count (WORD, Aseprite 1.3+)
                    pos += 6         # future (BYTE[6])
                    pos += 3         # deprecated color (BYTE[3])
                    pos += 1         # extra zero (BYTE)
                    tag_name = rd_str()
                    tags.append({'name': tag_name, 'from': from_f, 'to': to_f})

            elif chunk_type == 0x2019:  # New palette chunk
                rd('I')              # new palette size (total entries)
                first = rd('I')
                last  = rd('I')
                pos += 8             # future (BYTE[8])
                for i in range(last - first + 1):
                    entry_flags = rd('H')
                    r, g, b, a = rd('B'), rd('B'), rd('B'), rd('B')
                    idx = first + i
                    if idx < 256:
                        palette[idx] = (r, g, b, a)
                    if entry_flags & 1:
                        rd_str()     # color name

            pos = chunk_end          # advance to exact chunk boundary (safety net)

    # ---- Resolve linked cels and composite visible layers ----
    def resolve_cel(fi, li, depth=0):
        """Follow linked cel chains; depth guards against cycles.
        Always returns the CURRENT frame's (x,y) position — only pixel data
        is borrowed from the linked frame."""
        if depth > num_frames:
            return None, (0, 0)
        cel = raw_cels[fi].get(li)
        if cel is None:
            return None, (0, 0)
        if isinstance(cel, tuple):
            _, linked_fi = cel
            img, _ = resolve_cel(linked_fi, li, depth + 1)
            return img, raw_xy[fi][li]   # position is always from the original frame
        return cel, raw_xy[fi][li]

    frame_images = []
    for fi in range(num_frames):
        canvas = Image.new('RGBA', (width, height), (0, 0, 0, 0))
        for li, layer in enumerate(layers):
            if not layer['visible'] or layer['type'] != 0:
                continue
            cel, (ox, oy) = resolve_cel(fi, li)
            if cel is None:
                continue
            sx = max(0, -ox);  sy = max(0, -oy)
            dx = max(0,  ox);  dy = max(0,  oy)
            sw = min(cel.width - sx, width  - dx)
            sh = min(cel.height - sy, height - dy)
            if sw > 0 and sh > 0:
                canvas.alpha_composite(cel, dest=(dx, dy),
                                       source=(sx, sy, sx + sw, sy + sh))
        frame_images.append(canvas)

    return width, height, frame_images, frame_durations_ms, tags


def extract_aseprite_frames(path):
    """
    Validate and parse a .ase/.aseprite file.
    Returns (frame_images, frame_durations_ms, tags) or None on error.
    """
    name = pathlib.Path(path).name
    try:
        width, height, frame_images, frame_durations_ms, tags = _ase_parse(path)
    except Exception as e:
        print(f"ERROR: Could not parse {name}: {e}")
        return None

    if not frame_images:
        print(f"ERROR: {name}: no frames found. Skipping.")
        return None

    if width > 96 or height > 96:
        print(f"ERROR: {name}: canvas {width}x{height} exceeds max dimension 96. Skipping.")
        return None

    return frame_images, frame_durations_ms, tags


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

    # Locate the spritesheet PNG via meta.image
    meta = meta_data.get('meta', {})
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

    frame_images = []
    frame_durations_ms = []
    for entry in frame_list:
        rect = entry['frame']
        x, y, fw, fh = rect['x'], rect['y'], rect['w'], rect['h']
        duration = entry.get('duration', 100)
        frame_images.append(sheet.crop((x, y, x + fw, y + fh)))
        frame_durations_ms.append(duration)

    # Validate frame sizes
    w0, h0 = frame_images[0].size
    for f in frame_images[1:]:
        if f.size != (w0, h0):
            print(f"ERROR: {name}: inconsistent frame sizes. Skipping.")
            return None
    if w0 > 96 or h0 > 96:
        print(f"ERROR: {name}: frame size {w0}x{h0} exceeds max dimension 96. Skipping.")
        return None

    # Tags from meta.frameTags
    tags = []
    for t in meta.get('frameTags', []):
        tags.append({'name': t['name'], 'from': t['from'], 'to': t['to']})

    return frame_images, frame_durations_ms, tags


# ---------------------------------------------------------------------------
# Clip helpers (shared by ASE and JSON paths)
# ---------------------------------------------------------------------------

def _make_clips(tags, num_frames, base_symbol, frame_durations_ms):
    """
    Convert Aseprite tags to a clip list.
    If no tags exist, returns a single clip covering all frames named after base_symbol.
    Each clip: {'symbol': str, 'clip_name': str, 'frame_indices': [int], 'durations_ms': [int]}
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
        tag_sym = re.sub(r'[^A-Z0-9_]', '',
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


def _dur_comment(durations):
    if not durations:
        return ''
    if len(set(durations)) == 1:
        return f"  // {durations[0]}ms per frame"
    return f"  // per-frame ms: {durations}"


# ---------------------------------------------------------------------------
# Code generation
# ---------------------------------------------------------------------------

def render_pixel_array(stem: str, indices_2d: list, w: int, h: int) -> str:
    lines = [f"static constexpr uint8_t s_{stem}_pixels[{h}][{w}] = {{"]
    for row in indices_2d:
        inner = ', '.join(str(v) for v in row) + ','
        lines.append(f"    {{ {inner} }},")
    lines.append("};")
    return '\n'.join(lines)


def render_frame_array(stem: str, frame_idx: int, indices_2d: list, w: int, h: int) -> str:
    lines = [f"static constexpr uint8_t s_{stem}_f{frame_idx}[{h}][{w}] = {{"]
    for row in indices_2d:
        inner = ', '.join(str(v) for v in row) + ','
        lines.append(f"    {{ {inner} }},")
    lines.append("};")
    return '\n'.join(lines)


def render_auto_palette_array(stem: str, palette565: list) -> str:
    lines = [f"static constexpr uint16_t s_{stem}_palette[32] = {{"]
    lines.append(f"    {fmt_hex(palette565[0])},  // 0 - transparent (reserved)")
    for i in range(1, 32):
        lines.append(f"    {fmt_hex(palette565[i])},  // {i}")
    lines.append("};")
    return '\n'.join(lines)


def emit_auto_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                  indices_2d: list, palette565: list, w: int, h: int):
    pixel_block = render_pixel_array(stem, indices_2d, w, h)
    palette_block = render_auto_palette_array(stem, palette565)

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"\n"
        f"{pixel_block}\n"
        f"\n"
        f"{palette_block}\n"
        f"\n"
        f"constexpr Sprite Assets::{symbol} = Sprite::Create(s_{stem}_pixels, s_{stem}_palette);\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_named_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                   indices_2d: list, w: int, h: int,
                   palette_name: str, palette_header: str):
    pixel_block = render_pixel_array(stem, indices_2d, w, h)
    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  palette: {palette_name}\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"#include \"{include_path}\"\n"
        f"\n"
        f"{pixel_block}\n"
        f"\n"
        f"constexpr Sprite Assets::{symbol} = Sprite::Create(s_{stem}_pixels, {palette_name});\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_auto_gif_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                      all_indices_2d: list, palette565: list, w: int, h: int):
    n_frames = len(all_indices_2d)
    unique_frames, seq = deduplicate_frames(all_indices_2d)
    n_unique = len(unique_frames)

    palette_block = render_auto_palette_array(stem, palette565)
    frame_blocks = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h)
        for i in range(n_unique)
    )

    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, s_{stem}_palette);"
        for i in range(n_unique)
    )

    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = {{ {sprites_inner} }};"

    seq_inner = ', '.join(str(i) for i in seq) + ', 0xFF'
    seq_array = f"static constexpr uint8_t s_{stem}_seq[] = {{ {seq_inner} }};"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  {n_frames} frames  {n_unique} unique\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"\n"
        f"{palette_block}\n"
        f"\n"
        f"{frame_blocks}\n"
        f"\n"
        f"{sprite_block}\n"
        f"\n"
        f"{sprites_array}\n"
        f"{seq_array}\n"
        f"\n"
        f"constexpr WE_AnimationRaw Assets::{symbol} = {{ s_{stem}_sprites, s_{stem}_seq }};\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_named_gif_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                       all_indices_2d: list, w: int, h: int,
                       palette_name: str, palette_header: str):
    n_frames = len(all_indices_2d)
    unique_frames, seq = deduplicate_frames(all_indices_2d)
    n_unique = len(unique_frames)

    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"
    frame_blocks = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h)
        for i in range(n_unique)
    )

    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, {palette_name});"
        for i in range(n_unique)
    )

    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = {{ {sprites_inner} }};"

    seq_inner = ', '.join(str(i) for i in seq) + ', 0xFF'
    seq_array = f"static constexpr uint8_t s_{stem}_seq[] = {{ {seq_inner} }};"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  {n_frames} frames  {n_unique} unique  palette: {palette_name}\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"#include \"{include_path}\"\n"
        f"\n"
        f"{frame_blocks}\n"
        f"\n"
        f"{sprite_block}\n"
        f"\n"
        f"{sprites_array}\n"
        f"{seq_array}\n"
        f"\n"
        f"constexpr WE_AnimationRaw Assets::{symbol} = {{ s_{stem}_sprites, s_{stem}_seq }};\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def _build_clip_section(stem, clips, unique_frames, global_seq, palette_ref, source_rel):
    """
    Build the shared sprites array + per-clip seq arrays + WE_AnimationRaw declarations.
    palette_ref: either 's_{stem}_palette' (auto) or the named palette constant.
    Returns (sprite_block, sprites_array, clips_block).
    """
    n_unique = len(unique_frames)

    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, {palette_ref});"
        for i in range(n_unique)
    )

    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = (f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = "
                     f"{{ {sprites_inner} }};")

    clip_lines = []
    for clip in clips:
        _warn_nonuniform_durations(clip, source_rel)
        clip_sym_lower = clip['symbol'].lower()
        clip_seq = [global_seq[fi] for fi in clip['frame_indices'] if fi < len(global_seq)]
        seq_inner = ', '.join(str(i) for i in clip_seq) + ', 0xFF'
        tag_label = f"// clip: \"{clip['clip_name']}\"" if clip['clip_name'] else f"// clip: {clip['symbol']}"
        dur = _dur_comment(clip['durations_ms'])
        clip_lines.append(
            f"{tag_label}{dur}\n"
            f"static constexpr uint8_t s_{clip_sym_lower}_seq[] = {{ {seq_inner} }};\n"
            f"constexpr WE_AnimationRaw Assets::{clip['symbol']} = "
            f"{{ s_{stem}_sprites, s_{clip_sym_lower}_seq }};"
        )

    return sprite_block, sprites_array, '\n\n'.join(clip_lines)


def emit_auto_ase_cpp(output_dir: str, stem: str, clips: list, all_indices_2d: list,
                      palette565: list, w: int, h: int, source_rel: str):
    """Emit a .cpp for an Aseprite file with auto-quantized palette."""
    n_total = len(all_indices_2d)
    unique_frames, global_seq = deduplicate_frames(all_indices_2d)
    n_unique = len(unique_frames)
    clip_syms = ', '.join(c['symbol'] for c in clips)

    palette_block = render_auto_palette_array(stem, palette565)
    frame_blocks  = '\n\n'.join(render_frame_array(stem, i, unique_frames[i], w, h)
                                for i in range(n_unique))
    sprite_block, sprites_array, clips_block = _build_clip_section(
        stem, clips, unique_frames, global_seq, f"s_{stem}_palette", source_rel)

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  {n_total} frames  {n_unique} unique  clips: {clip_syms}\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"\n"
        f"{palette_block}\n"
        f"\n"
        f"{frame_blocks}\n"
        f"\n"
        f"{sprite_block}\n"
        f"\n"
        f"{sprites_array}\n"
        f"\n"
        f"{clips_block}\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_named_ase_cpp(output_dir: str, stem: str, clips: list, all_indices_2d: list,
                       w: int, h: int, source_rel: str,
                       palette_name: str, palette_header: str):
    """Emit a .cpp for an Aseprite file with a named palette."""
    n_total = len(all_indices_2d)
    unique_frames, global_seq = deduplicate_frames(all_indices_2d)
    n_unique = len(unique_frames)
    clip_syms = ', '.join(c['symbol'] for c in clips)
    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"

    frame_blocks = '\n\n'.join(render_frame_array(stem, i, unique_frames[i], w, h)
                               for i in range(n_unique))
    sprite_block, sprites_array, clips_block = _build_clip_section(
        stem, clips, unique_frames, global_seq, palette_name, source_rel)

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  {n_total} frames  {n_unique} unique  "
        f"clips: {clip_syms}  palette: {palette_name}\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"#include \"{include_path}\"\n"
        f"\n"
        f"{frame_blocks}\n"
        f"\n"
        f"{sprite_block}\n"
        f"\n"
        f"{sprites_array}\n"
        f"\n"
        f"{clips_block}\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_assets_header(output_dir: str, sprite_symbols: list, anim_symbols: list):
    lines = [
        "// AUTO-GENERATED — do not edit",
        "#pragma once",
        "#include \"WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp\"",
        "#include \"WolfEngine/Graphics/AnimationSystem/WE_Animation.hpp\"",
        "",
        "namespace Assets {",
    ]
    for sym in sprite_symbols:
        lines.append(f"    extern const Sprite {sym};")
    for sym in anim_symbols:
        lines.append(f"    extern const WE_AnimationRaw {sym};")
    lines.append("}")
    lines.append("")

    out_path = pathlib.Path(output_dir).parent / "WE_Assets.hpp"
    out_path.write_text('\n'.join(lines), encoding='utf-8')


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def _process_ase_or_json(path, images_path, output_dir, palettes_dir, asset_config,
                         conflicts, anim_symbols, written_cpps, extract_fn):
    """
    Shared processing logic for .ase/.aseprite and .json Aseprite inputs.
    extract_fn(path) → (frame_images, frame_durations_ms, tags) or None
    file_kind: short string for log messages, e.g. 'ase' or 'json'
    Returns number of clips emitted (0 on error/skip).
    """
    base_symbol = name_to_symbol(path, images_path)

    if base_symbol in conflicts:
        return 0

    stem = base_symbol.lower()
    relative = pathlib.Path(path).relative_to(images_path)
    source_rel = str(relative).replace("\\", "/")
    config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

    result = extract_fn(path)
    if result is None:
        return 0
    frame_images, frame_durations_ms, tags = result

    w, h = frame_images[0].size
    n = len(frame_images)
    clips = _make_clips(tags, n, base_symbol, frame_durations_ms)

    cfg = asset_config.get(config_key, {})
    palette_key = cfg.get('palette') if cfg else None

    if palette_key:
        palette_data = load_named_palette(palettes_dir, palette_key)
        if palette_data is None:
            available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
            print(f"ERROR: Palette '{palette_key}' not found for {pathlib.Path(path).name}. "
                  f"Available: {available}. Skipping.")
            return 0
        palette_rgb   = palette_rgb_from_toml(palette_data)
        palette_name  = palette_data['meta']['name']
        palette_header = palette_data['meta']['header']
        all_indices_2d = gif_named_palette_convert(frame_images, palette_rgb)
        cpp_path = emit_named_ase_cpp(output_dir, stem, clips, all_indices_2d,
                                      w, h, source_rel, palette_name, palette_header)
    else:
        all_indices_2d, palette565 = gif_auto_palette_convert(frame_images)
        cpp_path = emit_auto_ase_cpp(output_dir, stem, clips, all_indices_2d,
                                     palette565, w, h, source_rel)

    written_cpps.add(pathlib.Path(cpp_path))
    for clip in clips:
        anim_symbols.append(clip['symbol'])

    clip_names = ', '.join(c['symbol'] for c in clips)
    print(f"  Converted: {source_rel}  [{w}W x {h}H]  {n} frames  →  {stem}.cpp  [{clip_names}]")
    return len(clips)


def main(images_dir: str, output_dir: str, palettes_dir: str):
    images_path = pathlib.Path(images_dir)
    output_path = pathlib.Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # --- Load assets.toml ---
    assets_toml_path = images_path / 'assets.toml'
    asset_config = {}
    if assets_toml_path.exists():
        asset_config = _load_toml(assets_toml_path)

    # --- Collect all input files (sorted for deterministic output) ---
    png_files = sorted(images_path.rglob('*.png'), key=lambda p: str(p).lower())
    gif_files = sorted(images_path.rglob('*.gif'), key=lambda p: str(p).lower())
    ase_files = sorted(
        list(images_path.rglob('*.ase')) + list(images_path.rglob('*.aseprite')),
        key=lambda p: str(p).lower()
    )
    # JSON files: only those that reference a meta.image (checked during processing)
    json_files = sorted(images_path.rglob('*.json'), key=lambda p: str(p).lower())

    all_asset_keys = {
        "_".join(p.relative_to(images_path).with_suffix("").parts).lower().replace("-", "_")
        for p in png_files + gif_files + ase_files + json_files
    }

    # --- Validate assets.toml keys ---
    for key in asset_config:
        if key.lower() not in all_asset_keys:
            print(f"WARNING: assets.toml entry '[{key}]' has no matching asset — skipping.")

    # --- Track existing .cpp files for stale cleanup ---
    existing_cpps = {p for p in output_path.glob('*.cpp')}
    written_cpps = set()

    # --- Detect symbol conflicts across all file types ---
    # For ASE/JSON we register the base file symbol (e.g. PLAYER) to catch
    # file-level duplicates (player.ase vs player.gif etc.).
    symbol_map = {}
    conflicts = set()
    for path in png_files + gif_files + ase_files + json_files:
        sym = name_to_symbol(path, images_path)
        if sym in symbol_map:
            if sym not in conflicts:
                print(f"ERROR: Symbol conflict — Assets::{sym} would be produced by both:\n"
                      f"  {symbol_map[sym]}\n  {path}\n"
                      f"Skipping both. Rename one of these files.")
            conflicts.add(sym)
        else:
            symbol_map[sym] = path

    converted = 0
    skipped   = 0
    errors    = 0
    sprite_symbols = []
    anim_symbols   = []

    # --- Process PNGs → Sprite ---
    for png_path in png_files:
        symbol = name_to_symbol(png_path, images_path)

        if symbol in conflicts:
            errors += 1
            continue

        stem = symbol.lower()
        relative = png_path.relative_to(images_path)
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

        cfg = asset_config.get(config_key, {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            palette_data = load_named_palette(palettes_dir, palette_key)
            if palette_data is None:
                available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
                print(f"ERROR: Palette '{palette_key}' not found for {png_path.name}. "
                      f"Available palettes: {available}. Skipping.")
                errors += 1
                continue

            palette_rgb = palette_rgb_from_toml(palette_data)
            palette_name = palette_data['meta']['name']
            palette_header = palette_data['meta']['header']
            indices_2d = named_palette_convert(img, palette_rgb)
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

        stem = symbol.lower()
        relative = gif_path.relative_to(images_path)
        source_rel = str(relative).replace("\\", "/")
        config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

        frames = extract_gif_frames(gif_path)
        if frames is None:
            errors += 1
            continue

        w, h = frames[0].size
        n = len(frames)

        cfg = asset_config.get(config_key, {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            palette_data = load_named_palette(palettes_dir, palette_key)
            if palette_data is None:
                available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
                print(f"ERROR: Palette '{palette_key}' not found for {gif_path.name}. "
                      f"Available palettes: {available}. Skipping.")
                errors += 1
                continue

            palette_rgb = palette_rgb_from_toml(palette_data)
            palette_name = palette_data['meta']['name']
            palette_header = palette_data['meta']['header']
            all_indices_2d = gif_named_palette_convert(frames, palette_rgb)
            cpp_path = emit_named_gif_cpp(
                output_dir, stem, symbol, source_rel, all_indices_2d, w, h, palette_name, palette_header
            )
        else:
            all_indices_2d, palette565 = gif_auto_palette_convert(frames)
            cpp_path = emit_auto_gif_cpp(
                output_dir, stem, symbol, source_rel, all_indices_2d, palette565, w, h
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
        # Quick pre-check: skip if the file doesn't look like an Aseprite JSON export
        try:
            probe = _json.loads(pathlib.Path(json_path).read_text(encoding='utf-8'))
            if 'frames' not in probe or 'meta' not in probe:
                skipped += 1
                continue
        except Exception:
            skipped += 1
            continue

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

    # --- Emit WE_Assets.hpp ---
    emit_assets_header(output_dir, sprite_symbols, anim_symbols)

    # --- Summary ---
    print(f"\nAsset conversion complete: {converted} converted, {skipped} skipped, {errors} errors.")
    print(f"Generated: WE_Assets.hpp")


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <images_dir> <output_dir> <palettes_dir>")
        sys.exit(1)

    images_dir, output_dir, palettes_dir = sys.argv[1], sys.argv[2], sys.argv[3]
    main(images_dir, output_dir, palettes_dir)
