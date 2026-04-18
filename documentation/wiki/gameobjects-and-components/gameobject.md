# GameObject

Game Objects are the center of WolfEngine. They are the characters, enemies, managers, map controllers—basically anything that interacts with game elements. GameObjects provide a transform, a lifecycle with `Start()` and `Update()`, and a factory system for safe creation and destruction. Most of your real objects will inherit from the GameObject class, so it's important to understand how to use the GameObject system.

---

## Defining a GameObject

Almost all of your managers, objects, enemies, etc. will inherit GameObject. You can override `Start()` and `Update()` to implement your own systems inside the code.

Example of a player starting at 64,64 pixels:

```cpp
#include "WolfEngine/WolfEngine.hpp"

class Player : public GameObject {
public:
    void Start() override {
        transform.position = { 64, 64 };
    }

    void Update() override {
        // ...
    }
};
```

---

## Constructor and Destructor

**Never add a constructor or destructor to your derived class.** Construction and destruction are specifically handled by the engine for registry purposes. If you add your own constructor or destructor, you can break these systems. Use `Start()` for initialization and, if you need cleanup, create an `End()` or similar function to handle quit states before calling destroy.

```cpp
// ✅ Correct
void EndThisObjectOrSmth(){
    // Your deconstruction here
    GameObject::DestroyGameObject(enemy);
}

// ❌ Wrong — registry still holds a dangling pointer
~Enemy(){
    // stuff
}
```

---

## Creating a GameObject

Always use `GameObject::Create<T>()` to instantiate objects. **Never use `new` directly**—doing so bypasses the engine registry and the object will not receive `Update()` calls, `Start()` won't be called, and its lifetime won't be managed.

```cpp
// ✅ Correct
Player* player = GameObject::Create<Player>();

// ❌ Wrong — bypasses the engine entirely
Player* player = new Player();
```

---

## GameObject Registry

Since WolfEngine runs on embedded systems, it doesn't have the storage capacities that a PC has. Thus, it handles object storage differently—small RAM, no heap. There is a cap for the number of game objects you can spawn. The cap can be increased, but we suggest leaving it at 64 or 128. It's defined in WE_Settings.hpp ([Settings](../settings.md)).

The registry is finite, and if you try to create more game objects than the registry has slots for, you will get a nullptr.

`Create<T>()` returns `nullptr` if the registry is full—always check the return value if your game is creating many objects.

So, if your code spawns a lot of game objects, make sure you check if its pointer actually exists. This is up to you since this is an embedded game engine and not C# or Python 😄

---

## Lifecycle

### `Start()`
Called once by the engine immediately after the object is created. Use this for initialization that depends on other systems being ready—setting position, assigning camera target, etc.

```cpp
void Start() override {
    transform.position = { 32, 32 };
}
```

> Use `Start()` instead of a constructor for initialization. **Never set a constructor for a GameObject!**

### `Update()`
Called every frame by the engine for all active objects. Put your game logic here—movement, input handling, collision checks.

```cpp
void Update() override {
    transform.position.x += 1;  // move right every frame
}
```

---

## Adding Components

Components define what a GameObject does and how it looks. They are declared as public members and wire themselves up automatically—no registration calls needed from your side.

Example:

```cpp
class Player : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, PALETTE_WARM, RenderLayer::Player);
    Animator       animator       = Animator(&spriteRenderer, WALK_FRAMES, 4);
};
```

You can check built-in components in the wiki.

---

## isActive

Set `isActive = false` to pause an object without destroying it. It will stop receiving `Update()` calls until re-enabled.

```cpp
enemy->isActive = false;  // pause during cutscene
enemy->isActive = true;   // resume after cutscene
```

---

## Destroying a GameObject

Always use the provided destroy functions. **Never call `delete` directly**—it leaves a dangling pointer in the engine registry.

**From outside the object:**
```cpp
// ✅ Correct
GameObject::DestroyGameObject(enemy);

// ❌ Wrong — registry still holds a dangling pointer
delete enemy;
```

**From within the object itself:**
```cpp
void Update() override {
    if (hitWall) destroyGameObject();  // ✅ safe self-destruction
}
```
