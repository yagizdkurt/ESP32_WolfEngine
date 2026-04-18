# Sprite Renderer

`SpriteRenderer` is the component that makes a [GameObject](gameobject.md) visible on screen. It is split into two parts — a `Sprite` asset that holds pixel data in flash, and the `SpriteRenderer` component that the renderer talks to.

---

## Step 1 — Define Pixel Data

Create a `constexpr uint8_t` array in flash. This can live anywhere — inline in your class, in a dedicated sprites header, wherever makes sense for your project.

**Valid sizes:** NxN odd numbers up to 65. Passing a non-square or unsupported size is a compile error.

```cpp
constexpr uint8_t playerPixels[3 * 3] = {
    0, 1, 0,
    1, 2, 1,
    0, 1, 0
};
```

> ✅ Define pixel arrays as `constexpr` so they live in flash, not RAM.

---

## Step 2 — Create a Sprite Asset

`Sprite` is a plain data asset — not a component. Use `Sprite::Create()` to turn your pixel array into a `Sprite`.

```cpp
constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);
```

---

## Step 3 — Attach a SpriteRenderer

Declare a `SpriteRenderer` as a public member of your GameObject using the constructor below:

```
SpriteRenderer(owner, sprite, palette, layer)
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `owner`   | `GameObject*`      | The owning object — always pass `this` |
| `sprite`  | `const Sprite*`    | Pointer to a `constexpr Sprite` asset in flash |
| `palette` | `const uint16_t*`  | 32-entry RGB565 palette array in flash |
| `layer`   | `RenderLayer`      | Render layer — defaults to `RenderLayer::Default` if omitted |

```cpp
#include "WolfEngine/ComponentSystem/Components/WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"

class Player : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, PALETTE_WARM, RenderLayer::Player);
};
```

The SpriteRenderer registers with the renderer on construction and unregisters on destruction — you never manage this manually.

> **Note:** You can use your defined render layers with the RenderLayer enum. Check [Layers](../settings.md) for more information.

---

## Pixel Data Format

Pixel arrays are row-major, left to right, top to bottom. Each byte is a palette index (0–31). Index 0 is always transparent.

```cpp
// 0 = transparent, 1 = outline, 2 = fill, 3 = highlight
constexpr uint8_t coinPixels[9 * 9] = {
    // 0 0 1 1 1 1 0 0 0
       0,0,1,1,1,1,0,0,0,
    // 0 1 2 3 3 2 1 0 0
       0,1,2,3,3,2,1,0,0,
    // 0 1 3 2 2 3 1 0 0
       0,1,3,2,2,3,1,0,0,
    // 0 1 2 3 3 2 1 0 0
       0,1,2,3,3,2,1,0,0,
    // 0 1 3 2 2 3 1 0 0
       0,1,3,2,2,3,1,0,0,
    // 0 1 2 3 3 2 1 0 0
       0,1,2,3,3,2,1,0,0,
    // 0 1 2 2 2 2 1 0 0
       0,1,2,2,2,2,1,0,0,
    // 0 0 1 1 1 1 0 0 0
       0,0,1,1,1,1,0,0,0,
};
constexpr Sprite SPRITE_COIN = Sprite::Create(coinPixels);
```

Keeping a comment map above each row makes sprite data human readable and easy to edit.

---

## Palette Swapping

Swap the active palette at runtime with `setPalette()`. Takes effect immediately on the next frame — no copying, just a pointer swap.

```cpp
spriteRenderer.setPalette(PALETTE_WARM);    // normal state
spriteRenderer.setPalette(PALETTE_SUNSET);  // hit flash
```

See [Palette](../settings.md) for common palette swap patterns like damage flash, color variants, and day/night cycles.

---

## Rotation

Sprites support four snap rotations applied per-pixel at draw time:

```cpp
spriteRenderer.setRotation(Rotation::R0);    // default — no rotation
spriteRenderer.setRotation(Rotation::R90);   // 90° clockwise
spriteRenderer.setRotation(Rotation::R180);  // 180°
spriteRenderer.setRotation(Rotation::R270);  // 270° clockwise
```

---

## Visibility

Hide a sprite without destroying the GameObject:

```cpp
spriteRenderer.setVisible(false);  // hidden — skipped by renderer each frame
spriteRenderer.setVisible(true);   // visible again
```

---

## Swapping Sprites

Swap the active `Sprite` asset at runtime with `setSprite()`. This is how the [Animator](animator.md) drives frame-based animation — just a pointer swap, no allocation or renderer registration changes.

```cpp
spriteRenderer.setSprite(&SPRITE_WALK_A);
spriteRenderer.setSprite(&SPRITE_WALK_B);
```

> **Note:** Never reassign a `SpriteRenderer` member with `= SpriteRenderer(...)` at runtime — this will cause double registration with the renderer and potential dangling pointers.

---

## Animation

For frame-based animation use the `Animator` component — it handles frame timing and calls `setSprite()` automatically. See [Animator](animator.md) for details.

---

## Getters

```cpp
spriteRenderer.getSprite();     // const Sprite*
spriteRenderer.isVisible();     // bool
spriteRenderer.getRotation();   // Rotation
spriteRenderer.getLayer();      // int
```
