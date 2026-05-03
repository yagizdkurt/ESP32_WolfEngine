"""
Aseprite binary file parser (.ase / .aseprite).

Public API
----------
extract_aseprite_frames(path)
    Returns (frame_images, frame_durations_ms, tags) or None on error.

Internal pipeline
-----------------
_ase_parse          — full binary parse → raw composited RGBA frames
_ase_decode_cel     — bytes → PIL RGBA for one cel (RGBA / grayscale / indexed)

Targets Aseprite format 1.3+. Handles RGBA (32-bit), grayscale (16-bit),
and indexed (8-bit) color modes. Resolves linked cels correctly.
"""

import pathlib
import struct
import zlib

from PIL import Image


def _ase_decode_cel(raw_bytes, w, h, color_depth, palette, transparent_index=None):
    """Convert raw cel pixel bytes to a PIL RGBA Image.

    transparent_index is checked before the palette lookup for indexed mode
    so a palette chunk that later overwrites that slot cannot clobber transparency.
    """
    if color_depth == 32:
        return Image.frombytes('RGBA', (w, h), bytes(raw_bytes))

    if color_depth == 16:
        px = bytearray()
        for i in range(0, len(raw_bytes), 2):
            v, a = raw_bytes[i], raw_bytes[i + 1]
            px += bytes([v, v, v, a])
        return Image.frombytes('RGBA', (w, h), bytes(px))

    if color_depth == 8:
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
    rd('I')                       # file size
    magic = rd('H')
    if magic != 0xA5E0:
        raise ValueError(f"Not an Aseprite file (magic=0x{magic:04X})")
    num_frames  = rd('H')
    width       = rd('H')
    height      = rd('H')
    color_depth = rd('H')         # 32=RGBA, 16=grayscale, 8=indexed
    pos += 4                      # flags (DWORD)
    pos += 2                      # speed, deprecated (WORD)
    pos += 8                      # two reserved DWORDs
    transparent_index = rd('B')   # transparent color index (indexed mode only)
    pos += 3                      # ignore (BYTE[3])
    pos += 2                      # num_colors (WORD)
    pos += 2                      # pixel_width, pixel_height (BYTE each)
    pos += 4                      # grid_x, grid_y (SHORT each)
    pos += 4                      # grid_width, grid_height (WORD each)
    pos += 84                     # future (BYTE[84])
    # pos == 128 here

    layers = []    # list of {visible, type, blend, opacity}
    tags   = []
    # transparent_index is NOT pre-applied here — palette chunks (0x2019) are
    # processed later and would overwrite any pre-set slot. Instead it is passed
    # to _ase_decode_cel which checks it before the palette lookup.
    palette = [(0, 0, 0, 255)] * 256

    frame_durations_ms = []
    # raw_cels[fi][li] = PIL Image | ('linked', target_fi)
    raw_cels = [{} for _ in range(num_frames)]
    raw_xy   = [{} for _ in range(num_frames)]

    for fi in range(num_frames):
        rd('I')                   # bytes_in_frame (unused; chunk_end used per-chunk)
        frame_magic = rd('H')
        if frame_magic != 0xF1FA:
            raise ValueError(f"Bad frame magic at frame {fi}")
        old_chunks  = rd('H')
        duration_ms = rd('H')
        pos += 2                  # reserved (BYTE[2])
        num_chunks  = rd('I') or old_chunks

        frame_durations_ms.append(duration_ms)

        for _ in range(num_chunks):
            chunk_start = pos
            chunk_size  = rd('I')
            chunk_type  = rd('H')
            chunk_end   = chunk_start + chunk_size

            if chunk_type == 0x2004:   # Layer
                layer_flags = rd('H')
                layer_type  = rd('H')
                pos += 2               # child_level (WORD)
                pos += 4               # default w, h (WORD each)
                blend   = rd('H')
                opacity = rd('B')
                pos += 3               # future (BYTE[3])
                rd_str()               # name
                layers.append({
                    'visible': bool(layer_flags & 1),
                    'type':    layer_type,   # 0=normal, 1=group, 2=tilemap
                    'blend':   blend,
                    'opacity': opacity,
                })

            elif chunk_type == 0x2005:  # Cel
                li       = rd('H')
                cx       = rd('h')
                cy       = rd('h')
                pos += 1               # cel opacity (BYTE)
                cel_type = rd('H')
                pos += 2               # z_index (SHORT, Aseprite 1.3+)
                pos += 5               # future (BYTE[5])

                if cel_type == 0:      # raw uncompressed
                    cw, ch = rd('H'), rd('H')
                    bpp = color_depth // 8
                    raw = data[pos:pos + cw * ch * bpp]
                    pos += len(raw)
                    raw_cels[fi][li] = _ase_decode_cel(raw, cw, ch, color_depth, palette, transparent_index)
                    raw_xy[fi][li]   = (cx, cy)

                elif cel_type == 1:    # linked cel — pixel data lives in another frame's cel
                    linked_fi = rd('H')
                    raw_cels[fi][li] = ('linked', linked_fi)
                    raw_xy[fi][li]   = (cx, cy)

                elif cel_type == 2:    # zlib-compressed image
                    cw, ch = rd('H'), rd('H')
                    raw = zlib.decompress(data[pos:chunk_end])
                    raw_cels[fi][li] = _ase_decode_cel(raw, cw, ch, color_depth, palette, transparent_index)
                    raw_xy[fi][li]   = (cx, cy)
                # cel_type 3 = compressed tilemap — not supported, falls through to chunk_end

            elif chunk_type == 0x2018:  # Tags
                num_tags = rd('H')
                pos += 8               # future (BYTE[8])
                for _ in range(num_tags):
                    from_f = rd('H')
                    to_f   = rd('H')
                    pos += 1           # loop direction (BYTE)
                    pos += 2           # repeat count (WORD, Aseprite 1.3+)
                    pos += 6           # future (BYTE[6])
                    pos += 3           # deprecated color (BYTE[3])
                    pos += 1           # extra zero (BYTE)
                    tags.append({'name': rd_str(), 'from': from_f, 'to': to_f})

            elif chunk_type == 0x2019:  # New palette chunk
                rd('I')                # new palette size (total entries)
                first = rd('I')
                last  = rd('I')
                pos += 8               # future (BYTE[8])
                for i in range(last - first + 1):
                    entry_flags = rd('H')
                    r, g, b, a = rd('B'), rd('B'), rd('B'), rd('B')
                    idx = first + i
                    if idx < 256:
                        palette[idx] = (r, g, b, a)
                    if entry_flags & 1:
                        rd_str()       # color name

            pos = chunk_end            # advance to exact chunk boundary (safety net)

    # ---- Resolve linked cels and composite visible layers ----
    def resolve_cel(fi, li, depth=0):
        """Follow linked cel chains. Always returns the current frame's (x,y) —
        only pixel data is borrowed from the linked frame."""
        if depth > num_frames:
            return None, (0, 0)
        cel = raw_cels[fi].get(li)
        if cel is None:
            return None, (0, 0)
        if isinstance(cel, tuple):
            _, linked_fi = cel
            img, _ = resolve_cel(linked_fi, li, depth + 1)
            return img, raw_xy[fi][li]  # position is always from the original frame
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
