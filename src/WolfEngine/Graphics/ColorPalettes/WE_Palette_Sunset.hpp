#pragma once
#include <stdint.h>

// =============================================================
//  WE_Palette_Sunset
//  Deep purples, magentas, oranges, and warm pinks.
//  Useful for evening scenes, neon/synthwave aesthetics,
//  and dramatic sky or background gradients.
//
//  Index 0 is always transparent (reserved by the engine).
// =============================================================

enum Sunset : uint8_t {
    PL_SS_Transparent     = 0,
    PL_SS_NearBlackPurple = 1,
    PL_SS_VeryDarkIndigo  = 2,
    PL_SS_DarkIndigo      = 3,
    PL_SS_DeepIndigo      = 4,
    PL_SS_DeepPurple      = 5,
    PL_SS_PurpleDark      = 6,
    PL_SS_Purple          = 7,
    PL_SS_MedPurple       = 8,
    PL_SS_PurpleMid       = 9,
    PL_SS_Violet          = 10,
    PL_SS_VioletBright    = 11,
    PL_SS_MagentaPurple   = 12,
    PL_SS_DarkMagenta     = 13,
    PL_SS_PureMagenta     = 14,
    PL_SS_BrightMagenta   = 15,
    PL_SS_HotPink         = 16,
    PL_SS_PinkDark        = 17,
    PL_SS_Pink            = 18,
    PL_SS_MedPink         = 19,
    PL_SS_SalmonPink      = 20,
    PL_SS_LightSalmon     = 21,
    PL_SS_DarkOrange      = 22,
    PL_SS_Orange          = 23,
    PL_SS_BrightOrange    = 24,
    PL_SS_Amber           = 25,
    PL_SS_Gold            = 26,
    PL_SS_WarmYellow      = 27,
    PL_SS_LightYellow     = 28,
    PL_SS_PaleYellow      = 29,
    PL_SS_OffWhite        = 30,
    PL_SS_White           = 31,
};

constexpr uint16_t PALETTE_SUNSET[32] = {
    0x0000,  // 0  - Transparent (reserved)
    0x1008,  // 1  - Near black purple
    0x1810,  // 2  - Very dark indigo
    0x2010,  // 3  - Dark indigo
    0x300F,  // 4  - Deep indigo
    0x400F,  // 5  - Deep purple
    0x580F,  // 6  - Purple dark
    0x680F,  // 7  - Purple
    0x780F,  // 8  - Medium purple
    0x880F,  // 9  - Purple mid
    0x980F,  // 10 - Violet
    0xA80F,  // 11 - Violet bright
    0xB817,  // 12 - Magenta purple
    0xC81F,  // 13 - Dark magenta
    0xF81F,  // 14 - Pure magenta
    0xF91F,  // 15 - Bright magenta
    0xF90F,  // 16 - Hot pink
    0xF96F,  // 17 - Pink dark
    0xF9AF,  // 18 - Pink
    0xFAAF,  // 19 - Medium pink
    0xFBAD,  // 20 - Salmon pink
    0xFC8D,  // 21 - Light salmon
    0xFC60,  // 22 - Dark orange
    0xFD00,  // 23 - Orange
    0xFD20,  // 24 - Bright orange
    0xFE60,  // 25 - Amber
    0xFEA0,  // 26 - Gold
    0xFF4A,  // 27 - Warm yellow
    0xFF8A,  // 28 - Light yellow
    0xFFCA,  // 29 - Pale yellow
    0xFFEF,  // 30 - Off white warm
    0xFFFF,  // 31 - White
};