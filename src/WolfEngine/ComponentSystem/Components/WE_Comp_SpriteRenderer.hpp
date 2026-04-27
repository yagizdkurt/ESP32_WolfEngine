#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_DrawCommand.hpp"
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Utilities/WE_Timer.hpp"
#include "WE_BaseComp.hpp"

// =============================================================
//  WE_Comp_SpriteRenderer.hpp
//  The SpriteRenderer component submits a DrawCommand to the
//  renderer each frame during the component-tick phase. It owns
//  the sprite asset pointer, rotation, and visibility
//  state for the attached GameObject.
//
//  USAGE:
//      // 1. Define pixel data in flash — 2D array [rows][cols]
//      constexpr uint8_t playerPixels[7][7] = { ... };
//
//      // 2. Create Sprite asset — palette is bundled at creation
//      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels, PALETTE_WARM);
//
//      // 3. Attach SpriteRenderer to your GameObject
//      class Player : public GameObject {
//      public:
//          SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, RenderLayer::Player);
//      };
// =============================================================

class SpriteRenderer : public Component {
public:
    /**
     * @brief Construct a sprite renderer component for a GameObject.
     *
     * The component registers itself on the owner and submits sprite draw commands
     * during pre-render tick when sprite rendering is enabled.
     *
     * @param owner Pointer to the owning GameObject (typically @c this).
     * @param sprite Pointer to a Sprite asset (pixels/palette/anchor metadata).
     * @param layer Render layer used for sorting and draw order.
     */
    SpriteRenderer(class GameObject*  owner,
                   const Sprite*      sprite,
                   RenderLayer        layer = RenderLayer::Default);

    /** @brief Destroy the component. */
    ~SpriteRenderer() = default;

    /**
     * @brief Set the active sprite asset.
     * @param sprite New sprite pointer. Used by Animator to switch frames.
     */
    void setSprite(const Sprite* sprite)     { m_sprite   = sprite;    }

    /**
     * @brief Enable a runtime palette override.
     * @param palette Pointer to the override palette.
     */
    void setPaletteOverride(const uint16_t* palette);

    /** @brief Disable palette override and fall back to the sprite palette. */
    void clearPaletteOverride();

    /** @brief Check whether a palette override is active. */
    bool hasPaletteOverride() const { return m_usePaletteOverride; }

    /**
     * @brief Get the currently configured palette override pointer.
     * @return Override palette pointer, or @c nullptr when no override is set.
     */
    const uint16_t* getPaletteOverride() const { return m_paletteOverride; }

    /**
     * @brief Apply a timed palette override using seconds.
     * @param seconds Override duration in seconds.
     * @param palette Pointer to the override palette.
     */
    void setPaletteOverrideForSeconds(float seconds, const uint16_t* palette);

    /**
     * @brief Apply a timed palette override using engine ticks.
     * @param tickCount Duration expressed in engine ticks.
     * @param palette Pointer to the override palette.
     */
    void setPaletteOverrideForTicks(uint32_t tickCount, const uint16_t* palette);

    /**
     * @brief Set sprite rotation in 90-degree steps.
     *
     * Rotation is applied by the renderer at draw time. Pixel data remains unchanged.
     *
     * Supported values:
     * - Rotation::R0
     * - Rotation::R90
     * - Rotation::R180
     * - Rotation::R270
     *
     * @param rotation New rotation value.
     */
    void setRotation(Rotation rotation)      { m_rotation = rotation;  }

    /**
     * @brief Show or hide the sprite without detaching the component.
     * @param visible True to render, false to skip submission.
     */
    void setVisible(bool visible)            { m_visible  = visible;   }

    /**
     * @brief Override default within-layer sorting.
     * @param key Explicit sort key value used instead of automatic draw Y sort.
     */
    void setSortKey(int16_t key) { m_sortKeyOverride = key; m_useSortKeyOverride = true; }

    /** @brief Clear explicit sort key and restore automatic sorting. */
    void clearSortKey()          { m_useSortKeyOverride = false; }

    /** @brief Get the currently active sprite asset pointer. */
    const Sprite* getSprite()   const { return m_sprite;   }

    /** @brief Check whether this renderer is currently visible. */
    bool          isVisible()   const { return m_visible;  }

    /** @brief Get current rotation. */
    Rotation      getRotation() const { return m_rotation; }

    /** @brief Get render layer as raw byte value. */
    uint8_t       getLayer()    const { return m_layer;    }

private:
    void onDraw();
    void preRenderTick() override;

    class GameObject*  m_owner;
    const Sprite*      m_sprite;
    const uint16_t*    m_paletteOverride = nullptr;
    uint8_t            m_layer;
    Rotation           m_rotation = Rotation::R0;
    Timer              m_paletteOverrideTimer;
    int64_t            m_paletteOverrideDurationMs = 0;
    bool               m_usePaletteOverride = false;
    bool               m_useTimedPaletteOverride = false;
    bool               m_useSortKeyOverride = false;
    int16_t            m_sortKeyOverride = 0;
    bool               m_visible  = true;

    friend class Animator;
    friend class GameObject; // for direct access to onDraw() during the render phase
};
