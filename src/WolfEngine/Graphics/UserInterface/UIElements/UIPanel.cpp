#include "UIPanel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

UIPanel::UIPanel(const UITransform* transform, UIPanelState* state, BaseUIElement** children)
    : BaseUIElement(transform), m_state(state), m_children(children) {}

void UIPanel::setSize(int16_t width, int16_t height) {
    m_state->width  = width;
    m_state->height = height;
    markDirty();
    propagateDirtyToChildren();
}

void UIPanel::setBackgroundEnabled(bool enabled) {
    m_state->backgroundEnabled = enabled;
    markDirty();
    propagateDirtyToChildren();
}

void UIPanel::setBackgroundColor(uint16_t color) {
    m_state->backgroundColor = color;
    markDirty();
    propagateDirtyToChildren();
}

BaseUIElement** UIPanel::getChildren() const { return m_children; }

void UIPanel::drawBackground(UIManager& mgr, int16_t panelX, int16_t panelY) {
    if (!m_state->backgroundEnabled) return;
    if (m_state->width <= 0 || m_state->height <= 0) return;

    for (int16_t dy = 0; dy < m_state->height; ++dy) {
        for (int16_t dx = 0; dx < m_state->width; ++dx) {
            drawPixelRaw(panelX + dx, panelY + dy, m_state->backgroundColor);
        }
    }
}

void UIPanel::syncChildManagers() {
    if (!m_children) return;

    for (int16_t i = 0; m_children[i] != nullptr; ++i) {
        BaseUIElement* child = m_children[i];
        if (!child) continue;

        child->m_manager = m_manager;
    }
}

void UIPanel::propagateDirtyToChildren() {
    // There is only manager-level dirty tracking in this UI system.
    // Sync manager linkage so child updates can mark the UI dirty.
    syncChildManagers();
}

void UIPanel::draw(UIManager& mgr) {
    if (!m_visible) return;

    const int16_t panelX = getX();
    const int16_t panelY = getY();

    syncChildManagers();
    drawBackground(mgr, panelX, panelY);

    if (!m_children) return;

    for (int16_t i = 0; m_children[i] != nullptr; ++i) {
        BaseUIElement* child = m_children[i];
        if (!child || child == this) continue;

        const UITransform* original = child->m_transform;
        if (!original) continue;

        UITransform adjusted = *original;
        adjusted.x = static_cast<int16_t>(adjusted.x + panelX);
        adjusted.y = static_cast<int16_t>(adjusted.y + panelY);

        child->m_transform = &adjusted;
        child->draw(mgr);
        child->m_transform = original;
    }
}
