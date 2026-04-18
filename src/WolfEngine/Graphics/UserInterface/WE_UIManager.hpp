#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"

// =============================================================
//  WE_UIManager
//  UI manager owned by the WolfEngine class.
//  Holds pointers to all UI elements declared by the user.
//  Renders only elements that have their dirty flag set.
//
//  USAGE:
//
//  // Flash
//  static const UITransform scoreTf = { 52, 12, true };
//
//  // RAM
//  static UILabelState scoreState = { "Score: 0" };
//
//  // Element
//  static UILabel scoreLabel(&scoreTf, &scoreState);
//
//  // Array MUST be null-terminated — always end with nullptr
//  static BaseUIElement* uiElements[] = { &scoreLabel, nullptr };
//
//  // Before StartGame():
//  engine.ui.setElements(uiElements);
// =============================================================

class Renderer;
class UIManager {
public:
    void setElements(BaseUIElement** elements); // THIS MUST BE NULL TERMINATED!!! PLEASE DONT FORGET THIS
    uint16_t* getFramebuffer() const { return m_framebuffer; }
    static constexpr int SCREEN_WIDTH = Renderer::SCREEN_WIDTH;
    static constexpr int SCREEN_HEIGHT = Renderer::SCREEN_HEIGHT;
private:
    BaseUIElement** m_elements    = nullptr;
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