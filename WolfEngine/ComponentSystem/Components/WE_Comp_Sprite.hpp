#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/RenderSystem/WE_SpriteData.hpp"
#include "WolfEngine/Settings/WE_RenderLayers.hpp"
#include "WE_BaseComp.hpp"

// =============================================================
//  WE_Comp_Sprite.hpp
//  WolfEngine Sprite Component
//
//  Attach a Sprite to any GameObject to make it renderable.
//  The sprite holds a pointer to pixel data and a palette,
//  both defined as constexpr arrays in flash by the user.
//  The renderer reads this data each frame via getRenderData()
//  and handles all drawing logic itself.
//
//  USAGE
//  -----
//  Define your sprite pixel data as a constexpr array in flash,
//  then attach a Sprite to your GameObject:
//
//      // MyObject_Sprites.hpp
//      constexpr uint8_t SPRITE_PLAYER[16 * 16] = {
//          // 0 0 0 1 1 1 0 0 0 0 0 1 1 1 0 0   <- row 0
//             0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,
//          // ... remaining rows
//      };
//
//      // MyObject.hpp
//      #include "WolfEngine/ComponentSystem/Components/WE_Comp_Sprite.hpp"
//      #include "MyObject_Sprites.hpp"
//
//      class MyObject : public GameObject {
//      public:
//          MyObject() : m_sprite(this, SPRITE_PLAYER, 16, PALETTE_WARM, 0) {}
//      private:
//          Sprite m_sprite;
//      };
//
//  The sprite registers itself with the renderer on construction
//  and unregisters itself on destruction automatically.
//  You do not need to manage this manually.
//
//  PIXEL DATA FORMAT
//  -----------------
//  Pixel arrays must be constexpr uint8_t of exactly size*size bytes.
//  Each byte is a palette index (0-31). Index 0 is transparent.
//  Row-major order, left to right, top to bottom.
//
//  Example comment map convention for authoring sprite data:
//      // 0 = transparent, 1 = outline, 2 = fill, 3 = highlight
//      constexpr uint8_t SPRITE_COIN[8 * 8] = {
//          // 0 0 1 1 1 1 0 0
//             0,0,1,1,1,1,0,0,
//          // 0 1 2 2 2 2 1 0
//             0,1,2,2,2,2,1,0,
//          // ...
//      };
//
//  VALID SIZES
//  -----------
//  Any of: 4, 8, 16, 32, 64
//  Passing a size that does not match the actual pixel array
//  length will result in incorrect rendering.
// =============================================================

class Renderer;


// -------------------------------------------------------------
//  Sprite
//  A sprite component that can be attached to any GameObject.
//  Holds pointers to pixel data and palette in flash, plus
//  rotation and visibility state. The renderer reads this data automatically.
//
//  Construction:
//    Sprite(owner, pixels, size, palette, layer = 0)
//
//    owner   — pointer to the owning GameObject (pass 'this')
//    pixels  — constexpr uint8_t[size*size] pixel index array in flash
//    size    — sprite dimensions in pixels NxN, must be 4/8/16/32/64
//    palette — constexpr uint16_t[32] palette array in flash
//    layer   — render layer (0 = bottom, higher = drawn on top)
// -------------------------------------------------------------
class Sprite : public Component {
public:

    // Construct and auto-register with the renderer.
    //
    // owner   — pointer to the owning GameObject (pass 'this')
    // pixels  — constexpr uint8_t[size*size] pixel index array in flash
    // size    — sprite dimensions in pixels NxN, must be 4/8/16/32/64
    // palette — constexpr uint16_t[32] palette array in flash
    // layer   — render layer (0 = bottom, higher = drawn on top)
    Sprite(class GameObject*  owner,
           const uint8_t*     pixels,
           int                size,
           const uint16_t*    palette,
           int                layer = 0);

    // Destructor automatically unregisters from the renderer.
    ~Sprite();

    // Returns all data the renderer needs to draw this sprite.
    // Reads the owner's current position so it is always up to date.
    SpriteData getRenderData() const;

    // ---------------------------------------------------------
    //  Runtime controls
    // ---------------------------------------------------------

    // Swap the active palette.
    // Useful for color variants, damage flash, day/night effects.
    // The new palette must be a constexpr uint16_t[32] in flash.
    void setPalette(const uint16_t* palette) { m_palette  = palette;   }

    // Change the current snap rotation.
    void setRotation(Rotation rotation)      { m_rotation = rotation;  }

    // Change the sprite pointer. Useful for animation or morphing effects.
    void setPixels(const uint8_t* pixels)    { m_pixels = pixels; }

    // Show or hide without removing from the render layer.
    // Hidden sprites are skipped by the renderer each frame.
    void setVisible(bool visible)            { m_visible  = visible;   }

    // ---------------------------------------------------------
    //  Getters
    // ---------------------------------------------------------
    bool     isVisible()   const { return m_visible;  }
    Rotation getRotation() const { return m_rotation; }
    int      getLayer()    const { return m_layer;    }
    int      getSize()     const { return m_size;     }

private:
    class GameObject*  m_owner;                  // Owning object, for position lookup
    const uint8_t*     m_pixels;                 // Pixel index data in flash
    const uint16_t*    m_palette;                // Active palette in flash
    int                m_size;                   // Sprite width and height in pixels
    int                m_layer;                  // Render layer this sprite sits on
    Rotation           m_rotation = Rotation::R0;
    bool               m_visible  = true;
};