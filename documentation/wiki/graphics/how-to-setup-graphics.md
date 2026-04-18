# How To Setup Graphics

This page walks through everything you need to get graphics on screen — from defining pixel data to rendering a moving object with a palette.

---

## The Three Parts

Getting a sprite on screen requires three things working together:

| Part        | Lives in         | Purpose                                 |
|-------------|------------------|-----------------------------------------|
| Pixel data  | Flash (`constexpr`) | Defines the shape using palette indices |
| Palette     | Flash (`constexpr`) | Maps indices to RGB565 colors           |
| SpriteRenderer component | GameObject | Connects pixel data + palette to the renderer |

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

See [Palette](../settings.md) for the full built-in palette reference.
> **Note:** Every palette has all the real values and explanations in their file
> **Tip:** After you select your palette you can actually see what colors are in the palette by writing as below.

```cpp
PL_GS_
```

---

## Step 2 — Define Pixel Data

Create a `constexpr uint8_t` array. Each value is a palette index (0–31). Index 0 is always transparent. Sprites must be square — valid sizes are NxN where N is odd.

```cpp
// 0 = transparent, 1 = dark, 2 = mid, 3 = light
constexpr uint8_t PlayerPixels[] = { // 7x7
    0, 0, 1, 1, 1, 1, 0, 
    0, 1, 2, 2, 2, 2, 1, 
    0, 1, 2, 3, 3, 2, 1, 
    1, 2, 3, 2, 2, 3, 2, 
    1, 2, 3, 2, 2, 3, 2, 
    0, 1, 2, 3, 3, 2, 1, 
    0, 1, 2, 2, 2, 2, 1, 
    0, 0, 1, 1, 1, 1, 0, 
};
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
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, PALETTE_GRAYSCALE, RenderLayer::Player);

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
SpriteRenderer background = SpriteRenderer(this, &BG_SPRITE, PALETTE_COOL, RenderLayer::BackGround);
SpriteRenderer enemy      = SpriteRenderer(this, &ENEMY_SPRITE, PALETTE_WARM, RenderLayer::Entities);
SpriteRenderer player     = SpriteRenderer(this, &PLAYER_SPRITE, PALETTE_GRAYSCALE, RenderLayer::Player);
SpriteRenderer explosion  = SpriteRenderer(this, &FX_SPRITE, PALETTE_SUNSET, RenderLayer::FX);
```

See [Render Layers](../render-layers.md) to customize the layer list.

Within the same layer, sprites are sorted by draw Y by default. Use `setSortKey()` on `SpriteRenderer` if you need explicit ordering.

---

## Palette Swapping

Swap the palette at runtime for effects like damage flash or color variants:

```cpp
void Update() override {
    if (isHit) {
        spriteRenderer.setPalette(PALETTE_SUNSET);  // flash red/pink on hit
    } else {
        spriteRenderer.setPalette(PALETTE_GRAYSCALE);  // normal
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
constexpr uint8_t CoinA[] = { // 3x3 frame A
    0, 1, 0,
    1, 2, 1, 
    0, 1, 0,
};

constexpr uint8_t CoinB[] = { // 3x3 frame B
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
};

class Coin : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &COIN_A, PALETTE_WARM, RenderLayer::Entities);

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
