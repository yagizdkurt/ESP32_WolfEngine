# Sprite Renderer

`SpriteRenderer` is the component that makes a [GameObject](gameobject.md) visible on screen. It is split into two parts - a `Sprite` asset that holds pixel data in flash, and the `SpriteRenderer` component that submits draw commands to the renderer.

---

## Step 1 — Define Pixel Data

Create a `constexpr uint8_t` array in flash. This can live anywhere — inline in your class, in a dedicated sprites header, wherever makes sense for your project.

**Valid sizes:** rectangular `[H][W]` arrays with `W` and `H` each in the range `1..63`. Non-2D arrays or out-of-range dimensions are compile errors.

```cpp
constexpr uint8_t playerPixels[3][3] = {
   {0, 1, 0},
   {1, 2, 1},
   {0, 1, 0}
};
```

> ✅ Define pixel arrays as `constexpr` so they live in flash, not RAM.

---

## Step 2 — Create a Sprite Asset

`Sprite` is a plain data asset — not a component. Use `Sprite::Create()` to turn your pixel array into a `Sprite`.

```cpp
constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels, PALETTE_WARM);
```

---

## Step 3 — Attach a SpriteRenderer

Declare a `SpriteRenderer` as a public member of your GameObject using the constructor below:

```
SpriteRenderer(owner, sprite, layer)
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `owner`   | `GameObject*`      | The owning object — always pass `this` |
| `sprite`  | `const Sprite*`    | Pointer to a `constexpr Sprite` asset in flash (pixels + palette + anchor) |
| `layer`   | `RenderLayer`      | Render layer — defaults to `RenderLayer::Default` if omitted |

```cpp
#include "WolfEngine/ComponentSystem/Components/WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"

class Player : public GameObject {
public:
   SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, RenderLayer::Player);
};
```

`SpriteRenderer` is tick-enabled by default. During component tick it:

1. Converts world position to screen position using the camera
2. Culls sprites fully outside the game region
3. Submits a sprite `DrawCommand` to the renderer

This happens only when `Settings.render.spriteSystemEnabled` is `true`.

Because command submission happens in component tick (before `Update()` and camera follow), sprites render using previous-frame transform/camera state.

> **Note:** You can use your defined render layers with the RenderLayer enum. Check [Layers](../engine-settings/render-layers.md) for more information.

---

## Pixel Data Format

Pixel arrays are row-major, left to right, top to bottom. Each byte is a palette index (0–31). Index 0 is always transparent.

```cpp
// 0 = transparent, 1 = outline, 2 = fill, 3 = highlight
constexpr uint8_t coinPixels[8][9] = {
    // 0 0 1 1 1 1 0 0 0
    {  0,0,1,1,1,1,0,0,0 },
    // 0 1 2 3 3 2 1 0 0
    {  0,1,2,3,3,2,1,0,0 },
    // 0 1 3 2 2 3 1 0 0
    {  0,1,3,2,2,3,1,0,0 },
    // 0 1 2 3 3 2 1 0 0
    {  0,1,2,3,3,2,1,0,0 },
    // 0 1 3 2 2 3 1 0 0
    {  0,1,3,2,2,3,1,0,0 },
    // 0 1 2 3 3 2 1 0 0
    {  0,1,2,3,3,2,1,0,0 },
    // 0 1 2 2 2 2 1 0 0
    {  0,1,2,2,2,2,1,0,0 },
    // 0 0 1 1 1 1 0 0 0
    {  0,0,1,1,1,1,0,0,0 },
};
constexpr Sprite SPRITE_COIN = Sprite::Create(coinPixels, PALETTE_WARM);
```

Keeping a comment map above each row makes sprite data human readable and easy to edit.

---

## Palette Variants

`SpriteRenderer` no longer owns a separate palette pointer. To change appearance, create sprite variants that share the same pixels and use `setSprite()`.

```cpp
constexpr uint8_t enemyPixels[8][8] = { /* ... */ };
constexpr Sprite ENEMY_NORMAL = Sprite::Create(enemyPixels, PALETTE_WARM);
constexpr Sprite ENEMY_HIT    = Sprite::Create(enemyPixels, PALETTE_SUNSET);

spriteRenderer.setSprite(&ENEMY_NORMAL);
spriteRenderer.setSprite(&ENEMY_HIT);
```

See [Settings](../engine-settings/settings.md) for related render settings and configuration context.

---

## Rotation

Sprites support four snap rotations applied per-pixel at draw time:

```cpp
spriteRenderer.setRotation(Rotation::R0);    // default — no rotation
spriteRenderer.setRotation(Rotation::R90);   // 90° clockwise
spriteRenderer.setRotation(Rotation::R180);  // 180°
spriteRenderer.setRotation(Rotation::R270);  // 270° clockwise
```

Anchors are stored in `Sprite` (`anchorX`, `anchorY`). By default they are centered (`W/2`, `H/2`) and remain pinned to the GameObject position across all rotations.

---

## Visibility

Hide a sprite without destroying the GameObject:

```cpp
spriteRenderer.setVisible(false);  // hidden — skipped by renderer each frame
spriteRenderer.setVisible(true);   // visible again
```

---

## Sort Key Override

By default, sprites are ordered within the same render layer by draw Y. You can override this with `setSortKey()`:

```cpp
spriteRenderer.setSortKey(100); // explicit order inside the same layer
spriteRenderer.clearSortKey();  // back to default drawY sorting
```

---

## Swapping Sprites

Swap the active `Sprite` asset at runtime with `setSprite()`. This is how the [Animator](animator.md) drives frame-based animation - just a pointer swap, no allocation and no manual render-pipeline changes.

```cpp
spriteRenderer.setSprite(&SPRITE_WALK_A);
spriteRenderer.setSprite(&SPRITE_WALK_B);
```

> **Note:** Never reassign a `SpriteRenderer` member with `= SpriteRenderer(...)` at runtime. Update sprite/rotation/visibility on the existing component instance instead.

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
