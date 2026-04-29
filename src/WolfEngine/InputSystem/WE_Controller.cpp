#include "WolfEngine/InputSystem/WE_Controller.hpp"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "WolfEngine/Utilities/Debug/WE_Debug.hpp"
#include <new>

// ─────────────────────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────────────────────

void Controller::init(const ControllerSettings& settings, adc_oneshot_unit_handle_t adcHandle, int64_t now) {
    m_settings  = &settings;
    // Shared ADC handle owned by InputManager — valid for the lifetime of the engine.
    m_adcHandle = adcHandle;

    // ── GPIO setup ────────────────────────────────────────────
    uint64_t pinMask = 0;
    for (int i = 0; i < Settings.input.buttonCount; i++) {
        if (m_settings->gpioPins[i] != -1)
            pinMask |= (1ULL << m_settings->gpioPins[i]);
    }

    if (pinMask != 0) {
        gpio_config_t cfg = {};
        cfg.mode          = GPIO_MODE_INPUT;
        cfg.pull_up_en    = m_settings->activeLow ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE;
        cfg.pull_down_en  = m_settings->activeLow ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE;
        cfg.intr_type     = GPIO_INTR_DISABLE;
        cfg.pin_bit_mask  = pinMask;
        gpio_config(&cfg);
    }

    // NOTE: IntelliSense may incorrectly report "no instance of overloaded operator new"
    // for placement new syntax. This is a known false positive with ESP-IDF + VS Code.
    // The code compiles correctly with 'pio run' as long as <new> is included.
    // ── Expander setup ────────────────────────────────────────
    if (m_settings->expander.type != ExpanderType::None) {
        void* buf = static_cast<void*>(m_expanderBuf);

        switch (m_settings->expander.type) {
            case ExpanderType::PCF8574:
                m_expander = new(buf) PCF8574(static_cast<uint8_t>(m_settings->expander.addr));
                break;
            case ExpanderType::PCF8575:
                m_expander = new(buf) PCF8575(static_cast<uint8_t>(m_settings->expander.addr));
                break;
            case ExpanderType::MCP23017:
                m_expander = new(buf) MCP23017(static_cast<uint8_t>(m_settings->expander.addr));
                break;
            default:
                break;
        }

        if (m_expander) {
            esp_err_t expanderErr = m_expander->begin();
            if (expanderErr != ESP_OK)
                WE_LOGE("Controller", "Expander begin() failed (addr=0x%02X): %s",
                        (uint8_t)m_settings->expander.addr, esp_err_to_name(expanderErr));
            else
                WE_LOGI("Controller", "Expander ready at 0x%02X", (uint8_t)m_settings->expander.addr);
        }
    }

    // ── Debounce timestamps ───────────────────────────────────
    for (int i = 0; i < Settings.input.buttonCount; i++)
        m_debounceTimestamp[i] = now;
}

// ─────────────────────────────────────────────────────────────
//  tick
// ─────────────────────────────────────────────────────────────

void Controller::tick(int64_t now, int64_t debounceUs) {

    // ── Button debounce ───────────────────────────────────────
    for (int i = 0; i < Settings.input.buttonCount; i++) {
        bool raw = readRaw(static_cast<Button>(i));

        if (raw != m_rawState[i]) {
            m_rawState[i]          = raw;
            m_debounceTimestamp[i] = now;
        } else if ((now - m_debounceTimestamp[i]) >= debounceUs) {
            m_prevState[i] = m_currState[i];
            m_currState[i] = raw;
        }
    }

    // ── Joystick ──────────────────────────────────────────────
    if (m_settings->joyXEnabled && m_adcHandle) {
        int raw = 0;
        adc_oneshot_read(m_adcHandle, m_settings->joyXChannel, &raw);
        m_axisX = normalizeAxis(raw, m_settings->joyXCenter, m_settings->joyXMin, m_settings->joyXMax);
    }

    if (m_settings->joyYEnabled && m_adcHandle) {
        int raw = 0;
        adc_oneshot_read(m_adcHandle, m_settings->joyYChannel, &raw);
        m_axisY = normalizeAxis(raw, m_settings->joyYCenter, m_settings->joyYMin, m_settings->joyYMax);
    }
}

// ─────────────────────────────────────────────────────────────
//  readRaw
// ─────────────────────────────────────────────────────────────

bool Controller::readRaw(Button btn) const {
    int idx = static_cast<int>(btn);

    // GPIO takes priority over expander
    if (m_settings->gpioPins[idx] != -1) {
        int level = gpio_get_level(static_cast<gpio_num_t>(m_settings->gpioPins[idx]));
        return m_settings->activeLow ? level == 0 : level == 1;
    }

    // Expander
    if (m_expander && m_settings->expander.pins[idx] != -1) {
        int level = m_expander->pinRead(static_cast<uint8_t>(m_settings->expander.pins[idx]));
        if (level < 0) return false;
        return m_settings->activeLow ? level == 0 : level == 1;
    }

    return false;
}

// ─────────────────────────────────────────────────────────────
//  normalizeAxis
// ─────────────────────────────────────────────────────────────

float Controller::normalizeAxis(int raw, int centre, int minVal, int maxVal) const {
    float normalized;

    if (raw >= centre) {
        int range = maxVal - centre;
        normalized = (range > 0) ? static_cast<float>(raw - centre) / range : 0.0f;
    } else {
        int range = centre - minVal;
        normalized = (range > 0) ? static_cast<float>(raw - centre) / range : 0.0f;
    }

    if (normalized > -m_settings->joyDeadzone && normalized < m_settings->joyDeadzone)
        return 0.0f;

    if (normalized >  1.0f) return  1.0f;
    if (normalized < -1.0f) return -1.0f;

    return normalized;
}

// ─────────────────────────────────────────────────────────────
//  Public query methods
// ─────────────────────────────────────────────────────────────

bool Controller::getButton(Button btn) const {
    return m_currState[static_cast<int>(btn)];
}

bool Controller::getButtonDown(Button btn) const {
    int i = static_cast<int>(btn);
    return m_currState[i] && !m_prevState[i];
}

bool Controller::getButtonUp(Button btn) const {
    int i = static_cast<int>(btn);
    return !m_currState[i] && m_prevState[i];
}

float Controller::getAxis(JoyAxis axis) const {
    return (axis == JoyAxis::X) ? m_axisX : m_axisY;
}

void Controller::simulateButton(Button btn, bool pressed) {
    int i = static_cast<int>(btn);
    m_prevState[i] = m_currState[i];
    m_currState[i] = pressed;
}

void Controller::simulateJoystick(JoyAxis axis, float value) {
    if (value >  1.0f) value =  1.0f;
    if (value < -1.0f) value = -1.0f;
    if (axis == JoyAxis::X) m_axisX = value;
    else                    m_axisY = value;
}
