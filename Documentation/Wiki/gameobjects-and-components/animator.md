# Animator

The `Animator` component drives a `SpriteRenderer` by cycling through an `Animation`'s `Sprite` frames automatically. Attach it to a `SpriteRenderer` and it handles frame advancement every tick.

---

## Setup

Define your pixel data, create a `Sprite` array, wrap it in an `Animation`, then attach an `Animator` to your `SpriteRenderer`:

```cpp
constexpr uint8_t walkPixels0[7 * 7] = { ... };
constexpr uint8_t walkPixels1[7 * 7] = { ... };
constexpr uint8_t walkPixels2[7 * 7] = { ... };
constexpr uint8_t walkPixels3[7 * 7] = { ... };

constexpr Sprite WALK_FRAMES[] = {
    Sprite::Create(walkPixels0),
    Sprite::Create(walkPixels1),
    Sprite::Create(walkPixels2),
    Sprite::Create(walkPixels3),
};

constexpr Animation WALK = Animation::Create(WALK_FRAMES);

class Player : public GameObject {
public:
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &WALK_FRAMES[0], PALETTE_WARM, RenderLayer::Player);
    Animator       animator       = Animator(&spriteRenderer, WALK, 8);
};
```

The engine ticks the animator automatically every frame.

> **Note:** Pixel arrays and `Sprite` arrays must be declared as separate `constexpr` variables. Inline initializer lists are not supported.

---

## Constructor

```cpp
Animator(SpriteRenderer*  mySpriteRenderer,
         const Animation& animation,
         uint8_t          frameDuration = 8);
```

| Parameter         | Description                              |
|-------------------|------------------------------------------|
| `mySpriteRenderer`| The `SpriteRenderer` this animator controls |
| `animation`       | The `Animation` to play                   |
| `frameDuration`   | How many engine ticks each frame is shown for. Defaults to 8 |

---

## Controls

### Switching Animations
Swap the active animation at runtime — for example switching from walk to jump:

```cpp
animator.setAnimation(JUMP);
```

This resets the frame counter and starts the new animation from the beginning.

### Jumping to a Frame
Jump to a specific frame immediately — resets the tick counter:

```cpp
animator.setFrame(0); // go back to first frame
```

### Playback Speed
Change how many ticks each frame is shown for:

```cpp
animator.setFrameDuration(4);  // faster
animator.setFrameDuration(16); // slower
```

### Pause and Resume
```cpp
animator.pause();
animator.resume();
```

---

## Getters

```cpp
animator.getCurrentFrame(); // returns current frame index
animator.isPaused();        // returns true if paused
```

---

## Frame Duration and Speed

`frameDuration` is measured in engine ticks, not milliseconds. At 30fps:

| frameDuration | Frames per second |
|---------------|-------------------|
| 2             | 15 fps            |
| 4             | 7.5 fps           |
| 8             | 3.75 fps          |
| 15            | 2 fps             |

Adjust based on how snappy you want the animation to feel.

---

## Memory

All pixel data, `Sprite` arrays, and `Animation` objects should be declared `constexpr` so they live in flash. The `Animator` itself only stores pointers and counters — a few bytes of RAM regardless of frame count.
