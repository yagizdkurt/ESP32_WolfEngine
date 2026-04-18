#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/WolfEngine.hpp"

SpriteRenderer::SpriteRenderer(GameObject* owner, const Sprite* sprite, const uint16_t* palette, RenderLayer layer)
    : m_owner   (owner)
    , m_sprite  (sprite)
    , m_palette (palette)
    , m_layer   (static_cast<int16_t>(layer))
{
    type        = COMP_SPRITE;
    tickEnabled = true;  // base class defaults to false — must be set explicitly
}

void SpriteRenderer::tick() { if constexpr (RENDER_SETTINGS.spriteSystemEnabled) onDraw(); }

void SpriteRenderer::onDraw() {
    if (!m_visible || !m_sprite) return;

    // World → screen, centered on the sprite's world position
    Vec2 screen = MainCamera().worldToScreen(m_owner->transform.position);
    int16_t drawX = static_cast<int16_t>(screen.x) - static_cast<int16_t>(m_sprite->size / 2);
    int16_t drawY = static_cast<int16_t>(screen.y) - static_cast<int16_t>(m_sprite->size / 2);

    // Coarse cull — skip if entirely outside the game region
    const int sz = m_sprite->size;
    if (drawX + sz <= RENDER_SETTINGS.gameRegion.x1 ||
        drawX       >= RENDER_SETTINGS.gameRegion.x2 ||
        drawY + sz <= RENDER_SETTINGS.gameRegion.y1 ||
        drawY       >= RENDER_SETTINGS.gameRegion.y2)
        return;

    DrawCommand cmd;
    cmd.type             = DrawCommandType::Sprite;
    cmd.layer            = static_cast<RenderLayer>(m_layer);
    cmd.x                = drawX;
    cmd.y                = drawY;
    cmd.sortKey          = m_useSortKeyOverride ? m_sortKeyOverride : drawY;
    cmd.sprite.pixels    = m_sprite->pixels;
    cmd.sprite.palette   = m_palette;
    cmd.sprite.size      = sz;
    cmd.sprite.rotation  = m_rotation;

    RenderSys().submitDrawCommand(cmd);
}
