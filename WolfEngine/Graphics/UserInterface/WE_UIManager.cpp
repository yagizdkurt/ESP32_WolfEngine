#include "WE_UIManager.hpp"
#include <string.h>

void UIManager::setElements(BaseUIElement** elements) {
    m_elements = elements;
    m_count    = 0;
    while (elements[m_count] != nullptr) m_count++;

    for (uint8_t i = 0; i < m_count; i++) {
        if (m_elements[i]) {
            m_elements[i]->m_manager = this;
            if (m_framebuffer) m_elements[i]->fb = m_framebuffer;
        }
    }
    if (m_framebuffer) m_dirty = true;
}

void UIManager::initialize(uint16_t* framebuffer) {
    m_framebuffer = framebuffer;
    if (m_elements) {
        for (uint8_t i = 0; i < m_count; i++) if (m_elements[i]) m_elements[i]->fb = framebuffer;
        m_dirty = true;
    }
}

void UIManager::render() {
    if (!m_elements || !m_framebuffer) return;
    for (uint8_t i = 0; i < m_count; i++) { if (m_elements[i]) m_elements[i]->draw(*this); }
    m_dirty = false;
}

