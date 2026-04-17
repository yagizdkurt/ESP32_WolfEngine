#pragma once
#include <stdint.h>
#include "Base/WE_BaseUIElement.hpp"


// {width , height , background enabled, background color}
struct UIPanelState {
    int16_t  width             = 0;
    int16_t  height            = 0;
    bool     backgroundEnabled = false;
    uint16_t backgroundColor   = 0;
};

// =============================================================
//  UIPanel
//  A container UI element that groups child BaseUIElements and
//  renders them at positions relative to the panel's own origin.
//  Optionally fills a solid background before drawing children.
//
//  Children are positioned relative to the panel — their
//  UITransform x/y are treated as offsets from the panel origin.
//  The panel temporarily adjusts each child's transform during
//  draw, then restores it. Children must NOT store their
//  transform pointer across frames.
//
//  USAGE:
//
//  // Flash
//  static const UITransform panelTf = { 0, 108, false };
//
//  // RAM
//  static UIPanelState panelState = { 128, 20, true, 0x0000 };
//
//  // Children (null-terminated)
//  static BaseUIElement* children[] = { &myLabel, &myShape, nullptr };
//
//  // Element
//  static UIPanel panel(&panelTf, &panelState, children);
//
//  // Register with engine (panel registers, not children individually)
//  static BaseUIElement* uiElements[] = { &panel, nullptr };
//  engine.ui.setElements(uiElements);
//
//  // Update at runtime
//  panel.setBackgroundColor(0xF800);
//  panel.setBackgroundEnabled(true);
// =============================================================
class UIPanel : public BaseUIElement {
public:
    UIPanel(const UITransform* transform, UIPanelState* state, BaseUIElement** children);

    void draw(UIManager& mgr) override;
    void setSize(int16_t width, int16_t height);
    void setBackgroundEnabled(bool enabled);
    void setBackgroundColor(uint16_t color);

    BaseUIElement** getChildren() const;

private:
    UIPanelState*   m_state;
    BaseUIElement** m_children;

    void drawBackground(UIManager& mgr, int16_t panelX, int16_t panelY);
    void syncChildManagers();
    void propagateDirtyToChildren();
};
