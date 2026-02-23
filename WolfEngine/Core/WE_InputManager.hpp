#pragma once
#include "WolfEngine/Settings/WE_PINDEFS.hpp"
#include "WolfEngine/Settings/WE_InputSettings.hpp"
#include "driver/gpio.h"
#include "driver/adc.h"
#include <stdint.h>

// Pull in the PCF8574 driver only when the expander is configured.
#if INPUT_PCF8574_ADDR != -1
    #include "WolfEngine/Drivers/PCF8574.hpp"
#endif

class WolfEngine;

enum class Button { A, B, C, D, E, F, K };
enum class JoyAxis { X, Y }; // Analog joystick axes. Both always return a value in [-1.0, 1.0].

// ─────────────────────────────────────────────────────────────
//  InputManager
//
//  Singleton — access via the free function Input() defined at
//  the bottom of this file.
//
//  Each tick() (called by WolfEngine) the manager:
//    1. Reads raw button levels from GPIO and/or the PCF8574.
//    2. Runs per-button software debouncing.
//    3. Snapshots previous state so edge queries (Down/Up) work.
//    4. Reads joystick ADC values and normalizes them.
//
//  Buttons wired directly to GPIO and buttons on the PCF8574 can
//  coexist — routing is determined entirely by WE_InputSettings.hpp.
// ─────────────────────────────────────────────────────────────
class InputManager {
public:

    // Returns true every frame the button is held down.
    bool getButton(Button btn) const;

    // Returns true only on the frame the button transitions low→high.
    bool getButtonDown(Button btn) const;

    // Returns true only on the frame the button transitions high→low.
    bool getButtonUp(Button btn) const;

    // Returns the normalized joystick position on the requested axis.
    // Range: -1.0 (min) to +1.0 (max). Returns 0.0 inside the dead zone
    // defined by INPUT_JOY_DEADZONE in WE_InputSettings.hpp.
    float getAxis(JoyAxis axis) const;

private:
    static constexpr int BUTTON_COUNT = 7;

    // GPIO pin for each button index, sourced from settings. -1 means the button is not on a direct GPIO pin.
    static constexpr int GPIO_PINS[BUTTON_COUNT] = {
        INPUT_PIN_BUTTON_A,
        INPUT_PIN_BUTTON_B,
        INPUT_PIN_BUTTON_C,
        INPUT_PIN_BUTTON_D,
        INPUT_PIN_BUTTON_E,
        INPUT_PIN_BUTTON_F,
        INPUT_PIN_BUTTON_K,
    };

    // PCF8574 pin for each button index, sourced from settings. -1 means the button is not on the expander.
    static constexpr int EXP_PINS[BUTTON_COUNT] = {
        INPUT_PCF8574_PIN_BUTTON_A,
        INPUT_PCF8574_PIN_BUTTON_B,
        INPUT_PCF8574_PIN_BUTTON_C,
        INPUT_PCF8574_PIN_BUTTON_D,
        INPUT_PCF8574_PIN_BUTTON_E,
        INPUT_PCF8574_PIN_BUTTON_F,
        INPUT_PCF8574_PIN_BUTTON_K,
    };

    // ── State ──────────────────────────────────────────────────

    // Committed (debounced) button states for the current and previous frame.
    bool m_currState[BUTTON_COUNT] = {};
    bool m_prevState[BUTTON_COUNT] = {};

    // Raw (pre-debounce) level seen on the last poll — used to detect
    // when the signal has been stable for INPUT_DEBOUNCE_MS.
    bool m_rawState[BUTTON_COUNT] = {};

    // Timestamp (µs from esp_timer_get_time) of the last raw state change
    // per button. A committed state change only happens once the raw level
    // has held stable for INPUT_DEBOUNCE_MS without flipping again.
    int64_t m_debounceTimestamp[BUTTON_COUNT] = {};

    // Cached normalized joystick axes, updated every tick.
    float m_axisX = 0.0f;
    float m_axisY = 0.0f;

#if INPUT_PCF8574_ADDR != -1
    PCF8574 m_expander { INPUT_PCF8574_ADDR };
#endif


    // Reads the physical level of a single button from whichever source
    // it is wired to (GPIO or PCF8574). Returns the logical pressed state
    // after applying INPUT_BUTTON_ACTIVE_LOW. Returns false if the button
    // has no valid pin assignment on any source.
    bool readRaw(Button btn) const;

    // Normalizes a raw ADC value for one axis into [-1.0, 1.0] using the
    // centre, min, and max constants from WE_InputSettings.hpp, then applies
    // the dead zone. The centre is treated as the true zero point so that
    // asymmetric ADC ranges around centre are handled correctly.
    float normalizeAxis(int raw, int centre, int minVal, int maxVal) const;

    friend class WolfEngine;
    InputManager() { init(); }
    void init();
    void tick();
};