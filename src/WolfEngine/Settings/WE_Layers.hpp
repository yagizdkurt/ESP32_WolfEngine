#pragma once
#include <stdint.h>

// =============================================================
//  Render layers
// =============================================================
enum class RenderLayer : uint8_t {
    Default    = 2,
    DEFAULT    = 2,
    BackGround = 0,
    World      = 1,
    Entities   = 2,
    Player     = 3,
    FX         = 4,
    MAX_LAYERS
};

// =============================================================
//  Collision layers (bitmask)
// =============================================================
enum class CollisionLayer : uint16_t {
    DEFAULT    = 1 << 0,
    Player     = 1 << 1,
    Enemy      = 1 << 2,
    Wall       = 1 << 3,
    Trigger    = 1 << 4,
    Projectile = 1 << 5
};

// Bitmask flags for which update layers run in a given state.
// Plain enum (not enum class) so values can be OR'd freely without casting.
enum UpdateLayer : uint16_t {
    UL_UPDATE_GAMEPLAY = 1 << 0,
    UL_UPDATE_PHYSICS  = 1 << 1,
    UL_UPDATE_AI       = 1 << 2,
    UL_UPDATE_UI       = 1 << 3,
    UL_UPDATE_INPUT    = 1 << 4,
};