"""
C++ code generation for the WolfEngine asset converter.

All emit functions receive pre-normalized data:
  - unique_frames / seq are already deduplicated by the normalize phase
  - clips have already been validated (no empty frame ranges)
  - duration warnings have already been printed by the caller

No decisions, no mutations, no PIL — pure string formatting and file writing.
"""

import pathlib


# ---------------------------------------------------------------------------
# Formatting helpers
# ---------------------------------------------------------------------------

def _fmt_hex(v: int) -> str:
    return f"0x{v:04X}"


def _dur_comment(durations: list) -> str:
    if not durations:
        return ''
    if len(set(durations)) == 1:
        return f"  // {durations[0]}ms per frame"
    return f"  // per-frame ms: {durations}"


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
    lines.append(f"    {_fmt_hex(palette565[0])},  // 0 - transparent (reserved)")
    for i in range(1, 32):
        lines.append(f"    {_fmt_hex(palette565[i])},  // {i}")
    lines.append("};")
    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Sprite emitters (single-frame PNG)
# ---------------------------------------------------------------------------

def emit_auto_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                  indices_2d: list, palette565: list, w: int, h: int) -> str:
    pixel_block   = render_pixel_array(stem, indices_2d, w, h)
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
                   palette_name: str, palette_header: str) -> str:
    pixel_block  = render_pixel_array(stem, indices_2d, w, h)
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


# ---------------------------------------------------------------------------
# Animation emitters (GIF — single clip, pre-deduplicated)
# ---------------------------------------------------------------------------

def emit_auto_gif_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                      unique_frames: list, seq: list,
                      palette565: list, w: int, h: int) -> str:
    n_frames = len(seq)
    n_unique = len(unique_frames)

    palette_block = render_auto_palette_array(stem, palette565)
    frame_blocks  = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h) for i in range(n_unique)
    )
    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, s_{stem}_palette);"
        for i in range(n_unique)
    )
    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = {{ {sprites_inner} }};"
    seq_inner     = ', '.join(str(i) for i in seq) + ', 0xFF'
    seq_array     = f"static constexpr uint8_t s_{stem}_seq[] = {{ {seq_inner} }};"

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
                       unique_frames: list, seq: list,
                       w: int, h: int, palette_name: str, palette_header: str) -> str:
    n_frames = len(seq)
    n_unique = len(unique_frames)
    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"

    frame_blocks = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h) for i in range(n_unique)
    )
    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, {palette_name});"
        for i in range(n_unique)
    )
    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = {{ {sprites_inner} }};"
    seq_inner     = ', '.join(str(i) for i in seq) + ', 0xFF'
    seq_array     = f"static constexpr uint8_t s_{stem}_seq[] = {{ {seq_inner} }};"

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


# ---------------------------------------------------------------------------
# Animation emitters (ASE / JSON — multi-clip, pre-deduplicated)
# ---------------------------------------------------------------------------

def _build_clip_section(stem: str, clips: list, unique_frames: list,
                        global_seq: list, palette_ref: str) -> tuple:
    """
    Build shared sprite array + per-clip seq arrays + WE_AnimationRaw declarations.
    palette_ref: C++ expression — either 's_{stem}_palette' or a named palette constant.
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
        clip_sym_lower = clip['symbol'].lower()
        clip_seq  = [global_seq[fi] for fi in clip['frame_indices'] if fi < len(global_seq)]
        seq_inner = ', '.join(str(i) for i in clip_seq) + ', 0xFF'
        tag_label = f"// clip: \"{clip['clip_name']}\"" if clip['clip_name'] else f"// clip: {clip['symbol']}"
        dur       = _dur_comment(clip['durations_ms'])
        clip_lines.append(
            f"{tag_label}{dur}\n"
            f"static constexpr uint8_t s_{clip_sym_lower}_seq[] = {{ {seq_inner} }};\n"
            f"constexpr WE_AnimationRaw Assets::{clip['symbol']} = "
            f"{{ s_{stem}_sprites, s_{clip_sym_lower}_seq }};"
        )

    return sprite_block, sprites_array, '\n\n'.join(clip_lines)


def emit_auto_ase_cpp(output_dir: str, stem: str, clips: list,
                      unique_frames: list, global_seq: list,
                      palette565: list, w: int, h: int, source_rel: str) -> str:
    n_total  = len(global_seq)
    n_unique = len(unique_frames)
    clip_syms = ', '.join(c['symbol'] for c in clips)

    palette_block = render_auto_palette_array(stem, palette565)
    frame_blocks  = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h) for i in range(n_unique)
    )
    sprite_block, sprites_array, clips_block = _build_clip_section(
        stem, clips, unique_frames, global_seq, f"s_{stem}_palette"
    )
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


def emit_named_ase_cpp(output_dir: str, stem: str, clips: list,
                       unique_frames: list, global_seq: list,
                       w: int, h: int, source_rel: str,
                       palette_name: str, palette_header: str) -> str:
    n_total  = len(global_seq)
    n_unique = len(unique_frames)
    clip_syms    = ', '.join(c['symbol'] for c in clips)
    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"

    frame_blocks = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h) for i in range(n_unique)
    )
    sprite_block, sprites_array, clips_block = _build_clip_section(
        stem, clips, unique_frames, global_seq, palette_name
    )
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


# ---------------------------------------------------------------------------
# Header emitter
# ---------------------------------------------------------------------------

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
