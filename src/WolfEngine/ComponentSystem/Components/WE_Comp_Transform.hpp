#pragma once
#include "WE_BaseComp.hpp"
#include "WolfEngine/Utilities/WE_Vector2d.hpp"
//=============================================================================
//                  TRANSFORM COMPONENT
//=============================================================================

struct TransformComponent : public Component {
public:
    Vec2 position;
    uint8_t width;
    uint8_t height;

    TransformComponent() {
        type = COMP_TRANSFORM; // Set the component type to transform
        position = {0, 0}; // Default position at the origin
        width = 1; // Default width
        height = 1; // Default height
    }
};