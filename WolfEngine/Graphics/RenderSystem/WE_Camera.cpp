#include "WE_Camera.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"

void Camera::initialize() {
    m_position    = Vec2::zero();
    m_zoom        = 1.0f;
    m_target      = nullptr;
    m_followSpeed = 0.1f;
}

// ------------------------------------------------------------------ //
//  Position
// ------------------------------------------------------------------ //

void Camera::setPosition(Vec2 pos)  { m_position = pos; }
Vec2 Camera::getPosition()    const { return m_position; }
void Camera::move(Vec2 delta)       { m_position += delta; }

// ------------------------------------------------------------------ //
//  Zoom
// ------------------------------------------------------------------ //

void  Camera::setZoom(float zoom)  { m_zoom = zoom; }
float Camera::getZoom()      const { return m_zoom; }
void  Camera::zoomIn(float amount) { m_zoom += amount; }
void  Camera::zoomReset()          { m_zoom = 1.0f; }

void Camera::zoomOut(float amount) {
    m_zoom -= amount;
    if (m_zoom < 0.1f) m_zoom = 0.1f;
}

// ------------------------------------------------------------------ //
//  Follow
// ------------------------------------------------------------------ //

void Camera::setTarget(GameObject* target, float speed) {
    m_target      = target;
    m_followSpeed = speed;
}

void Camera::clearTarget() { m_target = nullptr; }

void Camera::follow(Vec2 target)                    { m_position = target; }
void Camera::followSmooth(Vec2 target, float speed) { m_position = lerp(m_position, target, speed); }

void Camera::followTick() {
    if (!m_target) return;

    Vec2 target_pos = m_target->transform.position;

    if (m_followSpeed <= 0.0f)
        follow(target_pos);
    else
        followSmooth(target_pos, m_followSpeed);
}

// ------------------------------------------------------------------ //
//  World <-> Screen
// ------------------------------------------------------------------ //

Vec2 Camera::worldToScreen(Vec2 world_pos) const {
    return {
        (world_pos.x - m_position.x) * m_zoom + RENDER_SETTINGS.gameRegion.centerX(),
        (world_pos.y - m_position.y) * m_zoom + RENDER_SETTINGS.gameRegion.centerY()
    };
}

Vec2 Camera::screenToWorld(Vec2 screen_pos) const {
    return {
        (screen_pos.x - RENDER_SETTINGS.gameRegion.centerX()) / m_zoom + m_position.x,
        (screen_pos.y - RENDER_SETTINGS.gameRegion.centerY()) / m_zoom + m_position.y
    };
}

bool Camera::isVisible(Vec2 world_pos, float margin) const {
    Vec2 screen = worldToScreen(world_pos);
    return screen.x >= RENDER_SETTINGS.gameRegion.x1 - margin && screen.x <= RENDER_SETTINGS.gameRegion.x2 + margin &&
           screen.y >= RENDER_SETTINGS.gameRegion.y1 - margin && screen.y <= RENDER_SETTINGS.gameRegion.y2 + margin;
}

// ------------------------------------------------------------------ //
//  Reset
// ------------------------------------------------------------------ //

void Camera::reset() {
    m_position = Vec2::zero();
    m_zoom     = 1.0f;
    m_target   = nullptr;
}