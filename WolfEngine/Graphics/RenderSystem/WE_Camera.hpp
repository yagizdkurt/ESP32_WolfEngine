#pragma once
#include "WolfEngine/Utilities/WE_Vector2d.hpp"
#include "WolfEngine/Settings/WE_Settings.hpp"

class GameObject;
class Camera {
public:
    // Position
    void setPosition(Vec2 pos);
    Vec2 getPosition() const;
    void move(Vec2 delta);

    // Zoom
    void  setZoom(float zoom);
    float getZoom() const;
    void  zoomIn(float amount);
    void  zoomOut(float amount);
    void  zoomReset();

    // Follow
    void setTarget(GameObject* target, float speed = 0.1f);
    void clearTarget();
    void follow(Vec2 target);
    void followSmooth(Vec2 target, float speed);
    void followTick();

    // World <-> Screen
    Vec2 worldToScreen(Vec2 world_pos) const;
    Vec2 screenToWorld(Vec2 screen_pos) const;
    bool isVisible(Vec2 world_pos, float margin = 0.0f) const;

    void reset();

private:
    Camera(){}
    Vec2        m_position;
    float       m_zoom        = 1.0f;
    float       m_followSpeed = 0.1f;
    GameObject* m_target      = nullptr;

    void initialize();

    friend class WolfEngine;
};