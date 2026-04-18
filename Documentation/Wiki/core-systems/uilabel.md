# UILabel

UILabel is a text label UI element. It displays a string of up to 32 characters in the UI region using the built-in 5x7 bitmap font.

---

## Declaration

UILabel requires two parts — a `UITransform` in flash and a `UILabelState` in RAM.

```cpp
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

// Flash — position never changes
static const UITransform scoreTf = { 52, 12, true };
//                                    x   y   anchor

// RAM — text and color can change at runtime
static UILabelState scoreState = { "Score: 0" };

// Label
static UILabel scoreLabel(&scoreTf, &scoreState);
```

Then register it with the UIManager before `StartGame()`. See [UI Manager](../core-systems/ui-manager.md) for the full setup.

---

## UITransform

Defines where the label is drawn. Stored in flash as `const` — position cannot change at runtime.

```cpp
static const UITransform myTf = { x, y, anchor };
```

| Field    | Type      | Description                                   |
|----------|-----------|-----------------------------------------------|
| `x`      | `int16_t` | Horizontal position in pixels                 |
| `y`      | `int16_t` | Vertical position in pixels                   |
| `anchor` | `bool`    | If `true`, y is relative to `RENDER_UI_START_ROW` |

**Anchor example:**
```
RENDER_UI_START_ROW = 128
anchor = true,  y = 12  →  draws at screen row 140
anchor = false, y = 12  →  draws at screen row 12
```

Using `anchor = true` is recommended for UI labels — it keeps your Y coordinates relative to the UI region regardless of the `RENDER_UI_START_ROW` setting.

---

## UILabelState

Mutable state stored in RAM. Text, color index, and palette can all change at runtime.

```cpp
struct UILabelState {
    char            text[32];
    uint8_t         colorIndex;  // defaults to PL_GS_White
    const uint16_t* palette;     // defaults to PALETTE_GRAYSCALE
};
```

Defaults are applied automatically so the simplest declaration is just:

```cpp
static UILabelState myState = { "Hello" };  // white text, grayscale palette
```

Override color and palette explicitly if needed:

```cpp
static UILabelState myState = { "Score: 0", PL_WM_Amber, PALETTE_WARM };
```

---

## Updating at Runtime

### `setText(const char* text)`
Updates the label text. Marks the label dirty so it redraws on the next frame.

```cpp
scoreLabel.setText("Score: 42");
```

### `setColorIndex(uint8_t index)`
Changes the color by swapping the palette index.

```cpp
scoreLabel.setColorIndex(PL_GS_MidGray);  // dim the label
scoreLabel.setColorIndex(PL_GS_White);    // restore full brightness
```

### `show()` / `hide()`
Controls visibility without removing the element from the manager.

```cpp
scoreLabel.hide();  // invisible — skipped by renderer
scoreLabel.show();  // visible again
```

---

## Getters

```cpp
scoreLabel.getText();        // const char*
scoreLabel.getColorIndex();  // uint8_t
scoreLabel.isVisible();      // bool
scoreLabel.isDirty();        // bool
```

---

## Full Example

```cpp
static const UITransform  hpTf    = { 4,  4, true };
static const UITransform  scoreTf = { 4, 18, true };

static UILabelState hpState    = { "HP: 100",  PL_CL_Green,  PALETTE_COOL };
static UILabelState scoreState = { "Score: 0", PL_GS_White,  PALETTE_GRAYSCALE };

static UILabel hpLabel   (&hpTf,    &hpState);
static UILabel scoreLabel(&scoreTf, &scoreState);

static BaseUIElement* uiElements[] = { &hpLabel, &scoreLabel, nullptr };

// In game code — update labels when values change:
void onScoreChanged(int newScore) {
    char buf[32];
    snprintf(buf, 32, "Score: %d", newScore);
    scoreLabel.setText(buf);
}

void onPlayerHurt(int hp) {
    char buf[32];
    snprintf(buf, 32, "HP: %d", hp);
    hpLabel.setText(buf);

    if (hp < 30) hpLabel.setColorIndex(PL_WM_PureRed);   // red when low
    else         hpLabel.setColorIndex(PL_CL_Green);      // green when healthy
}
```

---

## Text Limits

- Maximum text length is **32 characters** including the null terminator
- Only printable ASCII characters (32–126) are supported
- Characters outside this range are rendered as `?`
- Font is fixed 5x7 pixels per character with 1px spacing
