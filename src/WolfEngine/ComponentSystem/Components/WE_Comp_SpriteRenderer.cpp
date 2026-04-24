#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/WolfEngine.hpp"

SpriteRenderer::SpriteRenderer(GameObject* owner, const Sprite* sprite, const uint16_t* palette, RenderLayer layer)
    : m_owner   (owner)
    , m_sprite  (sprite)
    , m_palette (palette)
    , m_layer   (static_cast<uint8_t>(layer))
{
    type        = COMP_SPRITE;
    tickEnabled = true;  // base class defaults to false — must be set explicitly
    owner->registerComponent(this);
}

void SpriteRenderer::preRenderTick() { if constexpr (Settings.render.spriteSystemEnabled) onDraw(); }

void SpriteRenderer::onDraw() {
    if (!m_visible || !m_sprite) return;

    // World → screen, centered on the sprite's world position
    Vec2 screen = MainCamera().worldToScreen(m_owner->transform.position);
    int16_t drawX = static_cast<int16_t>(screen.x) - static_cast<int16_t>(m_sprite->size / 2);
    int16_t drawY = static_cast<int16_t>(screen.y) - static_cast<int16_t>(m_sprite->size / 2);

    // Coarse cull — skip if entirely outside the game region
    const int sz = m_sprite->size;
    if (drawX + sz <= Settings.render.gameRegion.x1 ||
        drawX       >= Settings.render.gameRegion.x2 ||
        drawY + sz <= Settings.render.gameRegion.y1 ||
        drawY       >= Settings.render.gameRegion.y2)
        return;

    DrawCommand cmd;
    cmd.type             = DrawCommandType::Sprite;
    cmd.x                = drawX;
    cmd.y                = drawY;
    cmd.flags            = cmdSetRotation(0, m_rotation);
    uint8_t sortByte     = m_useSortKeyOverride
                             ? static_cast<uint8_t>(m_sortKeyOverride)
                             : static_cast<uint8_t>(drawY);
    cmd.sortKey          = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), sortByte);
    cmd.sprite.pixels    = m_sprite->pixels;
    cmd.sprite.palette   = m_palette;
    cmd.sprite.size      = static_cast<uint8_t>(sz);

    RenderSys().submitDrawCommand(cmd);
}
