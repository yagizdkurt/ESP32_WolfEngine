#pragma once

#include <stdint.h>
#include "WE_BaseUIElement.hpp"

// Basic shape kinds supported by UIShape.
enum class UIShapeType : uint8_t { Rectangle, HLine, VLine, };

// Mutable state for UIShape.
struct UIShapeState {
    int16_t         width      = 0;
    int16_t         height     = 0;
    uint8_t         colorIndex = PL_GS_White;
    const uint16_t* palette    = PALETTE_GRAYSCALE;
    UIShapeType     shape      = UIShapeType::Rectangle;
    bool            filled     = true;
};

// =============================================================
//  WE_UIShape
//  A shape UI element. Inherits position, visibility,
//  and dirty tracking from BaseUIElement.
//
//  USAGE:
//
//  // Flash - never changes
//  constexpr UITransform transform = { 8, 10, true };  // anchor=true -> y=128+10=138
//
//  // RAM - changes at runtime
//  UIShapeState state = { 40, 12, PL_GS_White, PALETTE_GRAYSCALE, UIShapeType::Rectangle, true };
//
//  // Element
//  static UIShape panel(&transform, &state);
//
//  // Update at runtime
//  panel.setSize(48, 14);
//  panel.setColorIndex(2);
//  panel.setFilled(false);
// =============================================================
class UIShape : public BaseUIElement {
public:
    // Create a shape element bound to transform and mutable state.
    UIShape(const UITransform* transform, UIShapeState* state);

    // Draw the configured shape.
    void draw(UIManager& mgr) override;

    // Set shape size and mark dirty.
    void setSize(int16_t width, int16_t height);
    // Set palette color index and mark dirty.
    void setColorIndex(uint8_t index);
    // Select which shape primitive to draw.
    void setShape(UIShapeType shape);
    // Toggle filled/outline rendering for rectangles.
    void setFilled(bool filled);
    // Set the active palette and mark dirty.
    void setPalette(const uint16_t* palette);
    // Convenience for lines: sets width for HLine, height for VLine.
    void setLength(int16_t length);

    // --- Const getters ---
    int16_t     getWidth()      const;
    int16_t     getHeight()     const;
    uint8_t     getColorIndex() const;
    bool        isFilled()      const;
    UIShapeType getShape()      const;

private:
    UIShapeState* m_state;
};
