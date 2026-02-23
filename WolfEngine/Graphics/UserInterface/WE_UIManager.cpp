#include "WE_UIManager.hpp"
#include <string.h>

void UIManager::setElements(BaseUIElement** elements) {
    m_elements = elements;
    m_count    = 0;
    while (elements[m_count] != nullptr) m_count++;

    for (uint8_t i = 0; i < m_count; i++) {
        if (m_elements[i]) m_elements[i]->m_manager = this;
    }
    if (m_framebuffer) markAllDirty();
}

void UIManager::initialize(uint16_t* framebuffer, int16_t screenW, int16_t screenH) {
    m_framebuffer = framebuffer;
    m_screenW     = screenW;
    m_screenH     = screenH;
    if (m_elements) markAllDirty();
}

void UIManager::render() {
    if (!m_elements || !m_framebuffer) return;

    for (uint8_t i = 0; i < m_count; i++) {
        BaseUIElement* element = m_elements[i];
        if (!element || !element->isDirty()) continue;

        element->draw(*this);
        element->clearDirty();
    }

    m_dirty = false;
}

void UIManager::markAllDirty() {
    for (uint8_t i = 0; i < m_count; i++) {
        if (m_elements[i]) m_elements[i]->m_dirty = true;
    }
    m_dirty = true;
}

void UIManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= m_screenW || y < 0 || y >= m_screenH) return;
    m_framebuffer[y * m_screenW + x] = color;
}

void UIManager::drawChar(int16_t x, int16_t y, char c, uint16_t color) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t* glyph = FONT_5x7[c - 32];

    for (int16_t col = 0; col < FONT_5x7_WIDTH; col++) {
        uint8_t column = glyph[col];
        for (int16_t row = 0; row < FONT_5x7_HEIGHT; row++) {
            if (column & (1 << row)) {
                drawPixel(x + col, y + row, color);
            }
        }
    }
}