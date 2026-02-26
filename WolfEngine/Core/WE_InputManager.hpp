#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Utilities/WE_PCF8574.hpp"
#include <stdint.h>

class WolfEngine;

enum class Button { A, B, C, D, E, F, K };
enum class JoyAxis { X, Y };

class InputManager {
public:

    // Returns true every frame the button is held down.
    bool getButton(Button btn) const;

    // Returns true only on the frame the button transitions low→high.
    bool getButtonDown(Button btn) const;

    // Returns true only on the frame the button transitions high→low.
    bool getButtonUp(Button btn) const;

    // Returns the normalized joystick position on the requested axis.
    // Range: -1.0 (min) to +1.0 (max). Returns 0.0 inside the dead zone defined by INPUT_SETTINGS.joyDeadzone
    float getAxis(JoyAxis axis) const;

private:
    // ── State ──────────────────────────────────────────────────
    adc_oneshot_unit_handle_t m_adcHandle = nullptr;
    bool    m_currState[InputSettings::BUTTON_COUNT]         = {};
    bool    m_prevState[InputSettings::BUTTON_COUNT]         = {};
    bool    m_rawState[InputSettings::BUTTON_COUNT]          = {};
    int64_t m_debounceTimestamp[InputSettings::BUTTON_COUNT] = {};

    float m_axisX = 0.0f;
    float m_axisY = 0.0f;

    PCF8574 m_expander;

    bool  readRaw(Button btn) const;
    float normalizeAxis(int raw, int centre, int minVal, int maxVal) const;

    friend class WolfEngine;
    InputManager() { init(); }
    void init();
    void tick();
};