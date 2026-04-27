"""
WolfEngine asset converter.
Usage: asset_converter.py <images_dir> <output_dir> <palettes_dir>

Scans <images_dir> for PNG files, converts each to a constexpr Sprite
definition, and writes .cpp + WE_Assets.hpp into <output_dir>.
"""

import sys
import os
import pathlib

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib

from PIL import Image


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


def name_to_symbol(filename: str) -> str:
    """'player_idle.png'  →  'PLAYER_IDLE'"""
    return pathlib.Path(filename).stem.upper().replace('-', '_')


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
# Image conversion
# ---------------------------------------------------------------------------

def auto_palette_convert(img_rgba: Image.Image):
    """
    Auto-quantize to at most 31 colors.
    Returns (indices_2d, palette565_32) where:
      indices_2d   — list of rows, each row a list of uint8 palette indices
      palette565_32 — list of 32 uint16 RGB565 values (index 0 always 0x0000)
    """
    w, h = img_rgba.size
    w_, h_ = img_rgba.size
    pix_ = img_rgba.load()
    pixels = [pix_[x, y] for y in range(h_) for x in range(w_)]

    opaque_rgb = [(r, g, b) for r, g, b, a in pixels if a >= 128]

    # Deduplicate while preserving insertion order
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
    w_, h_ = img_rgba.size
    pix_ = img_rgba.load()
    pixels = [pix_[x, y] for y in range(h_) for x in range(w_)]

    indices_2d = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = pixels[y * w + x]
            row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
        indices_2d.append(row)

    return indices_2d


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


def render_auto_palette_array(stem: str, palette565: list) -> str:
    lines = [f"static constexpr uint16_t s_{stem}_palette[32] = {{"]
    lines.append(f"    {fmt_hex(palette565[0])},  // 0 - transparent (reserved)")
    for i in range(1, 32):
        lines.append(f"    {fmt_hex(palette565[i])},  // {i}")
    lines.append("};")
    return '\n'.join(lines)


def emit_auto_cpp(output_dir: str, stem: str, symbol: str,
                  indices_2d: list, palette565: list, w: int, h: int):
    pixel_block = render_pixel_array(stem, indices_2d, w, h)
    palette_block = render_auto_palette_array(stem, palette565)

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {stem}.png  [{w}W x {h}H]\n"
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


def emit_named_cpp(output_dir: str, stem: str, symbol: str,
                   indices_2d: list, w: int, h: int,
                   palette_name: str, palette_header: str):
    pixel_block = render_pixel_array(stem, indices_2d, w, h)
    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {stem}.png  [{w}W x {h}H]  palette: {palette_name}\n"
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


def emit_assets_header(output_dir: str, symbols: list):
    lines = [
        "// AUTO-GENERATED — do not edit",
        "#pragma once",
        "#include \"WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp\"",
        "",
        "namespace Assets {",
    ]
    for sym in symbols:
        lines.append(f"    extern const Sprite {sym};")
    lines.append("}")
    lines.append("")

    out_path = pathlib.Path(output_dir) / "WE_Assets.hpp"
    out_path.write_text('\n'.join(lines), encoding='utf-8')


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

    # --- Collect PNGs (sorted for deterministic output) ---
    png_files = sorted(images_path.glob('*.png'), key=lambda p: p.name.lower())
    png_stems = {p.stem.lower() for p in png_files}

    # --- Validate assets.toml keys ---
    for key in asset_config:
        if key.lower() not in png_stems:
            print(f"WARNING: assets.toml entry '[{key}]' has no matching {key}.png — skipping.")

    # --- Track existing .cpp files for stale cleanup ---
    existing_cpps = {p for p in output_path.glob('*.cpp')}
    written_cpps = set()

    converted = 0
    skipped = 0
    errors = 0
    symbols = []

    for png_path in png_files:
        stem = png_path.stem
        symbol = name_to_symbol(png_path.name)

        try:
            img = Image.open(png_path).convert('RGBA')
        except Exception as e:
            print(f"ERROR: Could not open {png_path.name}: {e}")
            errors += 1
            continue

        w, h = img.size

        if w > 96 or h > 96:
            print(f"ERROR: {png_path.name} is {w}x{h}, max dimension is 96. Skipping.")
            errors += 1
            continue

        # Look up config by stem (case-insensitive match against toml keys)
        cfg = next((v for k, v in asset_config.items() if k.lower() == stem.lower()), {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            # Named palette path
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
                output_dir, stem, symbol, indices_2d, w, h, palette_name, palette_header
            )
        else:
            # Auto-palette path
            indices_2d, palette565 = auto_palette_convert(img)
            cpp_path = emit_auto_cpp(
                output_dir, stem, symbol, indices_2d, palette565, w, h
            )

        written_cpps.add(pathlib.Path(cpp_path))
        symbols.append(symbol)
        converted += 1
        print(f"  Converted: {png_path.name}  [{w}W x {h}H]  →  {stem}.cpp")

    # --- Clean up stale .cpp files ---
    for stale in existing_cpps - written_cpps:
        stale.unlink()
        print(f"  Removed stale: {stale.name}")

    # --- Emit WE_Assets.hpp ---
    emit_assets_header(output_dir, symbols)

    # --- Summary ---
    print(f"\nAsset conversion complete: {converted} converted, {skipped} skipped, {errors} errors.")
    print(f"Generated: WE_Assets.hpp")


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <images_dir> <output_dir> <palettes_dir>")
        sys.exit(1)

    images_dir, output_dir, palettes_dir = sys.argv[1], sys.argv[2], sys.argv[3]
    main(images_dir, output_dir, palettes_dir)
