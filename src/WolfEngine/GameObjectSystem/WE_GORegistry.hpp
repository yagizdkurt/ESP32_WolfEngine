#pragma once
#include "WE_GameObject.hpp"
#include "WolfEngine/Settings/WE_Settings.hpp"

// =============================================================================
//                  GAME OBJECT REGISTRY
// =============================================================================

typedef struct {
    GameObject* gameObjects[MAX_GAME_OBJECTS];
    uint8_t count;
} GameObjectRegistry;