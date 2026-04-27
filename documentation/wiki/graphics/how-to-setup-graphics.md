# How To Setup Graphics

This page walks through everything you need to get graphics on screen — from defining pixel data to rendering a moving object with a palette.

---

## The Three Parts

Getting a sprite on screen requires three things working together:

| Part        | Lives in         | Purpose                                 |
|-------------|------------------|-----------------------------------------|
| Pixel data  | Flash (`constexpr`) | Defines the shape using palette indices |
| Palette     | Flash (`constexpr`) | Maps indices to RGB565 colors           |
| Sprite + SpriteRenderer | Flash + GameObject | `Sprite` bundles pixels, palette, dimensions, anchor; `SpriteRenderer` submits it to the renderer |

---

## Step 1 — Choose a Palette

Pick one of the built-in palettes or create your own. Include the master palette header to get all five at once:

```cpp
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"
```

Use the color enums to document what each index means in your sprite:

```cpp
// Mapping for PlayerPixels above:
// 1 = PL_GS_Shade5  (dark outline)
// 2 = PL_GS_MidGray (body)
// 3 = PL_GS_Shade25 (highlight)
```

See [Settings](../engine-settings/settings.md) for related render configuration.
> **Note:** Every palette has all the real values and explanations in their file
> **Tip:** After you select your palette you can actually see what colors are in the palette by writing as below.

```cpp
PL_GS_
```

---

## Step 2 — Define Pixel Data

Create a `constexpr uint8_t` 2D array as `[H][W]` (rows × columns). Each value is a palette index (0–31). Index 0 is always transparent. `W` and `H` can be different; each must be in `1..63`.

```cpp
// 0 = transparent, 1 = dark, 2 = mid, 3 = light
constexpr uint8_t PlayerPixels[8][8] = {
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 2, 2, 2, 2, 1, 0 },
    { 0, 1, 2, 3, 3, 2, 1, 0 },
    { 1, 2, 3, 2, 2, 3, 2, 1 },
    { 1, 2, 3, 2, 2, 3, 2, 1 },
    { 0, 1, 2, 3, 3, 2, 1, 0 },
    { 0, 1, 2, 2, 2, 2, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 },
};

constexpr Sprite SPRITE_PLAYER = Sprite::Create(PlayerPixels, PALETTE_GRAYSCALE);
```

> ✅ Keep pixel data in a separate header like `MyObject_Sprites.hpp` to keep your GameObject class clean.

> **Tip:** You can use palette enums or simple numbers. Although palette enums are more readable, numbers keep the object's actual shape more visible.

> **Warning:** Don't forget to put `constexpr` before the array. Or you will bloat your RAM.

---

## Step 3 — Attach a SpriteRenderer to a GameObject

Declare a `SpriteRenderer` as a public member and construct it with the array you created. See [Sprite Renderer](../gameobjects-and-components/sprite-renderer.md) for the constructor.

The SpriteRenderer submits a sprite draw command during component tick. The renderer executes buffered commands later in the frame.

```cpp
#include "WolfEngine/WolfEngine.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"
#include "PlayerSprites.hpp"

class Player : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, RenderLayer::Player);

    void Start() override {
        transform.position = { 64, 64 };
        MainCamera().setTarget(this);
    }

    void Update() override {
        if (Input().getButton(Button::A)) transform.position.x += 1;
        if (Input().getButton(Button::B)) transform.position.x -= 1;
    }
};
```

---

## Layers

Layers control draw order. Lower layers are drawn first — higher layers appear on top. Assign the layer at SpriteRenderer construction time:

```cpp
SpriteRenderer background = SpriteRenderer(this, &BG_SPRITE, RenderLayer::BackGround);
SpriteRenderer enemy      = SpriteRenderer(this, &ENEMY_SPRITE, RenderLayer::Entities);
SpriteRenderer player     = SpriteRenderer(this, &PLAYER_SPRITE, RenderLayer::Player);
SpriteRenderer explosion  = SpriteRenderer(this, &FX_SPRITE, RenderLayer::FX);
```

See [Render Layers](../engine-settings/render-layers.md) to customize the layer list.

Within the same layer, sprites are sorted by draw Y by default. Use `setSortKey()` on `SpriteRenderer` if you need explicit ordering.

---

## Palette Variants

To swap colors at runtime, prepare multiple `Sprite` assets that share the same pixel array and switch sprites:

```cpp
constexpr uint8_t playerPixels[8][8] = { /* ... */ };
constexpr Sprite PLAYER_NORMAL = Sprite::Create(playerPixels, PALETTE_GRAYSCALE);
constexpr Sprite PLAYER_HIT    = Sprite::Create(playerPixels, PALETTE_SUNSET);

void Update() override {
    if (isHit) {
        spriteRenderer.setSprite(&PLAYER_HIT);
    } else {
        spriteRenderer.setSprite(&PLAYER_NORMAL);
    }
}
```

---

## Rotation

Rotate a sprite in 90° increments:

```cpp
spriteRenderer.setRotation(Rotation::R90);   // face right
spriteRenderer.setRotation(Rotation::R270);  // face left
```

---

## Full Example — Animated Coin

```cpp
constexpr uint8_t CoinA[3][3] = {
    {0, 1, 0},
    {1, 2, 1},
    {0, 1, 0},
};

constexpr uint8_t CoinB[3][3] = {
    {0, 1, 0},
    {0, 1, 0},
    {0, 1, 0},
};

constexpr Sprite COIN_A = Sprite::Create(CoinA, PALETTE_WARM);
constexpr Sprite COIN_B = Sprite::Create(CoinB, PALETTE_WARM);

class Coin : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &COIN_A, RenderLayer::Entities);

    void Update() override {
        m_timer++;
        if (m_timer % 30 < 15)
            spriteRenderer.setSprite(&COIN_A);
        else
            spriteRenderer.setSprite(&COIN_B);
    }

private:
    int m_timer = 0;
};
```
