#pragma once
#include <stdint.h>
#include "esp_adc/adc_oneshot.h"
#include "WolfEngine/Drivers/IODrivers/WE_ExpanderDrivers.hpp"
// kButtonCount is defined in WE_ExpanderDrivers.hpp — available here via the include above.

// =============================================================
//  Render layers
// =============================================================
enum class RenderLayer : uint8_t {
    Default    = 2,
    DEFAULT    = 2,
    BackGround = 0,
    World      = 1,
    Entities   = 2,
    Player     = 3,
    FX         = 4,
    MAX_LAYERS
};

// =============================================================
//  Collision layers (bitmask)
// =============================================================
enum class CollisionLayer : uint16_t {
    DEFAULT    = 1 << 0,
    Player     = 1 << 1,
    Enemy      = 1 << 2,
    Wall       = 1 << 3,
    Trigger    = 1 << 4,
    Projectile = 1 << 5
};

// =============================================================
//  Region — rectangular screen area with helpers
// =============================================================
struct Region {
    int x1, y1, x2, y2;
    constexpr int width()   const { return x2 - x1; }
    constexpr int height()  const { return y2 - y1; }
    constexpr int centerX() const { return (x1 + x2) / 2; }
    constexpr int centerY() const { return (y1 + y2) / 2; }
};

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
    Region        gameRegion;
    uint16_t      maxDrawCommands;
    uint16_t      defaultBackgroundPixel;
    bool          spriteSystemEnabled;
    bool          cleanFramebufferEachFrame;
    uint32_t      targetFrameTimeUs;
    DisplayTarget displayTarget;
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
};

struct DebugConfig {
    // reserved for future log-level / category flags
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
};
