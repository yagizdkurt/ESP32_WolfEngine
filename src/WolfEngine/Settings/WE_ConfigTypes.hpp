#pragma once
#include <stdint.h>
#include "esp_adc/adc_oneshot.h"
#include "WolfEngine/Drivers/IODrivers/WE_ExpanderDrivers.hpp"
#include "WE_Layers.hpp"
// kButtonCount is defined in WE_ExpanderDrivers.hpp — available here via the include above.

// =============================================================
//  Display target selector
// =============================================================
enum class DisplayTarget : uint8_t { ST7735, SDL };

// =============================================================
//  ControllerSettings — configuration for one physical controller
// =============================================================
struct ControllerSettings {
    bool          enabled;
    int           gpioPins[kButtonCount];
    ExpanderSettings expander;
    bool          joyXEnabled;
    bool          joyYEnabled;
    adc_channel_t joyXChannel;
    adc_channel_t joyYChannel;
    int           joyXMin, joyXMax, joyXCenter;
    int           joyYMin, joyYMax, joyYCenter;
    float         joyDeadzone;
    bool          activeLow;
};

// =============================================================
//  Game flow states and masks
// =============================================================
enum class GameFlowState : uint8_t {Running, Dialogue, Menu, Cutscene, Custom1, Custom2, Custom3, Custom4 };

struct GameFlowMasksConfig {
    uint16_t running;
    uint16_t dialogue;
    uint16_t menu;
    uint16_t cutscene;
    uint16_t custom1;
    uint16_t custom2;
    uint16_t custom3;
    uint16_t custom4;
};

// =============================================================
//  Domain config structs
// =============================================================

struct I2CPinConfig     { int sda;  int scl; };
struct SPIPinConfig     { int mosi; int miso; int sclk; };
struct DisplayPinConfig { int cs;   int rst;  int dc;   };
struct SoundPinConfig   { int music; int sfx; };

struct HardwareConfig {
    I2CPinConfig     i2c;
    SPIPinConfig     spi;
    DisplayPinConfig display;
    SoundPinConfig   sound;
};

struct RenderConfig {
    uint16_t      screenWidth;
    uint16_t      screenHeight;
    uint16_t      maxDrawCommands;
    uint16_t      defaultBackgroundPixel;
    bool          spriteSystemEnabled;
    bool          cleanFramebufferEachFrame;
    uint32_t      targetFrameTimeUs;
    DisplayTarget displayTarget;
};

struct DisplayTargetConfig {
    uint16_t screenWidth;
    uint16_t screenHeight;
    DisplayTarget target;
};

struct InputConfig {
    int debounceMs;
    int pollIntervalMs;
    int buttonCount;
    ControllerSettings controllers[4];
};

struct LimitsConfig {
    uint8_t maxGameObjects;
    uint8_t maxUIElements;
    uint8_t maxPanelChildren;
    uint8_t maxPaletteInexes;
    uint8_t maxFlowStackDepth;
};

struct DebugConfig {
    // reserved for future log-level / category flags
};

struct GameFlowConfig {
    GameFlowMasksConfig masks;
};

// =============================================================
//  Top-level aggregate
//  NOTE: No SaveConfig field. WE_SaveSettings.hpp is a module-level
//  file explicitly out of scope for this migration.
// =============================================================
struct EngineConfig {
    HardwareConfig hardware;
    RenderConfig   render;
    InputConfig    input;
    LimitsConfig   limits;
    DebugConfig    debug;
    GameFlowConfig gameFlow;
};
