#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

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

class UIManager final {
public:
    void setElements(BaseUIElement** elements); // THIS MUST BE NULL TERMINATED!!! PLEASE DONT FORGET THIS
    void markAllDirty();
    bool isDirty() const { return m_dirty; }
private:
    BaseUIElement** m_elements    = nullptr;
    uint8_t         m_count       = 0;
    uint16_t*       m_framebuffer = nullptr;
    int16_t         m_screenW     = 0;
    int16_t         m_screenH     = 0;
    bool            m_dirty       = false;

    void initialize(uint16_t* framebuffer, int16_t screenW, int16_t screenH);
    void render();

    void drawChar(int16_t x, int16_t y, char c, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    friend class WolfEngine;
    friend class Renderer;
    friend class BaseUIElement;
    friend class UILabel;
};