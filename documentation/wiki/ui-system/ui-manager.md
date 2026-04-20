# UIManager

`UIManager` owns the top-level UI element list and drives UI command submission each frame.

You access it through:

```cpp
UI().setElements(uiElements);
```

The element list must be null-terminated.

```cpp
static BaseUIElement* uiElements[] = { &scoreLabel, &healthLabel, nullptr };
```

---

## Setup

Register UI elements after `StartEngine()` and before `StartGame()`.

```cpp
#include "WolfEngine/WolfEngine.hpp"
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

static UILabel scoreLabel(4, 4, 96, 7, "Score: 0", PL_GS_White, PALETTE_GRAYSCALE, 0, UIAnchor::TopLeft);
static UILabel healthLabel(4, 16, 96, 7, "HP: 100", PL_GS_White, PALETTE_GRAYSCALE, 0, UIAnchor::TopLeft);

static BaseUIElement* uiElements[] = { &scoreLabel, &healthLabel, nullptr };

extern "C" void app_main() {
    Engine().StartEngine();
    UI().setElements(uiElements);
    Engine().StartGame();
}
```

---

## setElements

```cpp
UI().setElements(BaseUIElement** elements);
```

What it does:

1. Stores the list and counts entries until `nullptr`
2. Wires each element to the manager pointer
3. Assigns each element draw metadata (`m_drawOrder`, `m_layer`)
4. Marks UI dirty if renderer/framebuffer has already been initialized

---

## Dirty Behavior

`BaseUIElement::markDirty()` sets a manager-level dirty flag.

At render time:

- Renderer currently calls `UI().render()` every frame.
- `UIManager::render()` iterates all registered elements and calls `draw(...)` on each.
- Elements submit `DrawCommand` objects (`FillRect`, `Line`, `Circle`, `TextRun`) via `RenderSys().submitDrawCommand(...)`.

The dirty flag still exists and is useful for future optimization/caching, but it no longer gates whether UI render is invoked.

---

## Layout Notes

Game code sets UI layout through constructor arguments (`x`, `y`, `w`, `h`, `layer`, `anchor`).

Use anchors (`TopLeft`, `BotCenter`, `Center`, etc.) to place elements relative to screen edges or center.
The engine still resolves final screen rectangles with `UITransform` internally in `BaseUIElement::resolveRect()`.

Note: `UILabel`, `UIShape`, and `UIPanel` are not aggregates (virtual base methods), so use constructors for initialization.
