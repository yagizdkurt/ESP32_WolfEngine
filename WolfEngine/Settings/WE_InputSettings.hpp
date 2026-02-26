#pragma once
#include "esp_adc/adc_oneshot.h"

// =================== INPUT SETTINGS STRUCT =======================
// This file contains struct definition for InputSettings, 
// which holds all the configuration parameters for the engine's input system.
// The actual instance of InputSettings with user-defined values is in WE_Settings.hpp. 
// This separation allows the struct definition to be included in multiple places without 
// causing redefinition errors, while keeping the user-configurable settings centralized in one header file.

struct InputSettings {

    // ---------------------------------------------------------
    //  Total button count — adjust if you add more buttons beyond A–K.
    // ---------------------------------------------------------
    static constexpr int BUTTON_COUNT = 7;

    // ---------------------------------------------------------
    //  Direct ESP32 GPIO pin for each button A–K in order.
    //  Set to -1 if the button is unused or routed through
    //  the PCF8574 expander instead.
    //
    //  Order: { A, B, C, D, E, F, K }
    //
    //  Recommended GPIO pins (input capable, no major caveats):
    //  4, 5, 13, 14, 18, 19, 21, 22, 23, 25, 26, 27
    // ---------------------------------------------------------
    int gpioPins[BUTTON_COUNT];

    // -------------------------------------------------------------
    //  INPUT_PCF8574_ADDR
    //  7-bit I2C address of the PCF8574 I/O expander.
    //  Set to -1 to disable expander support entirely.
    //  The address is determined by the A0/A1/A2 pins on the chip:
    //
    //   A2  A1  A0  │  Address
    //   ────────────┼──────────
    //    0   0   0  │  0x20   (default, all pins tied low)
    //    0   0   1  │  0x21
    //    0   1   0  │  0x22
    //    0   1   1  │  0x23
    //    1   0   0  │  0x24
    //    1   0   1  │  0x25
    //    1   1   0  │  0x26
    //    1   1   1  │  0x27
    // -------------------------------------------------------------
    int pcf8574Addr;

    // ---------------------------------------------------------
    //  PCF8574 pin (P0–P7) for each button A–K in order.
    //  Set to -1 if the button is unused or on a direct GPIO pin.
    //  GPIO takes priority — if both gpioPins and expanderPins
    //  are set for the same button, GPIO wins.
    //
    //  Order: { A, B, C, D, E, F, K }
    // ---------------------------------------------------------
    int expanderPins[BUTTON_COUNT];

    // Set to False if they are disabled.
    bool joyXEnabled;

    // -------------------------------------------------------------
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
    //
    //  Set to ADC_CHANNEL_MAX to disable joystick support entirely.
    // -------------------------------------------------------------
    adc_channel_t joyXChannel;

    // ---------------------------------------------------------
    //  Raw ADC calibration values for the joystick axes.
    //  Calibrate by printing raw ADC values while moving the
    //  stick to each extreme and leaving it untouched at rest.
    //  Range is 0–4095 for 12-bit ADC.
    // ---------------------------------------------------------
    int joyXMin, joyXMax, joyXCenter;

    // Same with Y Axis <--->

    bool joyYEnabled;
    adc_channel_t joyYChannel;
    int joyYMin, joyYMax, joyYCenter;

    // ---------------------------------------------------------
    //  Dead zone as a normalized fraction (0.0–1.0) applied
    //  around the centre point. Inputs within this fraction
    //  are returned as exactly 0. Increase if the stick drifts
    //  when untouched.
    // ---------------------------------------------------------
    float joyDeadzone;

    // ---------------------------------------------------------
    //  true  — buttons connect the pin to GND when pressed (most common).
    //          GPIO pins use ESP32 internal pull-up resistor.
    //          PCF8574 pins are written HIGH for quasi-bidirectional input.
    //  false — buttons connect the pin to VCC when pressed.
    //          Requires an external pull-down resistor on each pin.
    // ---------------------------------------------------------
    bool activeLow;

    // ---------------------------------------------------------
    //  debounceMs
    //  Software debounce window in milliseconds. A state change
    //  is only committed once the new level holds stable for this
    //  long. Eliminates mechanical contact bounce.
    //  Typical range: 10–50 ms.
    //  Increase if phantom presses appear.
    //  Decrease if fast inputs feel missed.
    // ---------------------------------------------------------
    int debounceMs;

    // ---------------------------------------------------------
    //  pollIntervalMs
    //  How often (in milliseconds) the input system polls all
    //  button states. 10 ms = 100 Hz, more than sufficient
    //  for a game controller.
    // ---------------------------------------------------------
    int pollIntervalMs;
};
