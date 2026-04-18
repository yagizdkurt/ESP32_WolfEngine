# Directly Accessing the Framebuffer

WolfEngine exposes the raw framebuffer for cases where you need pixel-level control. This bypasses the sprite rendering system entirely and lets you write directly to the screen.

```cpp
uint16_t* fb = RenderSys().getCanvas();
```

Call this in your `GameObject` — typically in `Update()` — and write pixels directly:

```cpp
void Update() override {
    uint16_t* fb = RenderSys().getCanvas();
    fb[y * RENDER_SCREEN_WIDTH + x] = 0xFFFF; // white pixel at (x, y)
}
```

Pixels are in **RGB565** format (16-bit). The framebuffer is laid out row by row — index `y * RENDER_SCREEN_WIDTH + x` maps to the pixel at column `x`, row `y`.

> **Warning:** You are writing directly to memory. There are no bounds checks. Writing outside `0` to `(RENDER_SCREEN_WIDTH * RENDER_SCREEN_HEIGHT - 1)` will corrupt memory. This is your responsibility.

---

## Render Settings for Manual Drawing

If you are managing the framebuffer yourself, you may want to adjust these settings in `WE_Settings.hpp`:

```cpp
// Set to false to disable the sprite rendering system entirely.
// Useful if you are drawing everything manually.
spriteSystemEnabled = true,

// Set to false to disable automatic framebuffer clearing each frame.
// Useful if you want to retain the previous frame's contents.
cleanFramebufferEachFrame = true,
```

Disabling `spriteSystemEnabled` stops `SpriteRenderer` from submitting sprite commands. The renderer still runs its command execution, UI render, and display flush path each frame. Disabling `cleanFramebufferEachFrame` means pixels you wrote last frame will persist unless you overwrite them yourself.
