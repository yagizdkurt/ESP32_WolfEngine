#pragma once
#include <stdint.h>
// =============================================================
//                          LAYERS
// =============================================================

// -------------------------------------------------------------
//  RenderLayer
//  Edit these names to match your game's rendering needs.
//  MAX_LAYERS is automatic — it always equals the count.
//  Layers are drawn in ascending order — layer 0 is drawn first
//  (bottom/background), the highest layer is drawn last (top/UI).
//
//  RAM cost: MAX_LAYERS * MAX_GAME_OBJECTS * 4 bytes
//
//  EXAMPLE SETUP for a typical 2D game:
//      Background — static backdrop, sky, floor tiles
//      World      — general world objects, platforms, terrain
//      Entities   — enemies, NPCs, items
//      Player     — player sprite, always above enemies
//      FX         — particle effects, explosions
// -------------------------------------------------------------
enum class RenderLayer : uint8_t {
// Generally Defaults to Entitites, but can be used for any layer you want to be the default when not specifying a layer.
    Default    = 2,
    DEFAULT    = 2, // alias for Default
    BackGround = 0,
    World      = 1,
    Entities   = 2,
    Player     = 3,
    FX         = 4,
    MAX_LAYERS   // always last — automatically equals layer count
    // MAX_LAYERS is used by the renderer to know how many layers to manage. It is not an actual layer and should always be last in the enum.
    // Do not remove or reorder MAX_LAYERS — it must always be last and will automatically equal the number of layers defined above it.
};

// -------------------------------------------------------------
//  CollisionLayer
//  Edit these names to match your game's collision needs.
//  Uses bitmasks for fast layer-to-layer collision checking.
//  Layers are checked using bitwise AND (&) to determine if
//  two objects should collide (both layer masks must overlap).
//
//  RAM cost: Doesnt change with layer count
//
//  EXAMPLE SETUP for a typical 2D game:
//      DEFAULT    — regular solid objects, platforms
//      Player     — player character
//      Enemy      — enemy characters
//      Wall       — walls, obstacles that block movement
//      Trigger    — trigger zones, area detectors
//      Projectile — bullets, arrows, projectiles
// -------------------------------------------------------------
enum class CollisionLayer : uint16_t {
    DEFAULT    = 1 << 0,
    Player     = 1 << 1,
    Enemy      = 1 << 2,
    Wall       = 1 << 3,
    Trigger    = 1 << 4,
    Projectile = 1 << 5
    // user can add more
};