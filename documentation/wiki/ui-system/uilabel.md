# UILabel

`UILabel` renders text using the built-in 5x7 font.

---

## Declaration

```cpp
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

// Minimal
static UILabel scoreLabel(4, 4, 96, 7, "Score: 0");

// Full control
static UILabel scoreLabelTopLeft(
    4, 4, 96, 7,
    "Score: 0",
    PL_GS_White,
    PALETTE_GRAYSCALE,
    0,
    UIAnchor::TopLeft
);
```

Register with UI manager before `StartGame()`. See [UI Manager](ui-manager.md).

---

## Constructor

```cpp
UILabel(int16_t x, int16_t y, int16_t w, int16_t h,
    const char* text = "",
    uint8_t colorIndex = PL_GS_White,
    const uint16_t* palette = PALETTE_GRAYSCALE,
    uint8_t layer = 0,
    UIAnchor anchor = UIAnchor::Center);
```

`UILabel` layout fields live on `BaseUIElement` (`x`, `y`, `w`, `h`, `layer`, `anchor`).
The engine resolves anchored screen coordinates internally at draw time.

Current renderer behavior:

- `UILabel::draw()` submits one `DrawCommandType::TextRun` command.
- `rect.width` from `resolveLayout()` is passed as `maxWidth`.
- `maxWidth = 0` means no clipping.
- `maxWidth > 0` clips at command execution time.

---

## Notes

- `UILabelState` was removed from the public UI API.
- Constructor text is copied with clamped `strncpy` and explicit null termination.
- `UILabel` is not an aggregate type, so initialize it through its constructor.

---

## Runtime API

```cpp
scoreLabel.setText("Score: 42");
scoreLabel.setColorIndex(PL_GS_White);

scoreLabel.hide();
scoreLabel.show();

const char* t = scoreLabel.getText();
uint8_t c = scoreLabel.getColorIndex();
bool visible = scoreLabel.isVisible();
```

---

## Text Limits

- Max length is 32 bytes including null terminator
- Printable ASCII (32-126) is supported
- Out-of-range characters are skipped
- Font is fixed-size 5x7 with spacing
