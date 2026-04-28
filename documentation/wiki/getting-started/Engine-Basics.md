# Engine Basics

This page explains the small pieces you need to get a WolfEngine project running cleanly: where your code should live, what `main.cpp` does, and how the engine learns about your game objects.

---

## The Short Version

**Remember this flow:**

| Step | What happens |
|:-----|:-------------|
| 1 | `Engine().StartEngine()` initializes the engine systems |
| 2 | You create your GameObjects and other game-side objects |
| 3 | `Engine().StartGame()` starts the main loop and never returns |

That means your setup code happens before `StartGame()`, and your gameplay code lives inside your own classes.

## Folder Structure Tips

WolfEngine keeps engine code and game code separate on purpose.

| Folder | Use it for |
|:-------|:-----------|
| `src/WolfEngine/` | Engine source code itself |
| `src/GameCode/` | Your game-specific classes, objects, systems... |
| `src/main.cpp` | Entry point that starts the engine and creates the first game objects |
| `src/GeneratedAssets/` | Auto-generated assets created by the asset pipeline |

Put your own gameplay classes in `src/GameCode/`

---

## What `main.cpp` Does

`main.cpp` is the entry point. Its job is to start the engine, create the first game objects, and hand control to the engine loop.

Current shape:

```cpp
#include "WolfEngine/WolfEngine.hpp"

extern "C" void app_main() {
    Engine().StartEngine();

    Engine().StartGame();
}
```

In a real game, you usually add your own headers and create your objects before `StartGame()`.

Example:

```cpp
#include "WolfEngine/WolfEngine.hpp"
#include "GameCode/MyPlayer.hpp"
#include "GameCode/MyWorld.hpp"

extern "C" void app_main() {
    Engine().StartEngine();

    GameObject::Create<MyWorld>();
    GameObject::Create<MyPlayer>();

    Engine().StartGame();
}
```

---

## What `StartEngine()` Does

`Engine().StartEngine()` is the one-time initialization step. It initializes the engine itself. Use this function before creating your gameplay objects.

## What `StartGame()` Does

`Engine().StartGame()` starts the actual game loop.

Once it starts:

- the engine begins ticking frames
- `Start()` is called on your created objects
- `Update()` runs every frame
- rendering and flushing are handled automatically

This call does not return during normal gameplay. That is why your setup code must happen before it.

## Where To Write Your Own Code

The cleanest approach is to put your own game code in `src/GameCode/`.

## How The Engine Sees Your Objects

The engine only knows about objects you create before `StartGame()`.

That usually means:

1. Define your class in `src/GameCode/`.
2. Include that header in `main.cpp`.
3. Create the object with `GameObject::Create<T>()`.
4. Start the engine loop.

Example:

```cpp
GameObject::Create<MyPlayer>();
GameObject::Create<MyHUD>();
```

If you never create an object, the engine will not update it or render it.

If you create a `GameObject` after `StartGame()` has already taken over, that can still work in many cases, but the simplest and safest pattern is to create your starting objects before the game loop begins.

---

## Practical Startup Pattern

This is the recommended pattern for most projects:

| Order | What to do |
|:------|:-----------|
| 1 | Include `WolfEngine/WolfEngine.hpp` and your game headers |
| 2 | Call `Engine().StartEngine()` |
| 3 | Create your starting GameObjects |
| 4 | Initialize any game state that needs to exist before the first frame |
| 5 | Call `Engine().StartGame()` |

In other words: configure first, create objects next, run last.

---

## A Small Example

```cpp
#include "WolfEngine/WolfEngine.hpp"
#include "GameCode/MyPlayer.hpp"
#include "GameCode/MyLevel.hpp"

extern "C" void app_main() {
    Engine().StartEngine();

    GameObject::Create<MyLevel>();
    GameObject::Create<MyPlayer>();

    Engine().StartGame();
}
```

And inside `src/GameCode/MyPlayer.hpp`:

```cpp
#pragma once
#include "WolfEngine/WolfEngine.hpp"

class MyPlayer : public GameObject {
public:
    void Start() override {
        transform.position = { 64, 64 };
    }

    void Update() override {
        // movement, input, gameplay, etc.
    }
};
```

---