# UIManager

The UIManager owns and renders all UI elements. Like the [Renderer](../core-systems/renderer.md), it is an internal system driven by WolfEngine — but unlike the Renderer, you do interact with it directly to register your UI elements before the game starts.

It is accessed through the centralized instance:

```cpp
UI().setElements(uiElements);
```

❗ **IMPORTANT:** The `uiElements` array needs to end with `nullptr`:
```cpp
static BaseUIElement* uiElements[] = { &scoreLabel, &healthLabel, nullptr };
```

---

## Setup

UI elements must be registered with the manager **after** `StartEngine()` but **before** `StartGame()`. This is the only step you need to do manually — everything else is handled by the engine.

```cpp
#include "WolfEngine/WolfEngine.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"

// 1. Declare transforms in flash — position never changes
static const UITransform scoreTf  = { 4,  4, true };   // top-left of UI region
static const UITransform healthTf = { 4, 18, true };   // second row

// 2. Declare state in RAM — text and color can change at runtime
static UILabelState scoreState  = { "Score: 0" };
static UILabelState healthState = { "HP: 100"  };

// 3. Create label elements
static UILabel scoreLabel (&scoreTf,  &scoreState);
static UILabel healthLabel(&healthTf, &healthState);

// 4. Collect into an array
static BaseUIElement* uiElements[] = { &scoreLabel, &healthLabel, nullptr };  // <-- MUST end with nullptr

extern "C" void app_main() {
    WolfEngine& engine = WolfEngine::getInstance();

    engine.StartEngine();

    // ✅ Register UI here — after StartEngine, before StartGame
    engine.ui.setElements(uiElements);

    engine.StartGame();
}
```

---

## setElements()

```cpp
engine.ui.setElements(BaseUIElement** elements);
```

Registers the UI element array with the manager. Also wires each element's back-pointer to the manager so dirty flag propagation works correctly. Call this once before `StartGame()`.

> **Warning:** The array must end with `nullptr` or it won't work. This is how the engine knows how many items are in the array.

---

## markAllDirty()

```cpp
engine.ui.markAllDirty();
```

Forces all registered elements to redraw on the next frame. Useful after a scene transition or when you want to guarantee the UI is fully redrawn — for example after changing many labels at once.

---

## How Rendering Works

The UIManager uses a dirty flag system to avoid redrawing the UI every frame. When any label changes — text, color, visibility — it marks itself dirty and sets the manager's global dirty flag. The renderer checks this flag once per frame:

- **Dirty** — UIManager redraws all dirty elements into the framebuffer and a full screen flush occurs
- **Clean** — UI step is skipped entirely, only the game region is flushed

This means a static HUD costs nothing in SPI bandwidth per frame. See [Renderer](../core-systems/renderer.md) for more on how the flush optimization works.

---

## UI Screen Region

The UI region starts at `RENDER_UI_START_ROW` (default 128) and extends to the bottom of the screen. Elements declared with `anchor = true` in their `UITransform` use Y coordinates relative to this row.

```
anchor = true,  y = 4  →  draws at screen row 132  (128 + 4)
anchor = false, y = 4  →  draws at screen row 4
```

See [Settings](../settings.md) to configure `RENDER_UI_START_ROW`.
