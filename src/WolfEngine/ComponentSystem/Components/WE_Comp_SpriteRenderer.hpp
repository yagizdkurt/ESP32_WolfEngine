#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp"
#include "WolfEngine/Graphics/SpriteSystem/WE_SpriteData.hpp"
#include "WolfEngine/Settings/WE_Layers.hpp"
#include "WE_BaseComp.hpp"
class GameObject;
class Renderer;

// =============================================================
//  WE_Comp_SpriteRenderer.hpp
//  The SpriteRenderer is the component the renderer talks to.
//  It holds a pointer to a Sprite asset, owns palette, rotation,
//  and visibility state, and registers itself with the renderer
//  automatically on construction.
//
//  USAGE:
//      // 1. Define pixel data in flash
//      constexpr uint8_t playerPixels[7 * 7] = { ... };
//
//      // 2. Create Sprite asset — size is automatically deduced and validated
//      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);
//
//      // 3. Attach SpriteRenderer to your GameObject
//      class Player : public GameObject {
//      public:
//          SpriteRenderer spriteRenderer = SpriteRenderer(this, &SPRITE_PLAYER, PALETTE_WARM, RenderLayer::Player);
//      };
// =============================================================


class SpriteRenderer : public Component {
public:

    // Construct and auto-register with the renderer.
    //
    // owner   — pointer to the owning GameObject (pass 'this')
    // sprite  — pointer to a constexpr Sprite asset in flash
    // palette — constexpr uint16_t[32] palette array in flash
    // layer   — render layer, defaults to 0
    SpriteRenderer(class GameObject*  owner,
                   const Sprite*      sprite,
                   const uint16_t*    palette,
                   RenderLayer        layer = RenderLayer::Default);

    // Destructor automatically unregisters from the renderer.
    ~SpriteRenderer();

    // Returns all data the renderer needs to draw this sprite.
    SpriteData getRenderData() const;

    // ---------------------------------------------------------
    //  Runtime controls
    // ---------------------------------------------------------

    // Swap the active Sprite asset — used by Animator for frame changes.
    void setSprite(const Sprite* sprite)     { m_sprite   = sprite;    }

    // Swap the active palette.
    void setPalette(const uint16_t* palette) { m_palette  = palette;   }

    // -------------------------------------------------------------
    //  Change rotation
    //
    //  Rotation:
    //  Represents the four supported 90-degree snap rotations.
    //  Applied by the renderer during drawing — sprite pixel data
    //  itself never changes, the renderer reads it in a different
    //  order depending on this value.
    //
    //  R0   — Normal, no rotation
    //  R90  — 90 degrees clockwise
    //  R180 — 180 degrees (upside down)
    //  R270 — 270 degrees clockwise (or 90 counter-clockwise)
    // -------------------------------------------------------------
    void setRotation(Rotation rotation)      { m_rotation = rotation;  }

    // Show or hide without removing from the render layer.
    void setVisible(bool visible)            { m_visible  = visible;   }


    const Sprite* getSprite()   const { return m_sprite;   }
    bool          isVisible()   const { return m_visible;  }
    Rotation      getRotation() const { return m_rotation; }
    int16_t       getLayer()    const { return m_layer;    }

private:
    class GameObject*  m_owner;
    const Sprite*      m_sprite;
    const uint16_t*    m_palette;
    int16_t            m_layer;
    Rotation           m_rotation = Rotation::R0;
    bool               m_visible  = true;

    friend class Renderer;
    friend class Animator;
};