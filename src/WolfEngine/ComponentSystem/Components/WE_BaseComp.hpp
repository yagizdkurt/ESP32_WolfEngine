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
    bool earlyTickEnabled = false;
    bool lateTickEnabled = false;
    bool preRenderTickEnabled = false;

    virtual ~Component() = default;
    friend class GameObject;

    private:
    virtual void earlyTick() { }
    virtual void tick() { }
    virtual void lateTick() { }
    virtual void preRenderTick() { }
};