# Palette System

WolfEngine uses an indexed color system. Sprites do not store raw colors — they store palette indices. The renderer looks up the actual color from the palette at draw time. This keeps sprite data small, makes color swapping free, and fits naturally on hardware with limited RAM.

---

## How Palettes Work

Each palette is an array of exactly **32 RGB565 color entries** stored as `constexpr` in flash — zero RAM cost.

Sprite pixel data stores one byte per pixel. Each byte is an index from `0` to `31` into the palette. At draw time the renderer does a single array lookup per pixel to get the color.

```
Sprite pixel value: 8
Palette[8]: 0xF800 → Pure red
```

This indirection is what makes palette swapping free — changing the palette pointer costs nothing at runtime.

---

## Index 0 — Transparent

**Index 0 is always reserved as transparent across all palettes.** When the renderer encounters a pixel with index 0, it skips that pixel entirely and lets whatever is on the layer below show through. The RGB565 value stored at index 0 is never drawn.

Never use index 0 as a visible color in any palette or sprite.

---

## Built-in Palettes

| Palette             | Prefix | Description                        |
|---------------------|--------|------------------------------------|
| `PALETTE_GRAYSCALE` | `PL_GS_` | 31 shades from near-black to white |
| `PALETTE_WARM`      | `PL_WM_` | Reds, oranges, yellows, earth tones|
| `PALETTE_COOL`      | `PL_CL_` | Blues, greens, teals               |
| `PALETTE_GAMEBOY`   | `PL_GB_` | DMG Game Boy green shades          |
| `PALETTE_SUNSET`    | `PL_SS_` | Purples, magentas, warm pinks      |

Include all built-in palettes with a single line:
```cpp
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"
```

---

## Color Enums

Each built-in palette ships with a plain enum so you can reference colors by name instead of by index number. The prefix prevents name clashes between palettes since plain enums live in the global namespace.

```cpp
constexpr uint8_t playerPixels[4][4] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
};
constexpr Sprite PLAYER = Sprite::Create(playerPixels, PALETTE_WARM);

// Instead of this:
// pixel index 8 = red ❌ 

// You can document sprite data like this: ✅
constexpr uint8_t PlayerSprite[4][4] = {
    {0,              0,              0,              0},
    {0,  PL_WM_PureRed, PL_WM_PureRed,              0},
    {0,  PL_WM_PureRed, PL_WM_PureRed,              0},
    {0,              0,              0,              0},
};
```

For UI labels, use the enum directly as the color index:

```cpp
static UILabel hpLabel(
    4, 4, 96, 7,
    "HP: 100",
    PL_GS_White,
    PALETTE_GRAYSCALE,
    0,
    UIAnchor::TopLeft
);
```

You can set colors anywhere using palette + enum combination like:

```cpp
PALETTE_SUNSET[PL_SS_WarmYellow]
```

---

## Palette Swapping

`SpriteRenderer` does not expose `setPalette()`. For runtime palette changes, define sprite variants using the same pixel array and switch with `setSprite()`.

```cpp
constexpr uint8_t enemyPixels[8][8] = { /* ... */ };
constexpr Sprite ENEMY_NORMAL = Sprite::Create(enemyPixels, PALETTE_WARM);
constexpr Sprite ENEMY_HIT    = Sprite::Create(enemyPixels, PALETTE_SUNSET);

// Normal state
spriteRenderer.setSprite(&ENEMY_NORMAL);

// Hit flash
spriteRenderer.setSprite(&ENEMY_HIT);
```

Common uses:
- **Color variants** — same sprite, different palette = different character skin or enemy type
- **Damage flash** — swap to a bright palette on hit, restore next frame
- **Day/night cycle** — swap all sprites to a darker, cooler palette at night
- **Team colors** — same unit sprite, different palette per team

---

## RGB565 Color Format

All colors in WolfEngine are stored as 16-bit RGB565 values — the native format of most TFT displays:

```
Bit 15-11 — Red   (5 bits, 0–31)
Bit 10-5  — Green (6 bits, 0–63)
Bit  4-0  — Blue  (5 bits, 0–31)
```

To convert from standard RGB888:
```cpp
uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
```

> **Display note:** Colors can look different on the physical TFT compared to a PC monitor, especially greens and blues. Always test on the actual display.

---

## Creating a Custom Palette

Create a `.hpp` file anywhere in your project and define a `constexpr uint16_t` array of exactly 32 entries. Index 0 must always be `0x0000`.

```cpp
#pragma once
#include <stdint.h>

constexpr uint16_t MY_PALETTE[32] = {
    0x0000,  // 0  - Transparent (always required)
    0x....,  // 1
    0x....,  // 2
    // ...
    0x....,  // 31
};
```

Then include it and assign to a sprite:
```cpp
#include "MyPalette.hpp"
constexpr uint8_t myPixels[4][4] = {
    {0, 1, 1, 0},
    {1, 2, 2, 1},
    {1, 2, 2, 1},
    {0, 1, 1, 0},
};
constexpr Sprite MY_SPRITE = Sprite::Create(myPixels, MY_PALETTE);
```

Optionally add an enum for named access:
```cpp
enum MyPalette : uint8_t {
    MP_Transparent = 0,
    MP_Red         = 1,
    // ...
};
```

> Define palettes as `constexpr` so they live in flash, not RAM. A single palette is 32 × 2 = **64 bytes**. With 5 built-in palettes that is 320 bytes total in flash.

---

## Sprite Data Format

Sprite pixel arrays are row-major, left to right, top to bottom. One byte per pixel, each byte a palette index.

```cpp
// 0 = transparent, 1 = outline, 2 = fill
constexpr uint8_t MySprite[4][4] = {
    {0, 1, 1, 0},
    {1, 2, 2, 1},
    {1, 2, 2, 1},
    {0, 1, 1, 0},
};
```

`Sprite` stores dimensions and anchor point directly (`width`, `height`, `anchorX`, `anchorY`). `SpriteRenderer` receives only a sprite pointer and render layer. See [Sprite Renderer](../gameobjects-and-components/sprite-renderer.md) for the full sprite API.
