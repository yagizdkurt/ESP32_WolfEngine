#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palette_Grayscale.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WE_UITransform.hpp"
class UIManager;


// =============================================================
// THIS NEEDS TO BE NULL TERMINATED!!! PLEASE DONT FORGET THIS
// =============================================================
//  WE_BaseUIElement
//  Abstract base class for all UI elements.
//  UITransform is stored in flash (const) - position never changes.
//  visible, dirty, color index, and palette pointer live in RAM.
//
//  Never instantiate directly - use UILabel, UIBar, etc.
// =============================================================
class BaseUIElement {
public:
    // Bind this element to a fixed transform.
    BaseUIElement(const UITransform* transform);

    virtual ~BaseUIElement() = default;
    // Render this element when dirty and visible.
    virtual void draw(class UIManager& mgr) = 0;  // pure virtual - must implement

    // Make the element visible and mark UI dirty.
    void show();
    // Hide the element and mark UI dirty.
    void hide();

    // Return current visibility state.
    bool isVisible() const { return m_visible; }

    // Get absolute X coordinate.
    int16_t getX() const;
    // Get absolute Y coordinate (applies anchor offset when enabled).
    int16_t getY() const;

protected:
    friend class UIManager;
    friend class UIPanel;

    uint16_t* fb = nullptr;
    const UITransform* m_transform;
    UIManager*         m_manager;
    bool               m_visible;
    
    static constexpr int SCREEN_WIDTH = Renderer::SCREEN_WIDTH;
    static constexpr int SCREEN_HEIGHT = Renderer::SCREEN_HEIGHT;

    inline void drawPixelRaw(int16_t x, int16_t y, uint16_t color) const {
        if (!fb || x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
        fb[y * SCREEN_WIDTH + x] = color;
    }

    void markDirty();
};
