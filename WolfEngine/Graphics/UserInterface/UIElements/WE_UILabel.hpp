#pragma once
#include <stdint.h>
#include "WE_BaseUIElement.hpp"

// =============================================================
//  WE_UILabel
//  A text label UI element. Inherits position, visibility,
//  dirty tracking, and palette from BaseUIElement.
//
//  USAGE:
//
//  // Flash - never changes
//  constexpr UITransform transform = { 52, 12, true };  // anchor=true -> y=128+12=140
//
//  // RAM - changes at runtime
//  UILabelState state = { "Score: 0", 1 };   // text, color index
//
//  // Element
//  UILabel scoreLabel(&transform, &state, PALETTE_GRAYSCALE);
//
//  // Update at runtime
//  scoreLabel.setText("Score: 42");
//  scoreLabel.setColorIndex(3);
// =============================================================

#define WE_UI_LABEL_MAX_LEN 32

// --- Stored in RAM (mutable) ---
struct UILabelState {
    char    text[WE_UI_LABEL_MAX_LEN];
    uint8_t colorIndex = PL_GS_White; // default to white
    const uint16_t* palette = PALETTE_GRAYSCALE; //default
};

// =============================================================
class UILabel : public BaseUIElement {
public:
    // Create a label bound to transform and mutable state.
    UILabel(const UITransform* transform, UILabelState* state);

    // Draw label text using current state values.
    void draw(UIManager& mgr) override;

    // Update label text (clamped to max length) and mark dirty.
    void setText(const char* text);
    // Update palette color index and mark dirty.
    void setColorIndex(uint8_t index);

    // Read current label text.
    const char* getText() const;
    // Read current palette color index.
    uint8_t getColorIndex() const;

private:
    UILabelState* m_state;
};
