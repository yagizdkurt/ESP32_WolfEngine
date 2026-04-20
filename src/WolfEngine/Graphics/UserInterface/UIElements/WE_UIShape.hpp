#pragma once

#include <stdint.h>
#include "Base/WE_BaseUIElement.hpp"

// Basic shape kinds supported by UIShape.
enum class UIShapeType : uint8_t { Rectangle, HLine, VLine, };

// =============================================================
//  WE_UIShape
//  A shape UI element. Inherits layout fields from
//  BaseUIElement (x, y, w, h, layer, anchor).
//
//  USAGE (constructor-based):
//
//  static UIShape divider(
//      4, 11, 120, 1,
//      UIShapeType::HLine,
//      true,
//      PL_GS_White,
//      PALETTE_GRAYSCALE,
//      0,
//      UIAnchor::TopLeft
//  );
//
//  // Update at runtime
//  divider.setLength(80);
//  divider.setColorIndex(2);
// =============================================================
class UIShape : public BaseUIElement {
public:
    UIShapeType     shape      = UIShapeType::Rectangle;
    bool            filled     = true;
    uint8_t         colorIndex = PL_GS_White;
    const uint16_t* palette    = PALETTE_GRAYSCALE;

    UIShape(int16_t x, int16_t y, int16_t w, int16_t h,
            UIShapeType shape = UIShapeType::Rectangle,
            bool filled = true,
            uint8_t colorIndex = PL_GS_White,
            const uint16_t* palette = PALETTE_GRAYSCALE,
            uint8_t layer = 0,
            UIAnchor anchor = UIAnchor::Center);

    void draw(UIManager& mgr, int16_t offX = 0, int16_t offY = 0) override;

    void setSize(int16_t width, int16_t height);
    void setColorIndex(uint8_t index);
    void setShape(UIShapeType shape);
    void setFilled(bool filled);
    void setPalette(const uint16_t* palette);
    // Convenience for lines: sets w for HLine, h for VLine.
    void setLength(int16_t length);

    int16_t     getWidth()      const;
    int16_t     getHeight()     const;
    uint8_t     getColorIndex() const;
    bool        isFilled()      const;
    UIShapeType getShape()      const;
};
