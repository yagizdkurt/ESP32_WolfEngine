#pragma once
#include "WE_ConfigTypes.hpp"

// ── Display target selector ─────────────────────────────────────────────────
// DISPLAY_SDL is injected by the desktop CMake build (-DDISPLAY_SDL).
// The macro must remain defined here so WE_RenderCore.hpp can use it in
// #if defined(DISPLAY_ST7735) for conditional driver #includes — that usage
// cannot be replaced by an enum.
#ifndef DISPLAY_SDL
    #define DISPLAY_ST7735
    #define RENDER_SCREEN_WIDTH  128
    #define RENDER_SCREEN_HEIGHT 160
#endif

// ── Module Enables ───────────────────────────────────────────────────────────
// Comment out a line to disable the corresponding engine module.
// These macros gate both #include directives and static variable declarations
// in WE_ModuleSystem.cpp — if constexpr cannot replace them.
#define WE_MODULE_SAVELOAD
#define WE_MODULE_COLLISION

/*
============================================================================================
 WOLF ENGINE SETTINGS
 User-facing configuration. Edit the values inside the Settings initializer below.
 All engine systems read from Settings.domain.field — never from the old macros.

 To add a new setting:
   1. Add a field to the appropriate struct in WE_ConfigTypes.hpp.
   2. Set the value in the Settings initializer below.
   3. Add a static_assert in the validation block if the value has a valid range.
============================================================================================
*/

inline constexpr EngineConfig Settings = {

    // ── Hardware ─────────────────────────────────────────────────────────────
    .hardware = {
        .i2c     = { .sda = 21, .scl = 22 },
        .spi     = { .mosi = 23, .miso = 19, .sclk = 18 },
        .display = { .cs = 17, .rst = 4, .dc = 16 },
        .sound   = { .music = 14, .sfx = 32 },
    },

    // ── Renderer ─────────────────────────────────────────────────────────────
    .render = {
        .screenWidth             = RENDER_SCREEN_WIDTH,
        .screenHeight            = RENDER_SCREEN_HEIGHT,
        // Rectangular game render area in half-open bounds: [x1, x2), [y1, y2).
        // This is compile-time validated against framebuffer dimensions in WE_RenderCore.hpp.
        .gameRegion              = { 0, 0, RENDER_SCREEN_WIDTH - 0, RENDER_SCREEN_HEIGHT - 0 },
        // Maximum DrawCommands that can be submitted per frame. Tune based on peak sprite count.
        .maxDrawCommands         = 128,
        // Background color in RGB565 format. 0x0000 = Black, 0xFFFF = White.
        .defaultBackgroundPixel  = 0x0000,
        // Set to false to disable sprite rendering.
        .spriteSystemEnabled     = true,
        // Set to false to disable automatic framebuffer clear each frame.
        .cleanFramebufferEachFrame = true,
        // Target frame time in microseconds (1,000,000 / 30 = 33,333 us).
        .targetFrameTimeUs       = 33333,
        // Display target — set automatically from the DISPLAY_SDL / DISPLAY_ST7735 macro above.
#ifdef DISPLAY_SDL
        .displayTarget           = DisplayTarget::SDL,
#else
        .displayTarget           = DisplayTarget::ST7735,
#endif
    },

    // ── Input ────────────────────────────────────────────────────────────────
    .input = {
        // Software debounce window in milliseconds.
        .debounceMs     = 20,
        // How often (in ms) all controllers are polled.
        .pollIntervalMs = 10,
        // Number of buttons per controller. Must equal kButtonCount.
        .buttonCount    = kButtonCount,

        .controllers    = {

            {   // ── Controller 1 ──────────────────────────────────────────
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

            {   // ── Controller 2 ──────────────────────────────────────────
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

            {   // ── Controller 3 ──────────────────────────────────────────
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

            {   // ── Controller 4 ──────────────────────────────────────────
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
        },
    },

    // ── Limits ───────────────────────────────────────────────────────────────
    .limits = {
        .maxGameObjects   = 64,
        // Maximum top-level UI elements accepted by UI().setElements(...).
        .maxUIElements    = 32,
        // Maximum children per UIPanel.
        .maxPanelChildren = 10,
        .maxPaletteInexes = 32,
    },

    // ── Debug ────────────────────────────────────────────────────────────────
    .debug = {},
};

// ── Validation ───────────────────────────────────────────────────────────────
static_assert(Settings.limits.maxGameObjects > 0 && Settings.limits.maxGameObjects <= 65535,
    "maxGameObjects must be between 1 and 65535 — live counter in WE_GORegistry is uint16_t");
static_assert(Settings.limits.maxUIElements > 0 && Settings.limits.maxUIElements <= 255,
    "maxUIElements must be between 1 and 255 — UIManager count is uint8_t");
static_assert(Settings.limits.maxPanelChildren > 0 && Settings.limits.maxPanelChildren <= 255,
    "maxPanelChildren must be between 1 and 255 — iterator in UIPanel is uint8_t");
static_assert(Settings.render.maxDrawCommands  > 0,  "maxDrawCommands must be > 0");
static_assert(Settings.render.targetFrameTimeUs > 0, "targetFrameTimeUs must be > 0");
static_assert(Settings.input.buttonCount == kButtonCount,
    "buttonCount must match kButtonCount (controls array sizes in ControllerSettings/ExpanderSettings)");
