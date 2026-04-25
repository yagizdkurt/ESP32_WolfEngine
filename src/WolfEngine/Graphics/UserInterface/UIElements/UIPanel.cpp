#include "UIPanel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/WolfEngine.hpp"

UIPanel::UIPanel(int16_t x, int16_t y, int16_t w, int16_t h,
                 uint16_t background,
                 bool backgroundEnabled,
                 uint8_t layer,
                 UIAnchor anchor)
{
    this->x                 = x;
    this->y                 = y;
    this->w                 = w;
    this->h                 = h;
    this->layer             = layer;
    this->anchor            = anchor;
    this->background        = background;
    this->backgroundEnabled = backgroundEnabled;
}

void UIPanel::clearChildren() {
    for (uint8_t i = 0; i < m_childCount; ++i) children[i] = nullptr;
    m_childCount = 0;
    markDirty();
}

void UIPanel::setSize(int16_t width, int16_t height) { w = width; h = height; markDirty(); }
void UIPanel::setBackgroundEnabled(bool enabled)      { backgroundEnabled = enabled; markDirty(); }
void UIPanel::setBackgroundColor(uint16_t color)      { background = color; markDirty(); }

void UIPanel::draw(UIManager& mgr, int16_t offX, int16_t offY) {
    if (!m_visible) return;

    UIRect rect = resolveRect();
    rect.x += offX;
    rect.y += offY;

    if (backgroundEnabled && w > 0 && h > 0) {
        DrawCommand bg;
        bg.type           = DrawCommandType::FillRect;
        bg.flags          = 0;
        bg.x              = rect.x;
        bg.y              = rect.y;
        bg.sortKey        = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
        bg.fillRect.w     = static_cast<uint8_t>(w);
        bg.fillRect.h     = static_cast<uint8_t>(h);
        bg.fillRect.color = background;
        RenderSys().submitDrawCommand(bg);
    }

    for (uint8_t i = 0; i < m_childCount; ++i) {
        BaseUIElement* child = children[i];
        if (child == this) continue;
        child->m_manager   = m_manager;
        uint8_t savedLayer = child->m_layer;
        child->m_layer     = m_layer + 1;
        child->draw(mgr, rect.x, rect.y);
        child->m_layer     = savedLayer;
    }
}
