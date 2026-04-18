# General Settings

WolfEngine uses a set of settings files to configure the engine before compilation. Each file is focused on a specific system. This page covers the main engine-wide settings. For system-specific settings see the references below.

---

## Settings Files Overview

| File                | Purpose                                                      |
|---------------------|--------------------------------------------------------------|
| `WE_Settings.hpp`   | Engine-wide settings — Input, frame rate, display target, renderer |
| `WE_PINDEFS.hpp`    | SPI and display GPIO pin assignments — see [Pin Definitions](pin-definitions.md) |
| `WE_RenderLayers.hpp` | Layer order for sprite rendering — see [Render Layers](render-layers.md) |

> These files live in `WolfEngine/Settings/`. Edit them to match your hardware and project needs before building.

---

## Frame Rate

```cpp
#define TARGET_FRAME_TIME_US 33333
```

Target frame time in microseconds. The engine tries to maintain this frame duration each loop iteration.

`1,000,000 / TARGET_FRAME_TIME_US` gives you the target FPS. Common values:

| FPS | Value   |
|-----|---------|
| 60  | 16667   |
| 30  | 33333   |
| 20  | 50000   |

The real bottleneck is SPI, not the CPU. The ESP32 runs at 240 MHz and can execute game logic, physics, and rendering calculations in a fraction of a millisecond. However, flushing the framebuffer to the ST7735 over SPI at 10 MHz takes roughly 33 ms for a full 128x160 screen — which is almost exactly one frame at 30 FPS. This means pushing beyond 30 FPS on a full-screen flush is physically limited by the SPI bus, not the processor.

WolfEngine's partial flush optimization helps — when no UI has changed, only the game region (128x128) is sent, which takes around 26 ms and leaves more headroom. If you need higher frame rates, reducing `RENDER_UI_START_ROW` to shrink the game region or increasing `ST7735_SPI_CLOCK_HZ` in the display driver are the most effective levers.

---

## Game Object Settings
```cpp
#define MAX_GAME_OBJECTS 64
```

Maximum number of GameObjects that can exist at the same time. If `GameObject::Create<T>()` is called when the registry is full it returns `nullptr`. Always check the return value when creating objects dynamically:
```cpp
Bullet* b = GameObject::Create<Bullet>();
if (!b) {
    // registry full — handle gracefully
}
```

**What counts toward the limit:** Every living GameObject — players, enemies, bullets, pickups, effects. Objects destroyed with `destroyGameObject()` free their slot immediately and new objects can take their place.

**Choosing a value:** 64 is sufficient for most small games. A typical setup might be 1 player + 10 enemies + 20 bullets + 10 pickups = 41 objects peak. If your game spawns many short-lived objects like particles or projectiles, increase this accordingly.

**Memory impact:** Each slot in the registry is a pointer (4 bytes on ESP32), so the registry itself is `MAX_GAME_OBJECTS * 4` bytes. The objects themselves live on the heap — their size depends on what members you add. Keep this in mind on ESP32 where total RAM is 520KB.

---

## Display Target

```cpp
#define DISPLAY_ST7735
// #define DISPLAY_CUSTOM
```

Selects which display driver the renderer compiles against. Only one should be defined at a time. `DISPLAY_ST7735` targets the ST7735 128x160 TFT. `DISPLAY_CUSTOM` allows plugging in a custom driver implementation.

---

## Render Settings

```cpp
constexpr RenderSettings RENDER_SETTINGS = {
    .defaultBackgroundPixel = 0x0000,
    .gameRegion = { 0, 0, 128, 108 }
};
```

### defaultBackgroundPixel:

The color the renderer clears the framebuffer to at the start of every frame. Visible anywhere no sprite covers the screen. Uses RGB565 format — the same format as all palette colors in WolfEngine.

Common values:

| Color | Value   |
|-------|---------|
| Black | 0x0000  |
| White | 0xFFFF  |
| Red   | 0xF800  |
| Green | 0x07E0  |
| Blue  | 0x001F  |

### gameRegion

Rectangular area of the screen used for game rendering. `{ x1, y1, x2, y2 }`
Outside this region will not be rendered per frame but will be rendered when UI gets dirty.
