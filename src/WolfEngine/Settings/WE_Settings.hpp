#pragma once
#include "WE_PINDEFS.hpp"
#include "WE_InputSettings.hpp"
#include "WE_Layers.hpp"
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
//   1. Set enabled = true for each controller slot you are using (up to 4).
//
//   2. Assign ESP32 GPIO pins in gpioPins for buttons wired directly to the ESP32.
//
//   3. If buttons are routed through an I/O expander, set expander.type to the
//      correct chip, expander.addr to its I2C address, and fill in expander.pins.
//
//   4. Set unused pins to -1 to disable them entirely.
//
//   5. If using a joystick, set joyXEnabled/joyYEnabled to true and assign
//      the correct ADC channels and calibration values.
//
//   6. debounceMs and pollIntervalMs apply to all controllers globally.
// ──────────────────────────────────────────────────────────────────────────────

constexpr InputSettings INPUT_SETTINGS = {
    .debounceMs     = 20,

    .pollIntervalMs = 10,

    // ----------------- CONTROLLERS -----------------
    // Configuration for up to 4 controllers (players 1–4).
    .controllers    = {

        {   // -------------- CONTROLLER 1 --------------
            .enabled     = true,
                           // A    B    C    D    E    F    G    H    I    J
            .gpioPins    = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
            .expander    = { ExpanderType::PCF8574, 0x20, {0,1,2,3,-1,-1,-1,-1,-1,-1} },
            .joyXEnabled = true,
            .joyYEnabled = true,
            .joyXChannel = ADC_CHANNEL_6,
            .joyYChannel = ADC_CHANNEL_7,
            .joyXMin = 0, .joyXMax = 4095, .joyXCenter = 2048,
            .joyYMin = 0, .joyYMax = 4095, .joyYCenter = 2048,
            .joyDeadzone = 0.1f,
            .activeLow   = true,
        },
    
        {   // ------------- CONTROLLER 2 --------------
            .enabled     = false,
                           // A    B    C    D    E    F    G    H    I    J
            .gpioPins    = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
            .expander    = { ExpanderType::PCF8574, 0x20, {1,-1,-1,-1,-1,-1,-1,-1,-1,-1} },
            .joyXEnabled = false,
            .joyYEnabled = false,
            .joyXChannel = ADC_CHANNEL_0,
            .joyYChannel = ADC_CHANNEL_0,
            .joyXMin = 0, .joyXMax = 4095, .joyXCenter = 2048,
            .joyYMin = 0, .joyYMax = 4095, .joyYCenter = 2048,
            .joyDeadzone = 0.1f,
            .activeLow   = true,
        },

        {   // ------------- CONTROLLER 3 --------------
            .enabled     = false,
                           // A    B    C    D    E    F    G    H    I    J
            .gpioPins    = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
            .expander    = { ExpanderType::None, -1, {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} },
            .joyXEnabled = false,
            .joyYEnabled = false,
            .joyXChannel = ADC_CHANNEL_0,
            .joyYChannel = ADC_CHANNEL_0,
            .joyXMin = 0, .joyXMax = 4095, .joyXCenter = 2048,
            .joyYMin = 0, .joyYMax = 4095, .joyYCenter = 2048,
            .joyDeadzone = 0.1f,
            .activeLow   = true,
        },

        {   // ------------- CONTROLLER 4 --------------
            .enabled     = false,
                           // A    B    C    D    E    F    G    H    I    J
            .gpioPins    = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
            .expander    = { ExpanderType::None, -1, {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} },
            .joyXEnabled = false,
            .joyYEnabled = false,
            .joyXChannel = ADC_CHANNEL_0,
            .joyYChannel = ADC_CHANNEL_0,
            .joyXMin = 0, .joyXMax = 4095, .joyXCenter = 2048,
            .joyYMin = 0, .joyYMax = 4095, .joyYCenter = 2048,
            .joyDeadzone = 0.1f,
            .activeLow   = true,
        },

    }

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
// Define exactly one display target — do not define more than one simultaneously.
// The default (DISPLAY_ST7735) can be overridden at build time by passing a
// compiler flag (e.g. -DDISPLAY_SDL), which is how the desktop CMake build
// selects its display driver.
#ifndef DISPLAY_SDL
    #define DISPLAY_ST7735
    //#define DISPLAY_OTHER
#endif


constexpr RenderSettings RENDER_SETTINGS = {
    // Background color shown anywhere no sprite covers the screen.
    // RGB565 format (16-bit): RRRRRGGGGGGBBBBB , Common values: 0x0000 Black, 0xFFFF White, 0xF800 Red, 0x001F Blue, 0x07E0 Green
    .defaultBackgroundPixel = 0x0000,

    // Rectangular area of the screen used for game rendering. { x1, y1, x2, y2 }
    // x1, y1 — top-left corner (inclusive), x2, y2 — bottom-right corner (inclusive)
    .gameRegion = { 0, 0, 128, 108 },

    // Set to false to disable sprite rendering to set framebuffer pixels directly.
    .spriteSystemEnabled = true,

    // Set to false to disable automatic clearing of the framebuffer each frame.
    .cleanFramebufferEachFrame = true

};

#pragma endregion



