# Renderer

The Renderer is an internal WolfEngine system. It manages the framebuffer, sprite layer table, and drives the full draw and flush cycle every frame. You do not interact with it directly in game code — sprites register themselves automatically and the engine drives rendering each frame.

This page explains how rendering works under the hood and how to add a custom display driver.

❗ If you are looking for how to add graphics to your game, check [How To Setup Graphics](../graphics/how-to-setup-graphics.md) ❗

---

## How Rendering Works

Every frame the renderer executes four steps in order:

**1. Clear**
If `cleanFramebufferEachFrame` is enabled in [Settings](../settings.md), the framebuffer is filled with `defaultBackgroundPixel`. This erases everything drawn in the previous frame. Disable this if you are managing the framebuffer manually and want to retain previous frame contents.

**2. Draw Game Region**
If `spriteSystemEnabled` is enabled in [Settings](../settings.md), all sprite layers from `LAYER_BACKGROUND` through `LAYER_FX` are iterated in order. Each sprite's world position is converted to screen coordinates via the camera, clipped to the game region, and written pixel-by-pixel into the framebuffer. Transparent pixels (palette index 0) are skipped. Sprites outside the game region are culled before drawing.

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

Sprites are sorted into layers which are drawn in ascending order — layer 0 is drawn first (bottom), the highest layer is drawn last (top). Layer assignment happens at sprite creation time and cannot be changed at runtime.

See [Render Layers](../settings.md) for the full layer configuration.

---

## Rotation

The renderer supports four rotation states per sprite — `R0`, `R90`, `R180`, `R270`. Rotation is applied per-pixel at draw time by remapping the source pixel index. There is no rotation matrix — it is index arithmetic, so it has minimal overhead.

---

## Render Settings

All rendering behaviour is configured in `WE_Settings.hpp`. Check [Settings](../settings.md) for more information.

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
The default driver targets the ST7735 128x160 TFT over SPI. Pin assignments are configured in [Pin Definitions](pin-definitions.md).

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
