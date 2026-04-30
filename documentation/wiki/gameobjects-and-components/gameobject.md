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

Prefer putting gameplay initialization in `Start()` instead of constructors. Constructors/destructors are still part of normal C++ object lifetime, but heavy engine-dependent setup should happen in `Start()` once subsystems are initialized.

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

WolfEngine keeps a fixed-size registry (`Settings.limits.maxGameObjects` in `WE_Settings.hpp`) and tracks active object pointers there.

The registry is finite. If your game creates many temporary objects, design with this cap in mind.

If your code spawns many objects (projectiles, particles, enemies), monitor usage against `Settings.limits.maxGameObjects`.

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
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, RenderLayer::Player);
    // Wrap the generated raw animation asset with playback params and pass to Animator
    constexpr WE_Animation WALK = { &Assets::PLAYER_WALK, 4, true };
    Animator       animator       = Animator(&spriteRenderer, &WALK);
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
