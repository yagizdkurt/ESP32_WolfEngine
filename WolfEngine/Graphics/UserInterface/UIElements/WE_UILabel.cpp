#include "WE_UILabel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

#include <string.h>

UILabel::UILabel(const UITransform* transform, UILabelState* state)
    : BaseUIElement(transform)
    , m_state(state) {}

void UILabel::draw(UIManager& mgr) {
    if (!m_visible) return;

    const char*    text  = m_state->text;
    const int16_t  x     = getX();
    const int16_t  y     = getY();
    const uint16_t color = m_state->palette[m_state->colorIndex];
    const int16_t  step  = FONT_5x7_WIDTH + FONT_5x7_SPACING;

    for (int i = 0; text[i] != '\0' && i < WE_UI_LABEL_MAX_LEN; i++) {
        drawChar(x + i * step, y, text[i], color);
    }
}

void UILabel::drawChar(int16_t x, int16_t y, char c, uint16_t color) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t* glyph = FONT_5x7[c - 32];

    for (int16_t col = 0; col < FONT_5x7_WIDTH; col++) {
        uint8_t column = glyph[col];
        for (int16_t row = 0; row < FONT_5x7_HEIGHT; row++) {
            if (column & (1 << row)) {
                drawPixelRaw(x + col, y + row, color);
            }
        }
    }
}

void UILabel::setText(const char* text) {
    strncpy(m_state->text, text, WE_UI_LABEL_MAX_LEN - 1);
    m_state->text[WE_UI_LABEL_MAX_LEN - 1] = '\0';
    markDirty();
}

void UILabel::setColorIndex(uint8_t index) {
    m_state->colorIndex = index;
    markDirty();
}

const char* UILabel::getText() const {
    return m_state->text;
}

uint8_t UILabel::getColorIndex() const {
    return m_state->colorIndex;
}
