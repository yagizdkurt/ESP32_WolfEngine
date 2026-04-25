# General Settings

WolfEngine configuration is centralized in `WolfEngine/Settings/WE_Settings.hpp` as a single `inline constexpr EngineConfig Settings` object.

All runtime systems read from `Settings.domain.field`.

---

## Settings Files Overview

| File                | Purpose                                                      |
|---------------------|--------------------------------------------------------------|
| `WE_Settings.hpp`   | User-facing `Settings` instance values (hardware/render/input/limits/debug) |
| `WE_ConfigTypes.hpp`| Engine-owned config types (`EngineConfig`, `RenderLayer`, `CollisionLayer`, `Region`, etc.) |

> Both files live in `WolfEngine/Settings/`. Edit values in `WE_Settings.hpp` for project customization.

---

## Frame Rate

```cpp
.targetFrameTimeUs = 33333,
```

Target frame time in microseconds. The engine tries to maintain this frame duration each loop iteration.

`1,000,000 / Settings.render.targetFrameTimeUs` gives the target FPS. Common values:

| FPS | Value   |
|-----|---------|
| 60  | 16667   |
| 30  | 33333   |
| 20  | 50000   |

The real bottleneck is SPI, not the CPU. The ESP32 can run gameplay logic quickly, but display transfer still dominates frame time for full-screen RGB565 updates.

Current renderer behavior is a two-pass world+UI command execution followed by a full-screen flush every frame. If you need higher frame rates, the most effective lever is increasing display bus throughput (for example SPI clock and driver efficiency).

---

## Game Object Settings
```cpp
.maxGameObjects = 64,
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

**Memory impact:** Each slot in the registry is a pointer (4 bytes on ESP32), so the registry itself is `Settings.limits.maxGameObjects * 4` bytes. The objects themselves live on the heap — their size depends on what members you add. Keep this in mind on ESP32 where total RAM is 520KB.

**Valid range:** `maxGameObjects` is `uint8_t` and validated with static assertions; keep it in `1..255`.

---

## UI Element Limits

```cpp
.maxUIElements = 32,
.maxPanelChildren = 10,
```

`maxUIElements` bounds the top-level `UI().setElements(...)` array length at compile time.
`maxPanelChildren` bounds `UIPanel` child array length at compile time.

Both values are validated with static assertions and must be in `1..255`.

---

## Display Target

```cpp
#define DISPLAY_ST7735
// DISPLAY_SDL is provided by desktop CMake (-DDISPLAY_SDL)
```

Display target is selected by compile definitions. `WE_Settings.hpp` defines `DISPLAY_ST7735` by default when `DISPLAY_SDL` is not present.

---

## Render Settings

```cpp
.render = {
    .gameRegion = { 0, 0, 128, 108 },
    .maxDrawCommands = 128,
    .defaultBackgroundPixel = 0x0000,
    .spriteSystemEnabled = true,
    .cleanFramebufferEachFrame = true,
    .targetFrameTimeUs = 33333,
    .displayTarget = DisplayTarget::ST7735,
},
```

`Settings.render.maxDrawCommands` controls command buffer capacity per frame. If the buffer fills, extra commands are dropped and counted in renderer diagnostics (the renderer logs only the first drop in that frame).

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

Rectangular area used by world sprite clipping/culling. `{ x1, y1, x2, y2 }` (`x2` and `y2` are exclusive).

`gameRegion` affects sprite drawing bounds. UI primitive commands are clipped to screen bounds, not `gameRegion`.

### spriteSystemEnabled

When `true`, `SpriteRenderer` components submit sprite draw commands during component tick.
When `false`, `SpriteRenderer` submits nothing. The renderer still runs command sort/execute and UI/flush each frame.

### cleanFramebufferEachFrame

When `true`, the renderer clears the framebuffer to `defaultBackgroundPixel` at frame start.
When `false`, previous frame pixels persist until overwritten.
