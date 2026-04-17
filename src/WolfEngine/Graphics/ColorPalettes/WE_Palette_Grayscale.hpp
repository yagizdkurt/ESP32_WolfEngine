#pragma once
#include <stdint.h>

// =============================================================
//  WE_Palette_Grayscale
//  31 shades from near-black to white.
//  Useful for UI, shadows, monochrome sprites, and debug visuals.
//
//  Index 0 is always transparent (reserved by the engine).
//  Shades progress linearly from darkest (1) to white (31).
// =============================================================

enum Grayscale : uint8_t {
    PL_GS_Transparent = 0,
    PL_GS_Shade1      = 1,
    PL_GS_Shade2      = 2,
    PL_GS_Shade3      = 3,
    PL_GS_Shade4      = 4,
    PL_GS_Shade5      = 5,
    PL_GS_Shade6      = 6,
    PL_GS_Shade7      = 7,
    PL_GS_Shade8      = 8,
    PL_GS_Shade9      = 9,
    PL_GS_Shade10     = 10,
    PL_GS_Shade11     = 11,
    PL_GS_Shade12     = 12,
    PL_GS_Shade13     = 13,
    PL_GS_MidGray     = 14,
    PL_GS_Shade15     = 15,
    PL_GS_Shade16     = 16,
    PL_GS_Shade17     = 17,
    PL_GS_Shade18     = 18,
    PL_GS_Shade19     = 19,
    PL_GS_Shade20     = 20,
    PL_GS_Shade21     = 21,
    PL_GS_Shade22     = 22,
    PL_GS_Shade23     = 23,
    PL_GS_Shade24     = 24,
    PL_GS_Shade25     = 25,
    PL_GS_Shade26     = 26,
    PL_GS_Shade27     = 27,
    PL_GS_Shade28     = 28,
    PL_GS_Shade29     = 29,
    PL_GS_Shade30     = 30,
    PL_GS_White       = 31,
};

constexpr uint16_t PALETTE_GRAYSCALE[32] = {
    0x0000,  // 0  - Transparent (reserved)
    0x0841,  // 1  - Near black
    0x1082,  // 2
    0x18C3,  // 3
    0x2104,  // 4
    0x2945,  // 5
    0x31A6,  // 6
    0x39E7,  // 7
    0x4228,  // 8
    0x4A69,  // 9
    0x52AA,  // 10
    0x5AEB,  // 11
    0x632C,  // 12
    0x6B6D,  // 13
    0x738E,  // 14 - Mid gray
    0x7BCF,  // 15
    0x8410,  // 16
    0x8C51,  // 17
    0x9492,  // 18
    0x9CD3,  // 19
    0xA514,  // 20
    0xAD55,  // 21
    0xB596,  // 22
    0xBDD7,  // 23
    0xC618,  // 24
    0xCE59,  // 25
    0xD69A,  // 26
    0xDEDB,  // 27
    0xE71C,  // 28
    0xEF5D,  // 29
    0xF79E,  // 30
    0xFFFF,  // 31 - White
};