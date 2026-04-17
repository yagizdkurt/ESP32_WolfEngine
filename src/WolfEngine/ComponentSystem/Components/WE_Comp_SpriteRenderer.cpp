#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/WolfEngine.hpp"

SpriteRenderer::SpriteRenderer(GameObject* owner, const Sprite* sprite, const uint16_t*  palette, RenderLayer layer)
    : m_owner   (owner)
    , m_sprite  (sprite)
    , m_palette (palette)
    , m_layer   (static_cast<int16_t>(layer))
{
    type = COMP_SPRITE;
    RenderSys().registerSprite(this, m_layer);
}

SpriteRenderer::~SpriteRenderer() { RenderSys().unregisterSprite(this, m_layer); }

SpriteData SpriteRenderer::getRenderData() const {
    SpriteData data;
    data.pixels   = m_sprite->pixels;
    data.palette  = m_palette;
    data.x        = m_owner->transform.position.x;
    data.y        = m_owner->transform.position.y;
    data.size     = m_sprite->size;
    data.rotation = m_rotation;
    data.visible  = m_visible;
    return data;
}
