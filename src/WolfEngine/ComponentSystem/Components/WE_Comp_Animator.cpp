#include "WE_Comp_Animator.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"

Animator::Animator(SpriteRenderer*  mySpriteRenderer,
                   const Animation& animation,
                   uint8_t          frameDuration)
    : m_mySpriteRenderer (mySpriteRenderer)
    , m_frames           (animation.frames)
    , m_frameCount       (animation.frameCount)
    , m_frameDuration    (frameDuration)
{
    type        = COMP_ANIMATOR;
    tickEnabled = true;
    if (m_mySpriteRenderer && m_frames) m_mySpriteRenderer->setSprite(&m_frames[0]);
    if (m_mySpriteRenderer) m_mySpriteRenderer->m_owner->registerComponent(this);
}

void Animator::tick() {
    if (m_paused || !m_mySpriteRenderer || !m_frames) return;

    m_tickCounter++;
    if (m_tickCounter >= m_frameDuration) {
        m_tickCounter = 0;
        m_currentFrame = (m_currentFrame + 1) % m_frameCount;
        m_mySpriteRenderer->setSprite(&m_frames[m_currentFrame]);
    }
}

void Animator::setFrame(uint8_t frame) {
    if (frame >= m_frameCount) return;
    m_currentFrame = frame;
    m_tickCounter  = 0;
    if (m_mySpriteRenderer) m_mySpriteRenderer->setSprite(&m_frames[m_currentFrame]);
}

void Animator::setAnimation(const Animation& animation) {
    m_frames       = animation.frames;
    m_frameCount   = animation.frameCount;
    m_currentFrame = 0;
    m_tickCounter  = 0;
    if (m_mySpriteRenderer) m_mySpriteRenderer->setSprite(&m_frames[0]);
}
