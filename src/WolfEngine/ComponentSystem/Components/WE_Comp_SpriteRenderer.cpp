#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/Utilities/WE_MathUtilities.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/Utilities/WE_Debug.hpp"
#include "WolfEngine/WolfEngine.hpp"

SpriteRenderer::SpriteRenderer(GameObject* owner, const Sprite* sprite, RenderLayer layer)
    : m_owner   (owner)
    , m_sprite  (sprite)
    , m_layer   (static_cast<uint8_t>(layer))
{
    type        = COMP_SPRITE;
    preRenderTickEnabled = true;  // base class defaults to false — must be set explicitly
    owner->registerComponent(this);
}

void SpriteRenderer::setPaletteOverride(const uint16_t* palette) {
    m_paletteOverride = palette;
    m_usePaletteOverride = (palette != nullptr);
    m_useTimedPaletteOverride = false;
    m_paletteOverrideDurationMs = 0;
    m_paletteOverrideTimer.stop();
}

void SpriteRenderer::clearPaletteOverride() {
    m_paletteOverride = nullptr;
    m_usePaletteOverride = false;
    m_useTimedPaletteOverride = false;
    m_paletteOverrideDurationMs = 0;
    m_paletteOverrideTimer.stop();
}

void SpriteRenderer::setPaletteOverrideForSeconds(float seconds, const uint16_t* palette) {
    if (palette == nullptr || seconds <= 0.0f) {
        clearPaletteOverride();
        return;
    }
    setPaletteOverride(palette);
    m_useTimedPaletteOverride = true;
    m_paletteOverrideDurationMs = static_cast<int64_t>(seconds * 1000.0f + 0.5f);
    m_paletteOverrideTimer.start();
}

void SpriteRenderer::setPaletteOverrideForTicks(uint32_t tickCount, const uint16_t* palette) {
    setPaletteOverrideForSeconds(static_cast<float>(tickCount) * WETime::DELTA_TIME, palette);
}

void SpriteRenderer::preRenderTick() {
    if (m_useTimedPaletteOverride &&
        m_paletteOverrideTimer.elapsed(m_paletteOverrideDurationMs)) {
        clearPaletteOverride();
    }
    if constexpr (Settings.render.spriteSystemEnabled) onDraw();
}

void SpriteRenderer::onDraw() {
    if (!m_visible || !m_sprite) return;

    const uint8_t* pixels = m_sprite->pixels;
    const uint16_t* selectedPalette = m_usePaletteOverride ? m_paletteOverride : m_sprite->palette;

    if (pixels == nullptr || selectedPalette == nullptr) { // Validitiy check: sprite data must not be null
        WE_LOGE("SpriteRenderer", "Invalid sprite pointers (sprite=%p pixels=%p palette=%p override=%d)",
                static_cast<const void*>(m_sprite), static_cast<const void*>(pixels),
                static_cast<const void*>(selectedPalette), m_usePaletteOverride ? 1 : 0);
        WE_ASSERT(false, "SpriteRenderer: pixels/palette pointer must not be null");
        return;
    }

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
    cmd.sprite.pixels  = pixels;
    cmd.sprite.palette = selectedPalette;
    cmd.sprite.width   = static_cast<uint8_t>(W);
    cmd.sprite.height  = static_cast<uint8_t>(H);

    RenderSys().submitDrawCommand(cmd);
}
