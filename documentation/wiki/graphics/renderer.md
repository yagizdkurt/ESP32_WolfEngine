# Renderer

The renderer owns the RGB565 framebuffer and a fixed-size per-frame `DrawCommand` buffer. Game systems submit commands; the engine executes rendering once per frame.

If you are looking for gameplay usage, see [How To Setup Graphics](../graphics/how-to-setup-graphics.md).

---

## How Rendering Works

Current frame pipeline is two-pass:

**1. Clear**
If `cleanFramebufferEachFrame` is enabled in [Settings](../settings.md), framebuffer is filled with `defaultBackgroundPixel`.

**2. World Pass (sort + execute)**
Commands buffered during component tick (mostly sprite commands) are sorted by `sortKey` and executed.

**3. UI Pass (submit + sort + execute)**
`UI().render()` is called every frame. UI elements submit `FillRect`, `Line`, `Circle`, and `TextRun` commands, then those commands are sorted and executed.

**4. Flush**
Framebuffer is flushed once per frame as full-screen.

---

## Command Types

`DrawCommandType` currently includes:
- `Sprite`
- `FillRect`
- `Line`
- `Circle`
- `TextRun`

Sprites still clip to `RENDER_SETTINGS.gameRegion`. UI primitives clip to screen bounds.

---

## Layers and Ordering

Within each pass, sorting is by ascending `sortKey`:
- High byte: `RenderLayer`
- Low byte: pass-local order key (`screenY` for sprites, draw order index for UI)

Important:
- World and UI are executed in separate passes.
- `RenderLayer` does not interleave world and UI across passes.
- UI pass always runs after world pass.

See [Render Layers](../render-layers.md) for layer definitions.

---

## Command Buffer Capacity

Capacity is `MAX_DRAW_COMMANDS` from render settings.

- If the buffer fills, new commands are dropped.
- Drops are counted in `commandsDropped`.
- `peakCommandCount` tracks the high-watermark command count.

```cpp
const FrameDiagnostics& d = RenderSys().getDiagnostics();
```

---

## Rotation

Sprite rotation uses `DrawCommand.flags` bits 7-6 (`R0`, `R90`, `R180`, `R270`). Execution remaps source indices per pixel; no matrix math is used.

---

## Render Settings

Renderer behavior is configured in `WE_RenderSettings.hpp` (included by `WE_Settings.hpp`). See [Settings](../settings.md).

---

## Advanced: Direct Framebuffer Access

You can still write raw pixels:

```cpp
uint16_t* fb = RenderSys().getCanvas();
fb[y * RENDER_SCREEN_WIDTH + x] = 0xFFFF;
```

This bypasses command submission for those writes. See [Framebuffer Access](../graphics/buffer.md).

---

## Advanced: Display Drivers

Display output is abstracted by `DisplayDriver`.

Core driver interface:

```cpp
void initialize();
void flush(const uint16_t* framebuffer, int x1, int y1, int x2, int y2);
```

Current renderer call uses full-screen bounds each frame.

Notes:
- ST7735 path may require byte-swapped RGB565 (`requiresByteSwap = true`).
- Desktop SDL path currently uploads/presents full framebuffer per flush.
