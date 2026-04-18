#pragma once
#include <stdint.h>

// =============================================================
//  WE_Palette_Cool
//  Blues, greens, teals, and cool neutrals.
//  Useful for water, ice, forests, night scenes, and UI themes.
//
//  Index 0 is always transparent (reserved by the engine).
// =============================================================

enum Cool : uint8_t {
    PL_CL_Transparent   = 0,
    PL_CL_NearBlackBlue = 1,
    PL_CL_VeryDarkNavy  = 2,
    PL_CL_DarkNavy      = 3,
    PL_CL_Navy          = 4,
    PL_CL_DarkTeal      = 5,
    PL_CL_Teal          = 6,
    PL_CL_PureBlue      = 7,
    PL_CL_OceanBlue     = 8,
    PL_CL_BrightBlue    = 9,
    PL_CL_VividBlue     = 10,
    PL_CL_SkyBlueDark   = 11,
    PL_CL_SkyBlue       = 12,
    PL_CL_Cyan          = 13,
    PL_CL_BrightCyan    = 14,
    PL_CL_VeryDarkGreen = 15,
    PL_CL_DarkForest    = 16,
    PL_CL_Forest        = 17,
    PL_CL_MedGreen      = 18,
    PL_CL_Green         = 19,
    PL_CL_PureGreen     = 20,
    PL_CL_YellowGreen   = 21,
    PL_CL_LimeGreen     = 22,
    PL_CL_BrightLime    = 23,
    PL_CL_GreenCyan     = 24,
    PL_CL_Seafoam       = 25,
    PL_CL_Mint          = 26,
    PL_CL_LightMint     = 27,
    PL_CL_IceBlue       = 28,
    PL_CL_PaleIce       = 29,
    PL_CL_Frost         = 30,
    PL_CL_NearWhite     = 31,
};

constexpr uint16_t PALETTE_COOL[32] = {
    0x0000,  // 0  - Transparent (reserved)
    0x0008,  // 1  - Near black blue
    0x000C,  // 2  - Very dark navy
    0x0010,  // 3  - Dark navy
    0x0014,  // 4  - Navy
    0x0198,  // 5  - Dark teal
    0x0278,  // 6  - Teal
    0x001F,  // 7  - Pure blue
    0x02DF,  // 8  - Ocean blue
    0x045F,  // 9  - Bright blue
    0x065F,  // 10 - Vivid blue
    0x07BF,  // 11 - Sky blue dark
    0x03EF,  // 12 - Sky blue
    0x07FF,  // 13 - Cyan
    0x03FF,  // 14 - Bright cyan
    0x0220,  // 15 - Very dark green
    0x0340,  // 16 - Dark forest green
    0x0440,  // 17 - Forest green
    0x0480,  // 18 - Medium green
    0x0520,  // 19 - Green
    0x07E0,  // 20 - Pure green
    0x27E0,  // 21 - Yellow green
    0x47E0,  // 22 - Lime green
    0x67E0,  // 23 - Bright lime
    0x07F0,  // 24 - Green cyan
    0x47F8,  // 25 - Seafoam
    0x87F0,  // 26 - Mint
    0xAFF8,  // 27 - Light mint
    0xAFFF,  // 28 - Ice blue
    0xC7FF,  // 29 - Pale ice
    0xE7FF,  // 30 - Frost
    0xF7FF,  // 31 - Near white cool
};