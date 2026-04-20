# First GameObject

This page walks you through creating your first moving, visible game object from scratch in WolfEngine.

---

## The Basics
Every game entity in WolfEngine is a `GameObject`. You create one by inheriting from it and overriding `Start()` and `Update()`:

```cpp
#include "WolfEngine/WolfEngine.hpp"
class MyObject : public GameObject {
public:
    void Start() override {
        // called once when the object is created
        transform.position = { 64, 64 };
    }
    void Update() override {
        // called every frame
    }
};
```

Then create it in `app_main()` before `StartGame()`:

```cpp
Engine().StartEngine();
GameObject::Create<MyObject>();
Engine().StartGame();
```

That's it — the engine handles the rest.

> **Tip:** Refer to [GameObject](../gameobjects-and-components/gameobject.md) to better understand how GameObjects work.
> 
> **Tip:** Refer to [Engine](../core-systems/engine.md) to better understand engine initialization.

---

## Adding Movement
Get a controller reference in `Start()` and read input in `Update()`:

```cpp
class MyObject : public GameObject {
public:
    Controller* controller = nullptr;

    void Start() override {
        controller = Input().getController(0); // player 1
    }

    void Update() override {
        if (!controller) return;
        if (controller->getButton(Button::A)) transform.position.x += 1;
        if (controller->getButton(Button::B)) transform.position.x -= 1;
        if (controller->getButton(Button::C)) transform.position.y -= 1;
        if (controller->getButton(Button::D)) transform.position.y += 1;
    }
};
```

Or use the joystick for analog movement:

```cpp
void Update() override {
    if (!controller) return;
    transform.position.x += controller->getAxis(JoyAxis::X) * 2.0f;
    transform.position.y += controller->getAxis(JoyAxis::Y) * 2.0f;
}
```

> **Tip:** Refer to [Input](../core-systems/input.md) to better understand how input handling works.

---

## Adding a Sprite
First define pixel data and create a `Sprite` asset in flash, then attach a `SpriteRenderer` component.

Sprite sizes must be odd numbers (1x1, 3x3, 5x5, 7x7 ...). Even sizes are not supported.

Pixel data must be declared as a separate `constexpr` array — inline initializer lists are not supported.

```cpp
#include "WolfEngine/WolfEngine.hpp"

constexpr uint8_t myPixels[7 * 7] = {
    0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 2, 1, 2, 1, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 2, 1, 2, 1, 0,
    0, 1, 1, 2, 1, 1, 0,
    0, 0, 1, 1, 1, 0, 0,
};
constexpr Sprite MY_SPRITE = Sprite::Create(myPixels);

class MyObject : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &MY_SPRITE, PALETTE_WARM, RenderLayer::Player);
    Controller* controller = nullptr;

    void Start() override {
        transform.position = { 64, 64 };
        controller = Input().getController(0);
    }

    void Update() override {
        if (!controller) return;
        if (controller->getButton(Button::A)) transform.position.x += 1;
        if (controller->getButton(Button::B)) transform.position.x -= 1;
    }
};
```

The sprite draws at `transform.position` every frame automatically.

`0` in pixel data is always transparent. Indices `1`–`31` map to palette colors.

> **Tip:** Refer to [Sprite Renderer](../gameobjects-and-components/sprite-renderer.md) to better understand how sprites work.

---

## Full Example — A Moving Player

```cpp
#include "WolfEngine/WolfEngine.hpp"

constexpr uint8_t playerPixels[7 * 7] = {
    0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 2, 1, 2, 1, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 2, 1, 2, 1, 0,
    0, 1, 1, 2, 1, 1, 0,
    0, 0, 1, 1, 1, 0, 0,
};
constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);

class Player : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, PALETTE_GRAYSCALE, RenderLayer::Player);
    Controller* controller = nullptr;

    void Start() override {
        transform.position = { 64, 64 };
        controller = Input().getController(0);
    }

    void Update() override {
        if (!controller) return;
        float speed = 2.0f;
        transform.position.x += controller->getAxis(JoyAxis::X) * speed;
        transform.position.y += controller->getAxis(JoyAxis::Y) * speed;
    }
};

extern "C" void app_main() {
    Engine().StartEngine();
    GameObject::Create<Player>();
    Engine().StartGame();
}
```

---

## What's Next

| | |
|---|---|
| ⏩ **[Engine Settings](../core-systems/engine.md)** | Configure frame rate, background color, and UI region |
| 📌 **[Pin Setup](../pin-definitions.md)** | Set up your GPIO and SPI wiring |
| 🛰️ **[About GameObjects](../gameobjects-and-components/gameobject.md)** | Learn everything about GameObjects |
| 🎮 **[Inputs!](../core-systems/input.md)** | How to get inputs from the controllers? |
| 🚗 **[Your First Game Object!](first-gameobject.md)** | Learn how to create and move objects |
| 🎨 **[How To Set Up Graphics?](../graphics/how-to-setup-graphics.md)** | Get graphics on screen |
| 🔉  **[How to pew pew?](../core-systems/audio.md)** | Make the buzzer go brr |
