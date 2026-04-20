# Render Layers

Layers are settings that users can change according to their game.

---

## RenderLayer

Render layers control draw order within a render pass. Layers are drawn in ascending order - layer 0 is drawn first (bottom), the highest layer is drawn last (top). In the renderer this is encoded into the high byte of `DrawCommand.sortKey`. See [Renderer](../core-systems/renderer.md) for pipeline details.

Important:
- World and UI are executed in separate passes.
- Layer values do not interleave world and UI across pass boundaries.
- UI pass runs after world pass.

---

## ColliderLayer

`CollisionLayer` defines the **collision categories** of game objects. Each layer is a **bitmask**, allowing fast layer-to-layer collision checks using bitwise operations.

---

## Default Layers

```cpp
enum class RenderLayer : uint8_t {
    BackGround = 0,
    World      = 1,
    Entities   = 2,
    Player     = 3,
    FX         = 4,
    MAX_LAYERS   // always last — automatically equals layer count
};

enum class CollisionLayer : uint16_t {
    DEFAULT    = 1 << 0,
    Player     = 1 << 1,
    Enemy      = 1 << 2,
    Wall       = 1 << 3,
    Trigger    = 1 << 4,
    Projectile = 1 << 5
    // user can add more
};
```

---

## Usage

Pass a layer when creating a sprite:

```cpp
SpriteRenderer sr = SpriteRenderer(this, &playerSprite, PALETTE_WARM, RenderLayer::Player);
```

See [Sprite Renderer](../gameobjects-and-components/sprite-renderer.md) for more details.

---

## Customizing Layers

You can rename, add, or remove layers freely to match your game. The only rule is `MAX_LAYERS` must always be the last entry - it is used internally for layer bounds and ordering and will automatically equal the layer count.

```cpp
enum class RenderLayer : uint8_t {
    Background = 0,
    Player     = 1,
    MAX_LAYERS  // always last
};
```

For bitmask layers do not add MAX_LAYERS:

```cpp
enum class CollisionLayer : uint16_t {
    DEFAULT    = 1 << 0,
    BigShac    = 1 << 1,
    Sputni     = 1 << 2,
    Cgurna     = 1 << 3,
};
```

---

## Memory

Render layers themselves do not allocate a per-layer sprite slot table anymore.

Renderer command memory is now primarily:

- `MAX_DRAW_COMMANDS * sizeof(DrawCommand)` for the shared command buffer
- `sizeof(FrameDiagnostics)` for runtime counters

With the default `MAX_DRAW_COMMANDS = 128` on ESP32, this is currently about `128 * 20 + 8 = 2568` bytes for the current command layout plus diagnostics.
