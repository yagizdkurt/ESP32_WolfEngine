#include "WE_BaseUIElement.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

BaseUIElement::BaseUIElement(const UITransform* transform)
    : m_transform(transform), m_manager(nullptr), m_visible(true) {}

void BaseUIElement::show() {
    m_visible = true;
    markDirty();
}

void BaseUIElement::hide() {
    m_visible = false;
    markDirty();
}

int16_t BaseUIElement::getX() const { return resolveLayout(*m_transform).x; }

int16_t BaseUIElement::getY() const { return resolveLayout(*m_transform).y; }

void BaseUIElement::markDirty() { if (m_manager) m_manager->m_dirty = true; }