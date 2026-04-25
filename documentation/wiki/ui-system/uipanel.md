# UIPanel

`UIPanel` is a container UI element. It can draw a background rectangle and then draw child `BaseUIElement`s relative to the panel position.

---

## Declaration

```cpp
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

static UIElementRef panelChildren[] = { myLabel, myShape };

static UIPanel panel(
    0, -20, 128, 20,
    panelChildren,
    0x0000,
    true,
    1,
    UIAnchor::BotLeft
);
```

Register only the panel in the top-level UI list:

```cpp
static UIElementRef uiElements[] = { panel };
UI().setElements(uiElements);
```

Do not register panel children separately in `uiElements`.

---

## Constructor

```cpp
UIPanel(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t background = 0x0000,
        bool backgroundEnabled = true,
        uint8_t layer = 0,
        UIAnchor anchor = UIAnchor::Center);

template <size_t N>
UIPanel(int16_t x, int16_t y, int16_t w, int16_t h,
        const UIElementRef (&ch)[N],
        uint16_t background = 0x0000,
        bool backgroundEnabled = true,
        uint8_t layer = 0,
        UIAnchor anchor = UIAnchor::Center);
```

---

## Child List Rules

1. Constructor input `ch` is a fixed-size non-null `UIElementRef[]` list.
2. Constructor copies children into fixed storage: `children[Settings.limits.maxPanelChildren]`.
3. Render order follows `children[]` index order.
4. Register only the panel in top-level `UI().setElements(...)`; do not also register children.

Bound rule:

- `N` must be `<= Settings.limits.maxPanelChildren` (compile-time checked)
- `nullptr` child entries are not allowed

---

## Runtime API

```cpp
panel.setSize(160, 24);
panel.setBackgroundEnabled(true);
panel.setBackgroundColor(0xF800);
```

---

## Draw Behavior

When `UIPanel::draw()` runs:

1. Resolve panel position from current layout fields.
2. Submit a `FillRect` command for panel background (if enabled and non-zero size).
3. Iterate registered children in insertion order.
4. Temporarily patch child layer to `panel_layer + 1` so children sort above panel background.
5. Call child `draw(...)` with `offX/offY` equal to the panel screen position.
6. Restore child layer.

This gives panel-local positioning without transform-pointer patching.

Note: `UIPanelState` and `getChildren()` are no longer part of the public API.
