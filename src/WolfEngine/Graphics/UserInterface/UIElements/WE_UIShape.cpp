#include "WE_UIShape.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/WolfEngine.hpp"

UIShape::UIShape(int16_t x, int16_t y, int16_t w, int16_t h,
                 UIShapeType shape,
                 bool filled,
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
    this->shape      = shape;
    this->filled     = filled;
    this->colorIndex = colorIndex;
    this->palette    = palette;
}

void UIShape::setSize(int16_t width, int16_t height) { w = width; h = height; markDirty(); }
void UIShape::setColorIndex(uint8_t index)           { colorIndex = index; markDirty(); }
void UIShape::setShape(UIShapeType s)                { shape = s; markDirty(); }
void UIShape::setFilled(bool f)                      { filled = f; markDirty(); }
void UIShape::setPalette(const uint16_t* p)          { palette = p; markDirty(); }

void UIShape::setLength(int16_t length) {
    switch (shape) {
    case UIShapeType::HLine: w = length; break;
    case UIShapeType::VLine: h = length; break;
    default: w = length; h = length; break;
    }
    markDirty();
}

int16_t     UIShape::getWidth()      const { return w; }
int16_t     UIShape::getHeight()     const { return h; }
uint8_t     UIShape::getColorIndex() const { return colorIndex; }
bool        UIShape::isFilled()      const { return filled; }
UIShapeType UIShape::getShape()      const { return shape; }

void UIShape::draw(UIManager& mgr, int16_t offX, int16_t offY) {
    if (!m_visible) return;

    UIRect rect = resolveRect();
    rect.x += offX;
    rect.y += offY;
    const int16_t  shapeW = w;
    const int16_t  shapeH = h;
    const uint16_t color  = ResolvePaletteColor(palette, colorIndex);

    // Shape-aware early-out: HLine only needs a valid width,
    // VLine only needs a valid height.
    switch (shape) {
    case UIShapeType::HLine:  if (shapeW <= 0) return; break;
    case UIShapeType::VLine:  if (shapeH <= 0) return; break;
    default:                  if (shapeW <= 0 || shapeH <= 0) return; break;
    }

    DrawCommand cmd;

    switch (shape) {
    case UIShapeType::HLine:
        cmd.type       = DrawCommandType::Line;
        cmd.flags      = 0;
        cmd.x          = rect.x;
        cmd.y          = rect.y;
        cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
        cmd.line.x2    = static_cast<int16_t>(rect.x + shapeW - 1);
        cmd.line.y2    = rect.y;
        cmd.line.color = color;
        RenderSys().submitDrawCommand(cmd);
        break;

    case UIShapeType::VLine:
        cmd.type       = DrawCommandType::Line;
        cmd.flags      = 0;
        cmd.x          = rect.x;
        cmd.y          = rect.y;
        cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
        cmd.line.x2    = rect.x;
        cmd.line.y2    = static_cast<int16_t>(rect.y + shapeH - 1);
        cmd.line.color = color;
        RenderSys().submitDrawCommand(cmd);
        break;

    case UIShapeType::Rectangle:
    default:
        if (filled) {
            cmd.type           = DrawCommandType::FillRect;
            cmd.flags          = 0;
            cmd.x              = rect.x;
            cmd.y              = rect.y;
            cmd.sortKey        = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.fillRect.w     = static_cast<uint8_t>(shapeW);
            cmd.fillRect.h     = static_cast<uint8_t>(shapeH);
            cmd.fillRect.color = color;
            RenderSys().submitDrawCommand(cmd);
        } else {
            // Top edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = rect.x;
            cmd.y          = rect.y;
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = static_cast<int16_t>(rect.x + shapeW - 1);
            cmd.line.y2    = rect.y;
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
            // Bottom edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = rect.x;
            cmd.y          = static_cast<int16_t>(rect.y + shapeH - 1);
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = static_cast<int16_t>(rect.x + shapeW - 1);
            cmd.line.y2    = static_cast<int16_t>(rect.y + shapeH - 1);
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
            // Left edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = rect.x;
            cmd.y          = rect.y;
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = rect.x;
            cmd.line.y2    = static_cast<int16_t>(rect.y + shapeH - 1);
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
            // Right edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = static_cast<int16_t>(rect.x + shapeW - 1);
            cmd.y          = rect.y;
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = static_cast<int16_t>(rect.x + shapeW - 1);
            cmd.line.y2    = static_cast<int16_t>(rect.y + shapeH - 1);
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
        }
        break;
    }
}
