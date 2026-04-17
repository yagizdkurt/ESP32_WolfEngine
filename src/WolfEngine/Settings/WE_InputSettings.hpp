#pragma once
#include "esp_adc/adc_oneshot.h"
#include "WolfEngine/Drivers/IODrivers/WE_ExpanderDrivers.hpp"

// =============================================================
//  WE_InputSettings.hpp
//  Struct definitions for the engine's input system.
//  The actual user-configured instances live in WE_Settings.hpp.
//  This separation allows the structs to be included in multiple
//  places without redefinition errors.
// =============================================================

// =============================================================
//  ControllerSettings
//  Configuration for a single physical controller.
//  One entry per player in the CONTROLLERS array.
// =============================================================
struct ControllerSettings {

    // ---------------------------------------------------------
    //  enabled
    //  Set to false to disable this controller slot entirely.
    //  Disabled controllers are excluded from polling and
    //  cannot be accessed via Input().getController<N>().
    // ---------------------------------------------------------
    bool enabled;

    // ---------------------------------------------------------
    //  gpioPins[10]
    //  Direct ESP32 GPIO pin for each button A–J in order.
    //  Set to -1 if the button is unused or routed through
    //  an expander instead.
    //
    //  Order: { A, B, C, D, E, F, G, H, I, J }
    //
    //  Recommended GPIO pins (input capable, no major caveats):
    //  4, 5, 13, 14, 18, 19, 21, 22, 23, 25, 26, 27
    // ---------------------------------------------------------
    int gpioPins[10];

    // ---------------------------------------------------------
    //  expander
    //  I/O expander configuration for this controller.
    //  Set expander.type to ExpanderType::None if all buttons
    //  are wired directly to GPIO.
    //
    //  Supported expanders:
    //  ExpanderType::PCF8574  — 8-bit,  address 0x20–0x27
    //  ExpanderType::PCF8575  — 16-bit, address 0x20–0x27
    //  ExpanderType::MCP23017 — 16-bit, address 0x20–0x27
    // ---------------------------------------------------------
    ExpanderSettings expander;

    // ---------------------------------------------------------
    //  joyXEnabled / joyYEnabled
    //  Set to false to disable the joystick axis entirely.
    //  When disabled, getAxis() always returns 0.0 for that axis.
    // ---------------------------------------------------------
    bool joyXEnabled;
    bool joyYEnabled;

    // ---------------------------------------------------------
    //  joyXChannel / joyYChannel
    //  ADC1 channel for the joystick X and Y axes.
    //  The GPIO pin number and ADC channel are NOT the same on ESP32.
    //  Use the table below to find your channel:
    //
    //   GPIO  │  ADC Channel
    //   ──────┼───────────────
    //    36   │  ADC_CHANNEL_0
    //    37   │  ADC_CHANNEL_1
    //    38   │  ADC_CHANNEL_2
    //    39   │  ADC_CHANNEL_3
    //    32   │  ADC_CHANNEL_4
    //    33   │  ADC_CHANNEL_5
    //    34   │  ADC_CHANNEL_6
    //    35   │  ADC_CHANNEL_7
    // ---------------------------------------------------------
    adc_channel_t joyXChannel;
    adc_channel_t joyYChannel;

    // ---------------------------------------------------------
    //  Joystick calibration values.
    //  Calibrate by printing raw ADC values while moving the
    //  stick to each extreme and leaving it untouched at rest.
    //  Range is 0–4095 for 12-bit ADC.
    // ---------------------------------------------------------
    int joyXMin, joyXMax, joyXCenter;
    int joyYMin, joyYMax, joyYCenter;

    // ---------------------------------------------------------
    //  joyDeadzone
    //  Dead zone as a normalized fraction (0.0–1.0) applied
    //  around the centre point. Inputs within this fraction
    //  are returned as exactly 0. Increase if the stick drifts
    //  when untouched.
    // ---------------------------------------------------------
    float joyDeadzone;

    // ---------------------------------------------------------
    //  activeLow
    //  true  — buttons connect the pin to GND when pressed (most common).
    //          GPIO pins use ESP32 internal pull-up resistor.
    //          Expander pins are driven HIGH for quasi-bidirectional input.
    //  false — buttons connect the pin to VCC when pressed.
    //          Requires an external pull-down resistor on each pin.
    // ---------------------------------------------------------
    bool activeLow;
};


// =============================================================
//  InputSettings
//  Engine-wide input configuration.
//  Contains global timing settings and all controller slots.
// =============================================================
struct InputSettings {

    // ---------------------------------------------------------
    //  debounceMs
    //  Software debounce window in milliseconds applied to all
    //  controllers. A state change is only committed once the
    //  new level holds stable for this long.
    //  Typical range: 10–50 ms.
    //  Increase if phantom presses appear.
    //  Decrease if fast inputs feel missed.
    // ---------------------------------------------------------
    int debounceMs;

    // ---------------------------------------------------------
    //  pollIntervalMs
    //  How often (in milliseconds) the input system polls all
    //  controllers. 10 ms = 100 Hz, more than sufficient
    //  for a game controller.
    // ---------------------------------------------------------
    int pollIntervalMs;

    // ---------------------------------------------------------
    //  controllers[4]
    //  Configuration for up to 4 controllers (players 1–4).
    //  Set enabled = false for unused controller slots.
    // ---------------------------------------------------------
    ControllerSettings controllers[4];
};

static constexpr int BUTTON_COUNT = 10; // Dont change this.