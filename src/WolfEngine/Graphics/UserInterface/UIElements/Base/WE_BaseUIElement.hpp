#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palette_Grayscale.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WE_UITransform.hpp"
class UIManager;


// =============================================================
//  WE_BaseUIElement
//  Abstract base class for all UI elements.
//  Layout fields (x, y, w, h, layer, anchor) live directly on
//  the element; set them via constructors or direct field assignment.
//
//  Never instantiate directly - use UILabel, UIShape, UIPanel.
// =============================================================
class BaseUIElement {
public:
    // Public layout fields — set at definition time.
    int16_t  x      = 0;
    int16_t  y      = 0;
    int16_t  w      = 0;
    int16_t  h      = 0;
    uint8_t  layer  = 0;
    UIAnchor anchor = UIAnchor::Center;

    virtual ~BaseUIElement() = default;
    // Render this element. offX/offY are additive pixel offsets from a parent panel.
    virtual void draw(class UIManager& mgr, int16_t offX = 0, int16_t offY = 0) = 0;

    // Make the element visible and mark UI dirty.
    void show();
    // Hide the element and mark UI dirty.
    void hide();

    // Return current visibility state.
    bool isVisible() const { return m_visible; }

protected:
    friend class UIManager;
    friend class UIPanel;

    UIManager* m_manager;
    bool       m_visible  = true;
    uint8_t    m_drawOrder = 0;
    uint8_t    m_layer     = 0;

    static constexpr int SCREEN_WIDTH  = Renderer::SCREEN_WIDTH;
    static constexpr int SCREEN_HEIGHT = Renderer::SCREEN_HEIGHT;

    void markDirty();

    UIRect resolveRect() const {
        return resolveLayout(UITransform(x, y, w, h, layer, anchor));
    }
};
