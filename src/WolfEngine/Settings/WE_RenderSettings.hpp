#pragma once
#include <stdint.h>

// -------------------------------------------------------------
//  GAME_REGION { x1, y1, x2, y2 }
//  Defines the rectangular area of the screen used for game
//  rendering. The camera centers to the middle of this region.
//  Sprites are culled and clipped against this rectangle.
//
//  x1, y1 — top-left corner of the game region (inclusive)
//  x2, y2 — bottom-right corner of the game region (exclusive)
//
//  Example — full 128x128 screen with no UI:
//      constexpr Region GAME_REGION = { 0, 0, 128, 128 };
//
//  Example — 128x108 game region, leaving 20 rows for UI:
//      constexpr Region GAME_REGION = { 0, 0, 128, 108 };
// -------------------------------------------------------------
struct Region {
    int x1, y1, x2, y2;
    constexpr int width()  const { return x2 - x1; }
    constexpr int height() const { return y2 - y1; }
    constexpr int centerX() const { return (x1 + x2) / 2; }
    constexpr int centerY() const { return (y1 + y2) / 2; }
};

struct RenderSettings {

    // Default background pixel color in RGB565 format (16-bit): RRRRRGGGGGGBBBBB
    // Common values:
    //     0x0000 — Black
    //     0xFFFF — White
    uint16_t defaultBackgroundPixel;

    // Game region defines the rectangular area of the screen used for game rendering.
    // Explained in detail in the Region struct definition in WE_RenderSettings.hpp. 
    // This is where you set your desired game region based on your game's layout and UI needs.
    Region gameRegion;

    // Set to false to disable sprite rendering. If you want set framebuffer pixels directly...
    bool spriteSystemEnabled = true;

    // Set to false to disable automatic clearing of the framebuffer each frame.
    bool cleanFramebufferEachFrame = true;
};

// Maximum number of DrawCommands that can be submitted per frame.
// Tune this based on peak sprite count (check FrameDiagnostics::peakCommandCount).
static constexpr uint16_t MAX_DRAW_COMMANDS = 128;
