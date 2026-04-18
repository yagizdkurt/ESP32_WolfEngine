#pragma once
#include <stdint.h>

// =============================================================
//  WE_Palette_Warm
//  Reds, oranges, yellows, earth tones, and warm neutrals.
//  Useful for fire, lava, desert, autumn, and warm-lit scenes.
//
//  Index 0 is always transparent (reserved by the engine).
// =============================================================

enum Warm : uint8_t {
    PL_WM_Transparent   = 0,
    PL_WM_NearBlackRed  = 1,
    PL_WM_VeryDarkRed   = 2,
    PL_WM_DarkMaroon    = 3,
    PL_WM_DarkRed       = 4,
    PL_WM_MedDarkRed    = 5,
    PL_WM_Rust          = 6,
    PL_WM_MediumRed     = 7,
    PL_WM_PureRed       = 8,
    PL_WM_RedOrange     = 9,
    PL_WM_DarkOrange    = 10,
    PL_WM_Orange        = 11,
    PL_WM_MedOrange     = 12,
    PL_WM_BrightOrange  = 13,
    PL_WM_AmberDark     = 14,
    PL_WM_Amber         = 15,
    PL_WM_GoldDark      = 16,
    PL_WM_Yellow        = 17,
    PL_WM_Gold          = 18,
    PL_WM_DarkGold      = 19,
    PL_WM_DarkSienna    = 20,
    PL_WM_Sienna        = 21,
    PL_WM_LightSienna   = 22,
    PL_WM_DarkBrown     = 23,
    PL_WM_MedBrown      = 24,
    PL_WM_LightBrown    = 25,
    PL_WM_Wheat         = 26,
    PL_WM_Salmon        = 27,
    PL_WM_LightSalmon   = 28,
    PL_WM_Peach         = 29,
    PL_WM_LightPeach    = 30,
    PL_WM_Cream         = 31,
};

constexpr uint16_t PALETTE_WARM[32] = {
    0x0000,  // 0  - Transparent (reserved)
    0x2000,  // 1  - Near black red
    0x3000,  // 2  - Very dark red
    0x5000,  // 3  - Dark maroon
    0x8000,  // 4  - Dark red
    0xA000,  // 5  - Medium dark red
    0xB201,  // 6  - Rust
    0xC000,  // 7  - Medium red
    0xF800,  // 8  - Pure red
    0xF8C0,  // 9  - Red orange
    0xFC60,  // 10 - Dark orange
    0xFC80,  // 11 - Orange
    0xFD00,  // 12 - Medium orange
    0xFD20,  // 13 - Bright orange
    0xFE00,  // 14 - Amber dark
    0xFF00,  // 15 - Amber
    0xFFA0,  // 16 - Gold dark
    0xFFE0,  // 17 - Yellow
    0xFEA0,  // 18 - Gold
    0xFD60,  // 19 - Dark gold
    0x8284,  // 20 - Dark sienna
    0xA285,  // 21 - Sienna
    0xC2C6,  // 22 - Light sienna
    0x6204,  // 23 - Dark brown
    0x8206,  // 24 - Medium brown
    0xA228,  // 25 - Light brown
    0xF6F6,  // 26 - Wheat
    0xFC0E,  // 27 - Salmon
    0xFBAD,  // 28 - Light salmon
    0xFBEA,  // 29 - Peach
    0xFCF3,  // 30 - Light peach
    0xFFFA,  // 31 - Cream
};