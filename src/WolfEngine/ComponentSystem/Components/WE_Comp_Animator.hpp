#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp"
#include "WE_Comp_SpriteRenderer.hpp"
#include "WE_BaseComp.hpp"

// =============================================================
//  A named collection of Sprite frames with a built-in count.
//  Always declare as constexpr so frame data lives in flash.
//
//  EXAMPLE:
//      constexpr Sprite WALK_FRAMES[] = { ... };
//      constexpr Animation WALK = Animation::Create(WALK_FRAMES);
// =============================================================
struct Animation {
    const Sprite* frames;
    uint8_t       frameCount;
    template<int N> static constexpr Animation Create(const Sprite (&frames)[N]) { return { frames, static_cast<uint8_t>(N) }; }
};


// =============================================================
//  WE_Comp_Animator.hpp
//  The Animator component drives a SpriteRenderer by cycling
//  through an Animation's Sprite frames automatically.
//
//  It holds a pointer to the SpriteRenderer it controls and
//  advances frames based on a configurable frame duration.
//  The engine ticks the animator every frame — no manual
//  tick() call needed in Update().
//
//  USAGE:
//      constexpr uint8_t walkPixels0[7][7] = { ... };
//      constexpr uint8_t walkPixels1[7][7] = { ... };
//      constexpr uint8_t walkPixels2[7][7] = { ... };
//      constexpr uint8_t walkPixels3[7][7] = { ... };
//
//      constexpr Sprite WALK_FRAMES[] = {
//          Sprite::Create(walkPixels0, PALETTE_WARM),
//          Sprite::Create(walkPixels1, PALETTE_WARM),
//          Sprite::Create(walkPixels2, PALETTE_WARM),
//          Sprite::Create(walkPixels3, PALETTE_WARM),
//      };
//
//      constexpr Animation WALK = Animation::Create(WALK_FRAMES);
//
//      class Player : public GameObject {
//      public:
//          SpriteRenderer spriteRenderer = SpriteRenderer(this, &WALK_FRAMES[0], LAYER_PLAYER);
//          Animator       animator       = Animator(&spriteRenderer, WALK, 8);
//      };
// =============================================================

class Animator : public Component {
public:
    // owner          — pointer to the SpriteRenderer this animator controls
    // frames         — array of Sprite assets representing animation frames
    // frameCount     — number of frames in the array
    // frameDuration  — how many game ticks each frame is shown for
    Animator(SpriteRenderer* owner, const Animation& animation, uint8_t frameDuration = 8);

    void setFrame(uint8_t frame); // Jump to a specific frame immediately

    void setAnimation(const Animation& animation); // Change the entire animation

    // Change how many ticks each frame is shown for
    void setFrameDuration(uint8_t duration) { m_frameDuration = duration; }

    uint8_t getCurrentFrame() const { return m_currentFrame; }

    void pause()          { m_paused =  true;  }
    void resume()         { m_paused = false;  }
    bool isPaused() const { return  m_paused;  }

private:
    void tick() override; 

    SpriteRenderer* m_mySpriteRenderer;
    const Sprite*   m_frames;
    uint8_t         m_frameCount;
    uint8_t         m_frameDuration;
    uint8_t         m_currentFrame = 0;
    uint8_t         m_tickCounter  = 0;
    bool            m_paused       = false;
};