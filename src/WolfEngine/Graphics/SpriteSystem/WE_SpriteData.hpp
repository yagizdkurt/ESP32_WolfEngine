#pragma once
#include <stdint.h>
#include "WE_SpriteRotation.hpp"

// =============================================================
//  WE_SpriteData.hpp       [ENGINE MANAGED INTERNAL STRUCTURE]
// =============================================================

struct SpriteData {
    const uint8_t*  pixels;
    const uint16_t* palette;
    int             x;
    int             y;
    int             size;
    Rotation        rotation;
    bool            visible;
};