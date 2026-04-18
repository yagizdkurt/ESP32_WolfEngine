#pragma once
#include <stdint.h>

// =============================================================
//  WE_Palette_GameBoy
//  Inspired by the original DMG Game Boy LCD green palette.
//  Extends the classic 4-shade look into 31 tones while keeping
//  the iconic green-tinted monochrome aesthetic.
//
//  Index 0 is always transparent (reserved by the engine).
// =============================================================

enum Gameboy : uint8_t {
    PL_GB_Transparent   = 0,
    PL_GB_DarkestGreen  = 1,
    PL_GB_DarkGreen2    = 2,
    PL_GB_DarkGreen3    = 3,
    PL_GB_DarkGreen4    = 4,
    PL_GB_DarkGreen5    = 5,
    PL_GB_DarkGreen6    = 6,
    PL_GB_DarkGreen7    = 7,
    PL_GB_DarkGreen8    = 8,
    PL_GB_DarkGreen9    = 9,
    PL_GB_DarkGreen10   = 10,
    PL_GB_MidDark       = 11,
    PL_GB_MidDark2      = 12,
    PL_GB_MidDark3      = 13,
    PL_GB_MidTone1      = 14,
    PL_GB_MidTone       = 15,
    PL_GB_MidTone2      = 16,
    PL_GB_MidTone3      = 17,
    PL_GB_MidTone4      = 18,
    PL_GB_MidTone5      = 19,
    PL_GB_MidTone6      = 20,
    PL_GB_MidLight1     = 21,
    PL_GB_MidLight2     = 22,
    PL_GB_Light         = 23,
    PL_GB_Light2        = 24,
    PL_GB_LightGreen1   = 25,
    PL_GB_LightGreen2   = 26,
    PL_GB_LightGreen3   = 27,
    PL_GB_VeryLight     = 28,
    PL_GB_VeryLight2    = 29,
    PL_GB_VeryLight3    = 30,
    PL_GB_Lightest      = 31,
};

constexpr uint16_t PALETTE_GAMEBOY[32] = {
    0x0000,  // 0  - Transparent (reserved)
    0x00E0,  // 1  - Darkest / near black green
    0x0120,  // 2
    0x0160,  // 3
    0x01A0,  // 4
    0x01E0,  // 5
    0x0220,  // 6
    0x0260,  // 7
    0x02A0,  // 8
    0x02E0,  // 9
    0x0320,  // 10
    0x0360,  // 11 - Mid dark
    0x03A0,  // 12
    0x03E0,  // 13
    0x0BE0,  // 14
    0x13E0,  // 15 - Mid tone
    0x1BE0,  // 16
    0x23E0,  // 17
    0x2BE0,  // 18
    0x33E0,  // 19
    0x3BE0,  // 20
    0x4BE0,  // 21
    0x5BE0,  // 22
    0x6BE0,  // 23 - Light
    0x7BE0,  // 24
    0x8BE1,  // 25
    0x9BE2,  // 26
    0xABE3,  // 27
    0xBBE4,  // 28 - Very light
    0xCBE5,  // 29
    0xDBE6,  // 30
    0xEBE7,  // 31 - Lightest / near white green
};