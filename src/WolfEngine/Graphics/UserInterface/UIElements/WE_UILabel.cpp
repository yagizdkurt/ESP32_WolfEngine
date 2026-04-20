#include "WE_UILabel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/WolfEngine.hpp"

#include <string.h>

UILabel::UILabel(int16_t x, int16_t y, int16_t w, int16_t h,
                 const char* text,
                 uint8_t colorIndex,
                 const uint16_t* palette,
                 uint8_t layer,
                 UIAnchor anchor)
{
    this->x          = x;
    this->y          = y;
    this->w          = w;
    this->h          = h;
    this->layer      = layer;
    this->anchor     = anchor;
    this->colorIndex = colorIndex;
    this->palette    = palette;
    strncpy(this->text, text ? text : "", WE_UI_LABEL_MAX_LEN - 1);
    this->text[WE_UI_LABEL_MAX_LEN - 1] = '\0';
}

void UILabel::draw(UIManager& mgr, int16_t offX, int16_t offY) {
    if (!m_visible) return;

    UIRect rect = resolveRect();
    rect.x += offX;
    rect.y += offY;
    const uint16_t color = palette[colorIndex];

    DrawCommand cmd;
    cmd.type             = DrawCommandType::TextRun;
    cmd.flags            = 0;
    cmd.x                = rect.x;
    cmd.y                = rect.y;
    cmd.sortKey          = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
    cmd.textRun.text     = text;
    cmd.textRun.color    = color;
    cmd.textRun.maxWidth = static_cast<uint8_t>(rect.width > 0 ? rect.width : 0);
    RenderSys().submitDrawCommand(cmd);
}

void UILabel::setText(const char* t) {
    strncpy(text, t, WE_UI_LABEL_MAX_LEN - 1);
    text[WE_UI_LABEL_MAX_LEN - 1] = '\0';
    markDirty();
}

void UILabel::setColorIndex(uint8_t index) {
    colorIndex = index;
    markDirty();
}

const char* UILabel::getText()       const { return text; }
uint8_t     UILabel::getColorIndex() const { return colorIndex; }
