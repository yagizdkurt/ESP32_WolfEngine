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

Inside the running loop, `gameLoop()` runs continuously. Each iteration performs:

1. `SoundManager::update()`
2. `ModuleSystem::FreeUpdate()`
3. A full `gameTick()` only when elapsed time reaches `Settings.render.targetFrameTimeUs`

`gameTick()` phase order:

1. Early phase
    - Input tick
    - `EarlyUpdate()` for active objects
    - `earlyComponentTick()` for active objects
    - `ModuleSystem::EarlyUpdate()`
2. Main phase
    - `Update()` for active objects
    - `componentTick()` for active objects
    - `ModuleSystem::Update()`
3. Late phase
    - `LateUpdate()` for active objects
    - `lateComponentTick()` for active objects
    - `ModuleSystem::LateUpdate()`
    - Camera follow tick
    - Collision handling runs inside `ModuleSystem::LateUpdate()` when `WE_MODULE_COLLISION` is enabled
4. End phase
    - `preRenderComponentTick()` for active objects
    - `ModuleSystem::PreRender()`
    - Renderer render/flush (clear -> world pass -> UI pass -> full-screen flush)
    - `WETime::incrementFrameCount()`

Renderer step detail (current):
- World pass: sort + execute currently buffered world commands
- UI pass: `UI().render()` submits UI commands, then sort + execute
- Flush: full-screen flush each frame

Frame pacing is currently done by elapsed-time gating, not by explicit sleep.

Because render runs at the end phase, movement/camera logic is completed before final flush.

---

## Subsystems

| Member               | Type              | Access |
|----------------------|-------------------|--------|
| `m_renderer`         | `Renderer`        | `RenderSys()` |
| `m_Camera`           | `Camera`          | `MainCamera()` |
| `m_InputManager`     | `InputManager`    | `Input()` |
| `m_UIManager`        | `UIManager`       | `UI()` |
| `m_SoundManager`     | `SoundManager`    | `Sound()` |

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
