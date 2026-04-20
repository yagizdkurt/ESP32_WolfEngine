#pragma once
#include <stdint.h>
#include "Base/WE_BaseUIElement.hpp"

#define WE_UI_LABEL_MAX_LEN 32

// =============================================================
//  WE_UILabel
//  A text label UI element. Inherits layout fields from
//  BaseUIElement (x, y, w, h, layer, anchor).
//
//  USAGE (constructor-based):
//
//  static UILabel scoreLabel(
//      52, 12, 60, 8,
//      "Score: 0",
//      PL_GS_White,
//      PALETTE_GRAYSCALE,
//      0,
//      UIAnchor::TopLeft
//  );
//
//  // Update at runtime
//  scoreLabel.setText("Score: 42");
//  scoreLabel.setColorIndex(3);
// =============================================================
class UILabel : public BaseUIElement {
public:
    char            text[WE_UI_LABEL_MAX_LEN] = {};
    uint8_t         colorIndex                = PL_GS_White;
    const uint16_t* palette                   = PALETTE_GRAYSCALE;

    UILabel(int16_t x, int16_t y, int16_t w, int16_t h,
            const char* text = "",
            uint8_t colorIndex = PL_GS_White,
            const uint16_t* palette = PALETTE_GRAYSCALE,
            uint8_t layer = 0,
            UIAnchor anchor = UIAnchor::Center);

    void draw(UIManager& mgr, int16_t offX = 0, int16_t offY = 0) override;

    void setText(const char* text);
    void setColorIndex(uint8_t index);

    const char* getText()       const;
    uint8_t     getColorIndex() const;
};
