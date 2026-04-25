#pragma once
#include <cstddef>
#include <stdint.h>
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIElementRef.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"

// =============================================================
//  WE_UIManager
//  UI manager owned by the WolfEngine class.
//  Holds pointers to all UI elements declared by the user.
//  Renders only elements that have their dirty flag set.
//
//  USAGE:
//
//  // Elements
//  static UILabel scoreLabel(4, 4, 96, 7, "Score: 0");
//  static UILabel hpLabel(4, 16, 96, 7, "HP: 100");
//
//  // Non-null list. nullptr entries are compile-time errors.
//  static UIElementRef uiElements[] = { scoreLabel, hpLabel };
//
//  // Before StartGame():
//  engine.ui.setElements(uiElements);
// =============================================================

class Renderer;
class UIManager {
public:
    template <size_t N>
    void setElements(const UIElementRef (&elements)[N]) {
        static_assert(N <= Settings.limits.maxUIElements,
            "UI element array exceeds Settings.limits.maxUIElements");
        setElementsInternal(elements, static_cast<uint8_t>(N));
    }

    void clearElements();

    uint16_t* getFramebuffer() const { return m_framebuffer; }
    static constexpr int SCREEN_WIDTH = Renderer::SCREEN_WIDTH;
    static constexpr int SCREEN_HEIGHT = Renderer::SCREEN_HEIGHT;
private:
    void setElementsInternal(const UIElementRef* elements, uint8_t count);

    BaseUIElement*  m_elements[Settings.limits.maxUIElements] = {};
    uint8_t         m_count       = 0;
    uint16_t*       m_framebuffer = nullptr;
    bool            m_dirty       = false;

    void initialize(uint16_t* framebuffer);
    void render();
    bool isDirty() const { return m_dirty; }

    friend class WolfEngine;
    friend class Renderer;
    friend class BaseUIElement;
};
