#pragma once
#include <cstddef>
#include <stdint.h>
#include "Base/WE_BaseUIElement.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIElementRef.hpp"

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
//  static UIElementRef panelChildren[] = { myLabel, myShape };
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
//  static UIElementRef uiElements[] = { panel };
//  UI().setElements(uiElements);
// =============================================================
class UIPanel : public BaseUIElement {
public:
    uint16_t       background        = 0x0000;
    bool           backgroundEnabled = true;
    BaseUIElement* children[Settings.limits.maxPanelChildren] = {};

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
            UIAnchor anchor = UIAnchor::Center)
        : UIPanel(x, y, w, h, background, backgroundEnabled, layer, anchor) {
        setChildren(ch);
    }

    void draw(UIManager& mgr, int16_t offX = 0, int16_t offY = 0) override;

    template <size_t N>
    void setChildren(const UIElementRef (&ch)[N]) {
        static_assert(N <= Settings.limits.maxPanelChildren,
            "UIPanel child array exceeds Settings.limits.maxPanelChildren");

        m_childCount = static_cast<uint8_t>(N);
        for (uint8_t i = 0; i < m_childCount; ++i) children[i] = ch[i].get();
        markDirty();
    }

    void clearChildren();
    void setSize(int16_t width, int16_t height);
    void setBackgroundEnabled(bool enabled);
    void setBackgroundColor(uint16_t color);

private:
    uint8_t m_childCount = 0;
};
