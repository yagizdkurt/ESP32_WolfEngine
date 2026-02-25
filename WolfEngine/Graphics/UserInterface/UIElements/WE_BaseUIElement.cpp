#include "WE_BaseUIElement.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

BaseUIElement::BaseUIElement(const UITransform* transform)
    : m_transform(transform)
    , m_manager(nullptr)
    , m_visible(true)
    , m_dirty(false) {}

void BaseUIElement::show() {
    m_visible = true;
    markDirty();
}

void BaseUIElement::hide() {
    m_visible = false;
    markDirty();
}

bool BaseUIElement::isVisible() const {
    return m_visible;
}

bool BaseUIElement::isDirty() const {
    return m_dirty;
}

void BaseUIElement::clearDirty() {
    m_dirty = false;
}

int16_t BaseUIElement::getX() const {
    return m_transform->x;
}

int16_t BaseUIElement::getY() const {
    return m_transform->anchor
        ? m_transform->y + RENDER_UI_START_ROW
        : m_transform->y;
}

void BaseUIElement::markDirty() {
    m_dirty = true;
    if (m_manager) m_manager->m_dirty = true;
}
