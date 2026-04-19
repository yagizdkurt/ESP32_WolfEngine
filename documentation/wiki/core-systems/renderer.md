# Renderer

The Renderer is an internal WolfEngine system. It manages the framebuffer, a fixed-size per-frame `DrawCommand` buffer, and the full draw/flush cycle every frame. You do not usually call it directly in game code - `SpriteRenderer` and other systems submit commands, and the engine drives rendering each frame.

This page explains how rendering works under the hood and how to add a custom display driver.

❗ If you are looking for how to add graphics to your game, check [How To Setup Graphics](../graphics/how-to-setup-graphics.md) ❗

---

## How Rendering Works

Every frame the renderer executes four steps in order:

**1. Clear**
If `cleanFramebufferEachFrame` is enabled in [Settings](../settings.md), the framebuffer is filled with `defaultBackgroundPixel`. This erases everything drawn in the previous frame. Disable this if you are managing the framebuffer manually and want to retain previous frame contents.

**2. Sort + Execute Command Buffer**
The renderer sorts buffered commands by `sortKey` and executes them in one linear pass. `sortKey` is packed as `uint16_t` (`RenderLayer` in the high byte, screenY in the low byte), so one ascending compare yields layer order plus in-layer Y sorting. For sprite commands, transparent pixels (palette index 0) are skipped and drawing is clipped to the game region.

`SpriteRenderer` produces sprite commands during component tick when `spriteSystemEnabled` is `true` (see [Settings](../settings.md)). The renderer itself still runs sort/execute every frame, even when sprite submission is disabled.

**3. Draw UI Region**
The UI region is only redrawn when something has changed — this is the dirty flag system. If no UI element has been modified since the last frame this step is skipped entirely. See [UI Manager](ui-manager.md) for details.

**4. Flush to Display**
The framebuffer is sent to the display over SPI. If the UI was redrawn, the full screen is flushed in one transfer. If only the game region changed, only the game region rectangle is sent — saving SPI bandwidth and keeping frame times lower.

---

## Screen Regions

The screen is divided into two independent regions:

```
(x1, y1)                   ┐
  ...                      │  Game region — cleared and redrawn every frame
(x2, y2)                   ┘

                           ┐
  Outside                  │  UI region — only redrawn when dirty
                           ┘
```

The game region is a configurable rectangle defined by `{ x1, y1, x2, y2 }` in [Settings](../settings.md). The area below it is the UI region. The UI region retains its last drawn content until a UI element changes — at that point it is redrawn and a full screen flush occurs. This split means a static HUD costs almost nothing per frame in SPI bandwidth.

See [Settings](../settings.md) to configure `gameRegion`.

---

## Sprite Layers

Commands are sorted by a single ascending packed `sortKey`. Because the high byte stores `RenderLayer`, layer 0 is drawn first (bottom) and the highest layer is drawn last (top).

Within a layer (same high byte), the low byte sorts by screenY in ascending order. For `SpriteRenderer`, the default sort source is draw Y (screen-space top), and you can override it with `setSortKey()`.

See [Render Layers](../render-layers.md) for the full layer configuration.

---

## Command Buffer Capacity

The command buffer capacity is configured by `MAX_DRAW_COMMANDS` in render settings.

- If the buffer fills in a frame, new commands are dropped.
- Drops are counted in renderer diagnostics (`commandsDropped`). Logging is throttled to the first drop in a frame.
- `peakCommandCount` helps tune capacity for your game.

You can inspect diagnostics via:

```cpp
const FrameDiagnostics& d = RenderSys().getDiagnostics();
```

---

## Rotation

The renderer supports four rotation states per sprite — `R0`, `R90`, `R180`, `R270`. Rotation is encoded in `DrawCommand.flags` bits 7-6 and extracted during command execution. It is applied per-pixel at draw time by remapping the source pixel index. There is no rotation matrix — it is index arithmetic, so it has minimal overhead.

---

## Render Settings

All rendering behaviour is configured in `WE_RenderSettings.hpp` (included by `WE_Settings.hpp`). Check [Settings](../settings.md) for more information.

---

## Advanced: Direct Framebuffer Access

If you need pixel-level control, you can write directly to the framebuffer at any time:

```cpp
uint16_t* fb = RenderSys().getCanvas();
fb[y * RENDER_SCREEN_WIDTH + x] = 0xFFFF;
```

This bypasses the sprite system entirely. See [Framebuffer Access](buffer.md) for full details and relevant render settings.

---

## Advanced: Display Drivers

The renderer is decoupled from the physical display through a `DisplayDriver` base class. The active driver is selected at compile time in [Settings](../settings.md):

```cpp
#define DISPLAY_ST7735   // built-in ST7735 driver
// #define DISPLAY_CUSTOM  // your own driver
```

**Built-in: ST7735**
The default embedded driver targets the ST7735 128x160 TFT over SPI. Pin assignments are configured in [Pin Definitions](pin-definitions.md).

**Built-in: SDL (desktop)**
Desktop builds can use the SDL display driver to preview rendering without ESP hardware.

**Custom Driver**
To use a different display, define `DISPLAY_CUSTOM` and implement the `DisplayDriver` interface in `Display_Custom.h`. Your driver must provide:

```cpp
void initialize();
void flush(const uint16_t* framebuffer, int x1, int y1, int x2, int y2);
int screenWidth;
int screenHeight;
```

`flush()` receives a pointer to the framebuffer and the region to send. The engine calls it once per frame with either the full screen or the game region only depending on whether the UI was dirty.

> **Note:** The ST7735 driver performs a byte swap on all pixel values before sending over SPI due to endianness differences between the ESP32 and the display. If you write a custom driver, be aware that palette colors are stored as logical RGB565 values — your driver is responsible for any byte ordering your display requires.
