#include "WE_UIManager.hpp"

void UIManager::setElementsInternal(const UIElementRef* elements, uint8_t count) {
    for (uint8_t i = 0; i < m_count; ++i) m_elements[i] = nullptr;

    m_count = count;
    for (uint8_t i = 0; i < m_count; ++i) {
        BaseUIElement* element = elements[i].get();
        m_elements[i]          = element;
        element->m_manager     = this;
        element->m_drawOrder   = i;
        element->m_layer       = element->layer;
    }

    if (m_framebuffer) m_dirty = true;
}

void UIManager::clearElements() {
    for (uint8_t i = 0; i < m_count; ++i) m_elements[i] = nullptr;
    m_count = 0;
    if (m_framebuffer) m_dirty = true;
}

void UIManager::initialize(uint16_t* framebuffer) {
    m_framebuffer = framebuffer;
    clearElements();
}

void UIManager::render() {
    if (!m_framebuffer || m_count == 0) return;
    for (uint8_t i = 0; i < m_count; ++i) m_elements[i]->draw(*this, 0, 0);
    m_dirty = false;
}
