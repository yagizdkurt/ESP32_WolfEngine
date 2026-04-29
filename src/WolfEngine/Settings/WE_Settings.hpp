#pragma once
#include "WE_ConfigTypes.hpp"
#include "WE_DisplaySelector.hpp"

// ── Module Enables ───────────────────────────────────────────────────────────
// Comment out a line to disable the corresponding engine module.
// These macros gate both #include directives and static variable declarations
// in WE_ModuleSystem.cpp — if constexpr cannot replace them.
// #define WE_MODULE_SAVELOAD
// #define WE_MODULE_COLLISION

// ── Dual-Core Rendering ──────────────────────────────────────────────────────
// When 1: Core 1 renders into the back buffer while Core 0 flushes the front
// buffer to the ST7735 via DMA, overlapping both operations each frame.
// When 0: single-core path — byte-for-byte identical to the baseline.
// Auto-forced to 0 on desktop (ESP_PLATFORM absent — no FreeRTOS, no DMA).
#ifdef ESP_PLATFORM
    #define WE_DUAL_CORE_RENDER 1   // <-- user-editable
#else
    #define WE_DUAL_CORE_RENDER 0   // forced off; do not edit
#endif

#if WE_DUAL_CORE_RENDER
    // Stack for the display task. flush() calls into esp_lcd_panel_draw_bitmap
    // which traverses the ESP-IDF LCD panel IO → SPI DMA layer — not a shallow chain.
    #define DISPLAY_TASK_STACK_SIZE  8192

    // Priority 5 puts the display task above typical user tasks (priority 1–3)
    // but below FreeRTOS system tasks. Since it blocks on semaphores most of the
    // time, it never burns CPU — it just needs to wake promptly when renderReady
    // is given.
    #define DISPLAY_TASK_PRIORITY    5

    // Pin to Core 0 (PRO_CPU). The main app_main() and game loop run on Core 1
    // (APP_CPU). WE_RADIO_ENABLED 0 means Core 0 carries no WiFi/BT load, so
    // it is effectively idle between display task wakeups. Pinning here gives
    // true hardware parallelism with Core 1's render pass.
    #define DISPLAY_TASK_CORE_ID     0
#endif

// ── Diagnostics ──────────────────────────────────────────────────────────────
// Master switch. When 0, timing helpers are no-ops and logs compile away.
// Automatically disabled when NDEBUG is defined (release builds).
#if defined(WE_DIAG_ENABLED)
    // already defined externally — respect it
#elif defined(NDEBUG)
    #define WE_DIAG_ENABLED 0
#else
    #define WE_DIAG_ENABLED 1
#endif

// Log diagnostics every N frames.
#define WE_DIAG_LOG_INTERVAL_FRAMES 60

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
        .screenWidth             = DISPLAY_TARGET.screenWidth,
        .screenHeight            = DISPLAY_TARGET.screenHeight,
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
        // Display target — set automatically from the DisplaySelector.hpp definitions.
        .displayTarget           = DISPLAY_TARGET.target,
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
static_assert(Settings.limits.maxGameObjects > 0,"maxGameObjects must be > 0");
static_assert(Settings.limits.maxUIElements > 0, "maxUIElements must be > 0");
static_assert(Settings.limits.maxPanelChildren > 0,"maxPanelChildren must be > 0");
static_assert(Settings.render.maxDrawCommands  > 0,  "maxDrawCommands must be > 0");
static_assert(Settings.render.targetFrameTimeUs > 0, "targetFrameTimeUs must be > 0");
static_assert(Settings.input.buttonCount == kButtonCount,
    "buttonCount must match kButtonCount (controls array sizes in ControllerSettings/ExpanderSettings)");
