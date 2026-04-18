# Engine

WolfEngine is the core class of the engine. It owns every major subsystem, drives the game loop, and exposes global accessor functions so you can reach any system from anywhere in game code without passing pointers around.

It is a singleton — there is always exactly one instance.

---

## Starting the Engine

Every WolfEngine project follows the same structure:

```cpp
#include "WolfEngine/WolfEngine.hpp"

extern "C" void app_main() {
    Engine().StartEngine();   // initializes all subsystems

    // Register UI elements here — after StartEngine, before StartGame
    UI().setElements(uiElements); // uiElements must be a null-terminated array

    // Create game objects here
    GameObject::Create<Player>();

    Engine().StartGame();     // starts the game loop — never returns
}
```

**`StartEngine()`** — initializes the display driver, renderer, camera, input system, UI, sound, and modules. Must be called first before anything else.

**`StartGame()`** — starts the game loop. Calls `Update()` on all active GameObjects every frame, ticks the input system, updates modules, and drives the renderer. This call never returns until `RequestQuit()` is called.

> **Note:** UI elements must be registered between `StartEngine()` and `StartGame()`. The array passed to `UI().setElements()` must be null-terminated. See [UI Manager](ui-manager.md) for details.

---

## Game Loop

Every frame WolfEngine:

1. Polls input via [Input](input.md)
2. Updates all engine modules
3. Ticks all active GameObject components
4. Calls `Update()` on all active GameObjects via [GameObject](../gameobjects-and-components/gameobject.md)
5. Evaluates collider interactions
6. Updates the camera
7. Renders all sprite layers via the Renderer
8. Renders dirty UI elements via [UI Manager](ui-manager.md)
9. Flushes the framebuffer to the display over SPI
10. Sleeps for any remaining time to hit the target frame time

Frame timing is controlled by `TARGET_FRAME_TIME_US` in [Settings](../settings.md).

---

## Subsystems

| Member           | Type           | Access via         |
|------------------|----------------|--------------------|
| `m_renderer`     | `Renderer`     | Internal           |
| `m_Camera`       | `Camera`       | `MainCamera()`     |
| `m_InputManager` | `InputManager` | `Input()`          |
| `m_UIManager`    | `UIManager`    | `UI()`             |
| `m_SoundManager` | `SoundManager` | `Sound()`          |
| `m_ColliderManager` | `ColliderManager` | Internal      |

---

## Global Accessors

WolfEngine provides free functions so you can reach any subsystem from anywhere — inside GameObjects, UI callbacks, or helper classes — without needing a reference to the engine instance.

```cpp
Engine();       // WolfEngine instance
MainCamera();   // Camera
Input();        // InputManager
UI();           // UIManager
Sound();        // SoundManager
```

These are the recommended way to access engine systems in game code:

```cpp
void Update() override {
    if (Input().getController(0)->getButtonDown(Button::A))
        transform.position.x += 1;

    MainCamera().setTarget(this);

    UI();  // access UIManager
}
```

---

For other systems, refer to their respective wiki pages.
