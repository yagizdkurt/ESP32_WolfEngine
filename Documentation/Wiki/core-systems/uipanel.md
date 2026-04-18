# UIPanel

UIPanel is a container UI element. It groups multiple `BaseUIElement` children and draws them relative to the panel origin. It can optionally render a background rectangle behind its children.

---

## Declaration

`UIPanel` requires two parts — a `UITransform` in flash and a `UIPanelState` in RAM.

```cpp
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

// Flash — panel position
static const UITransform panelTf = { 0, 108, false };

// RAM — size and background settings
static UIPanelState panelState = { 128, 20, true, 0x0000 };

// Children (panel-local coordinates)
static BaseUIElement* panelChildren[] = { &myLabel, &myShape, nullptr };

// Panel
static UIPanel panel(&panelTf, &panelState, panelChildren);
```

Then register **only the panel** with the UIManager before `StartGame()`.

```cpp
static BaseUIElement* uiElements[] = { &panel, nullptr };
engine.ui.setElements(uiElements);
```

> **Important:** Child elements must **not** be registered separately.

---

## UIPanelState

Mutable panel state stored in RAM.

```cpp
struct UIPanelState {
    int16_t  width;
    int16_t  height;
    bool     backgroundEnabled;
    uint16_t backgroundColor;
};
```

| Field                | Description                  |
|----------------------|------------------------------|
| `width`              | Panel width in pixels        |
| `height`             | Panel height in pixels       |
| `backgroundEnabled`  | Enables background fill      |
| `backgroundColor`    | RGB565 background color      |

Example:

```cpp
static UIPanelState panelState = { 128, 20, true, 0x0000 };
```

---

## Child Elements

Children are stored in a **null-terminated array**.

```cpp
static BaseUIElement* children[] = {
    &hpLabel,
    &scoreLabel,
    nullptr
};
```

Important rules:

- Children use **panel-local coordinates**
- The panel automatically offsets them when drawing
- The array **must end with `nullptr`**

Children are rendered **in array order**.

---

## Updating at Runtime

### `setSize(int16_t width, int16_t height)`

Changes the panel size.

```cpp
panel.setSize(160, 24);
```

### `setBackgroundEnabled(bool enabled)`

Enables or disables background rendering.

```cpp
panel.setBackgroundEnabled(true);
panel.setBackgroundEnabled(false);
```

### `setBackgroundColor(uint16_t color)`

Changes the background fill color.

```cpp
panel.setBackgroundColor(0xF800); // red
```

---

## Getters

```cpp
panel.getChildren();          // BaseUIElement**
panel.getWidth();             // int16_t
panel.getHeight();            // int16_t
panel.isBackgroundEnabled();  // bool
panel.getBackgroundColor();   // uint16_t
```

---

## Rendering Behavior

When the UI system calls `panel.draw()`:

1. The panel reads its `UITransform` to determine its position.
2. If `backgroundEnabled` is true, it draws the background rectangle.
3. The panel iterates the `children` array.
4. Each child’s transform is **temporarily offset by the panel position**.
5. `child->draw()` is called.
6. The child transform is restored.

This allows children to use **panel-relative coordinates** while rendering in **absolute screen space**.

---

## Full Example

```cpp
static const UITransform panelTf = { 0, 108, false };

static UIPanelState panelState = { 128, 20, true, 0x0000 };

static const UITransform hpTf    = { 4, 4, false };
static const UITransform scoreTf = { 4, 14, false };

static UILabelState hpState    = { "HP: 100" };
static UILabelState scoreState = { "Score: 0" };

static UILabel hpLabel   (&hpTf, &hpState);
static UILabel scoreLabel(&scoreTf, &scoreState);

static BaseUIElement* panelChildren[] = {
    &hpLabel,
    &scoreLabel,
    nullptr
};

static UIPanel panel(&panelTf, &panelState, panelChildren);

static BaseUIElement* uiElements[] = { &panel, nullptr };
```

---

## Notes

- Children render **after the panel background**, so they always appear on top.
- Rendering order of children is **array order**.
- Panel z-order relative to other UI elements depends on the order used in `UIManager::setElements()`.
