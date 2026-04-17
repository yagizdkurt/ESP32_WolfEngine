#pragma once

enum ComponentType {
    COMP_TRANSFORM,
    COMP_SPRITE,
    COMP_ANIMATOR,
    COMP_COLLIDER,
    COMP_MAX_TYPE
};

struct Component {
    ComponentType type;
    bool tickEnabled = false;
    friend class GameObject;
    virtual void tick() { }
    virtual ~Component() = default;
};