#pragma once
#include "WE_PINDEFS.hpp"
#include "WE_InputSettings.hpp"
#include "WE_RenderLayers.hpp"
#include "WE_RenderSettings.hpp"

/*
============================================================================================
WOLF ENGINE PRECOMPİLE SETTINGS HEADER FILE
This file contains all the necessary settings and configurations for the Wolf Engine conditional compilation. 
It only includes settings that matters before the engine is compiled, such as display target for the renderer,
input device configurations, audio settings, and other engine-wide configurations. Not like settings for game.
Never include this in any of your files. Include WolfEngine.hpp instead.
============================================================================================
*/

#pragma region InputSettings

// =================== INPUT SETTINGS =======================
// QUICK SETUP:
//   1. If buttons are wired directly to the ESP32, fill in gpioPins below.
//   2. If buttons are routed through a PCF8574, set INPUT_PCF8574_ADDR to the
//      chip's 7-bit address and fill in expanderPins instead.
//   3. Set unused pins to -1 to disable them entirely.
//   4. Joystick axes are always direct GPIO (ADC) — the PCF8574 is digital only.
// ──────────────────────────────────────────────────────────────────────────────

constexpr InputSettings INPUT_SETTINGS = {
    // Button count is set in InputSettings struct itself in WE_InputSettings.hpp, not here.

                        // A    B    C    D    E    F    K
    .gpioPins        = {  27,  -1,  -1,  -1,  -1,  -1,  -1 }, 

    .pcf8574Addr    = -1, // Set to -1 to disable expander support entirely.

                        // A    B    C    D    E    F    K
    .expanderPins    = {  -1,  -1,  -1,  -1,  -1,  -1,  -1 },

    .joyXEnabled  = true,
    .joyXChannel  = ADC_CHANNEL_0,
    .joyXMin      = 0,
    .joyXMax      = 4095,
    .joyXCenter   = 2048,

    .joyYEnabled  = false,
    .joyYChannel  = ADC_CHANNEL_0,  // ignored when joyYEnabled is false
    .joyYMin      = 0,
    .joyYMax      = 4095,
    .joyYCenter   = 2048,

    .joyDeadzone     = 0.1f,
    .activeLow       = true,
    .debounceMs      = 20,
    .pollIntervalMs  = 10
};

#pragma endregion


// ==================== ENGINE GENERAL SETTINGS ==================

// Target frame time in microseconds (1,000,000 / 30 = 33,333us)
#define TARGET_FRAME_TIME_US 33333

// =================== GAME OBJECT SETTINGS =======================
#define MAX_GAME_OBJECTS 64


#pragma region RenderSettings

// =================== RENDERER SETTINGS =======================

// ==== Display Target ====
// Define the target display for the renderer. Only one should be defined at a time.
#define DISPLAY_ST7735
// #define DISPLAY_CUSTOM

// -------------------------------------------------------------
//  RENDERER_SETTINGS
//
//  defaultBackgroundPixel — color shown anywhere no sprite covers the screen
//  gameRegion             — rectangular area of the screen used for game rendering
// -------------------------------------------------------------
constexpr RenderSettings RENDER_SETTINGS = {

    // Background color shown anywhere no sprite covers the screen.
    // RGB565 format (16-bit): RRRRRGGGGGGBBBBB
    // Common values: 0x0000 Black, 0xFFFF White, 0xF800 Red, 0x001F Blue, 0x07E0 Green
    .defaultBackgroundPixel = 0x0000,

    // Rectangular area of the screen used for game rendering. { x1, y1, x2, y2 }
    // Camera centers to the middle of this region.
    // Sprites are culled and clipped against this rectangle.
    // x1, y1 — top-left corner (inclusive), x2, y2 — bottom-right corner (exclusive)
    .gameRegion = { 0, 0, 128, 108 } // 128x108 game area, leaving 20 rows for UI
};

#pragma endregion



