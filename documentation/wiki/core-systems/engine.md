# Engine

`WolfEngine` is the core singleton class. It owns the major subsystems, runs the game loop, and exposes global accessor helpers so gameplay code can reach systems without passing references around.

---

## Startup Flow

```cpp
#include "WolfEngine/WolfEngine.hpp"

extern "C" void app_main() {
    Engine().StartEngine();

    // Register UI after StartEngine(), before StartGame().
    UI().setElements(uiElements); // must be null-terminated

    GameObject::Create<Player>();

    Engine().StartGame(); // blocks until RequestQuit()
}
```

`StartEngine()` currently performs:

1. I2C initialization
2. Input manager initialization
3. Renderer initialization
4. Camera initialization
5. UI manager initialization
6. Sound manager initialization
7. Module system initialization
8. Default UI registration with a `nullptr`-terminated empty list

`StartGame()`:

1. Marks the engine as running
2. Calls `Start()` once on already-created objects
3. Enters the main loop until `RequestQuit()` is called

---

## Tick Order

Inside the running loop, sound is updated continuously. A full game tick is executed whenever elapsed time reaches `TARGET_FRAME_TIME_US`:

1. Input tick
2. Module update
3. Component tick for active objects (SpriteRenderer submits draw commands here)
4. `Update()` for active objects
5. Collider manager tick
6. Camera follow tick
7. Renderer render/flush (clear -> world pass -> UI pass -> full-screen flush)
8. `WETime::incrementFrameCount()`

Renderer step detail (current):
- World pass: sort + execute currently buffered world commands
- UI pass: `UI().render()` submits UI commands, then sort + execute
- Flush: full-screen flush each frame

Frame pacing is currently done by elapsed-time gating, not by explicit sleep.

Because sprite commands are submitted in step 3 but movement/camera updates happen in steps 4 and 6, sprite rendering uses previous-frame transform/camera state.

---

## Subsystems

| Member               | Type              | Access |
|----------------------|-------------------|--------|
| `m_renderer`         | `Renderer`        | `RenderSys()` |
| `m_Camera`           | `Camera`          | `MainCamera()` |
| `m_InputManager`     | `InputManager`    | `Input()` |
| `m_UIManager`        | `UIManager`       | `UI()` |
| `m_SoundManager`     | `SoundManager`    | `Sound()` |
| `m_ColliderManager`  | `ColliderManager` | Internal |

---

## Global Accessors

```cpp
Engine();
MainCamera();
Input();
UI();
Sound();
RenderSys();
```
