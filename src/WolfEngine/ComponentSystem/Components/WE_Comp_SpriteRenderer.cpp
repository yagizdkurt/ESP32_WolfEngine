#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/WolfEngine.hpp"

SpriteRenderer::SpriteRenderer(GameObject* owner, const Sprite* sprite, RenderLayer layer)
    : m_owner   (owner)
    , m_sprite  (sprite)
    , m_layer   (static_cast<uint8_t>(layer))
{
    type        = COMP_SPRITE;
    tickEnabled = true;  // base class defaults to false — must be set explicitly
}

void SpriteRenderer::tick() { if constexpr (Settings.render.spriteSystemEnabled) onDraw(); }

void SpriteRenderer::onDraw() {
    if (!m_visible || !m_sprite) return;

    const int W  = m_sprite->width;
    const int H  = m_sprite->height;
    const int AX = m_sprite->anchorX;
    const int AY = m_sprite->anchorY;

    // World → screen position of the anchor point
    Vec2 screen = MainCamera().worldToScreen(m_owner->transform.position);
    const int16_t screenX = static_cast<int16_t>(screen.x);
    const int16_t screenY = static_cast<int16_t>(screen.y);

    // Compute top-left corner of the output rect from the anchor, per rotation
    int16_t topX, topY;
    switch (m_rotation) {
        case Rotation::R0:
            topX = screenX - AX;
            topY = screenY - AY;
            break;
        case Rotation::R90:
            topX = screenX - AY;
            topY = screenY - (W - 1 - AX);
            break;
        case Rotation::R180:
            topX = screenX - (W - 1 - AX);
            topY = screenY - (H - 1 - AY);
            break;
        default: // R270
            topX = screenX - (H - 1 - AY);
            topY = screenY - AX;
            break;
    }

    // Output dimensions swap at 90°/270°
    const int outW = (m_rotation == Rotation::R90 || m_rotation == Rotation::R270) ? H : W;
    const int outH = (m_rotation == Rotation::R90 || m_rotation == Rotation::R270) ? W : H;

    // Coarse cull — skip if entirely outside the game region
    if (topX + outW <= Settings.render.gameRegion.x1 ||
        topX        >= Settings.render.gameRegion.x2 ||
        topY + outH <= Settings.render.gameRegion.y1 ||
        topY        >= Settings.render.gameRegion.y2)
        return;

    DrawCommand cmd;
    cmd.type          = DrawCommandType::Sprite;
    cmd.x             = topX;
    cmd.y             = topY;
    cmd.flags         = cmdSetRotation(0, m_rotation);
    uint8_t sortByte  = m_useSortKeyOverride
                          ? WE_Math::clampToByte(m_sortKeyOverride)
                          : WE_Math::clampToByte(topY);
    cmd.sortKey       = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), sortByte);
    cmd.sprite.pixels  = m_sprite->pixels;
    cmd.sprite.palette = m_sprite->palette;
    cmd.sprite.width   = static_cast<uint8_t>(W);
    cmd.sprite.height  = static_cast<uint8_t>(H);

    RenderSys().submitDrawCommand(cmd);
}
