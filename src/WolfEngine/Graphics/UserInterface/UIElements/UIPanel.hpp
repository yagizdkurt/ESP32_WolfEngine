#pragma once
#include <stdint.h>
#include "Base/WE_BaseUIElement.hpp"

// =============================================================
//  UIPanel
//  A container UI element that groups child BaseUIElements and
//  renders them offset by the panel's resolved screen position.
//  Optionally fills a solid background before drawing children.
//
//  Children are drawn with the panel's top-left corner as their
//  coordinate origin — their x/y are treated as panel-relative
//  offsets. No transform patching; offsets are passed directly
//  to each child's draw() call.
//
//  USAGE (constructor-based):
//
//  static BaseUIElement* panelChildren[] = { &myLabel, &myShape, nullptr };
//
//  static UIPanel panel(
//      0, -24, 128, 24,
//      panelChildren,
//      0x0000,
//      true,
//      1,
//      UIAnchor::BotLeft
//  );
//
//  // Register panel only — not children individually
//  static BaseUIElement* uiElements[] = { &panel, nullptr };
//  UI().setElements(uiElements);
// =============================================================
class UIPanel : public BaseUIElement {
public:
    uint16_t       background        = 0x0000;
    bool           backgroundEnabled = true;
    BaseUIElement* children[WE_UI_MAX_PANEL_CHILDREN] = {};

    UIPanel(int16_t x, int16_t y, int16_t w, int16_t h,
            BaseUIElement** ch = nullptr,
            uint16_t background = 0x0000,
            bool backgroundEnabled = true,
            uint8_t layer = 0,
            UIAnchor anchor = UIAnchor::Center);

    void draw(UIManager& mgr, int16_t offX = 0, int16_t offY = 0) override;

    void setSize(int16_t width, int16_t height);
    void setBackgroundEnabled(bool enabled);
    void setBackgroundColor(uint16_t color);
};
