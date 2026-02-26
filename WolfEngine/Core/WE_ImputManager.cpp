#include "WolfEngine/Core/WE_InputManager.hpp"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

// ─────────────────────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────────────────────

void InputManager::init() {

    uint64_t pinMask = 0;
    for (int i = 0; i < InputSettings::BUTTON_COUNT; i++) {
        if (INPUT_SETTINGS.gpioPins[i] != -1)
            pinMask |= (1ULL << INPUT_SETTINGS.gpioPins[i]);
    }

    if (pinMask != 0) {
        gpio_config_t cfg = {};
        cfg.mode          = GPIO_MODE_INPUT;
        cfg.pull_up_en    = INPUT_SETTINGS.activeLow ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE;
        cfg.pull_down_en  = INPUT_SETTINGS.activeLow ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE;
        cfg.intr_type     = GPIO_INTR_DISABLE;
        cfg.pin_bit_mask  = pinMask;
        gpio_config(&cfg);
    }

    if constexpr (INPUT_SETTINGS.pcf8574Addr != -1) {
        m_expander = PCF8574(INPUT_SETTINGS.pcf8574Addr);
        m_expander.write(0xFF);
    }

    if (INPUT_SETTINGS.joyXEnabled || INPUT_SETTINGS.joyYEnabled) {
        adc_oneshot_unit_init_cfg_t unitCfg = {};
        unitCfg.unit_id = ADC_UNIT_1;
        adc_oneshot_new_unit(&unitCfg, &m_adcHandle);

        adc_oneshot_chan_cfg_t chanCfg = {};
        chanCfg.bitwidth = ADC_BITWIDTH_12;
        chanCfg.atten    = ADC_ATTEN_DB_12;

        if (INPUT_SETTINGS.joyXEnabled)
            adc_oneshot_config_channel(m_adcHandle, INPUT_SETTINGS.joyXChannel, &chanCfg);

        if (INPUT_SETTINGS.joyYEnabled)
            adc_oneshot_config_channel(m_adcHandle, INPUT_SETTINGS.joyYChannel, &chanCfg);
    }

    int64_t now = esp_timer_get_time();
    for (int i = 0; i < InputSettings::BUTTON_COUNT; i++)
        m_debounceTimestamp[i] = now;
}

// ─────────────────────────────────────────────────────────────
//  tick
// ─────────────────────────────────────────────────────────────

void InputManager::tick() {
    int64_t now = esp_timer_get_time();
    const int64_t debounceUs = static_cast<int64_t>(INPUT_SETTINGS.debounceMs) * 1000LL;

    for (int i = 0; i < InputSettings::BUTTON_COUNT; i++) {
        bool raw = readRaw(static_cast<Button>(i));

        if (raw != m_rawState[i]) {
            m_rawState[i]          = raw;
            m_debounceTimestamp[i] = now;
        } else if ((now - m_debounceTimestamp[i]) >= debounceUs) {
            m_prevState[i] = m_currState[i];
            m_currState[i] = raw;
        }
    }

    if (INPUT_SETTINGS.joyXEnabled) {
        int raw = 0;
        adc_oneshot_read(m_adcHandle, INPUT_SETTINGS.joyXChannel, &raw);
        m_axisX = normalizeAxis(raw, INPUT_SETTINGS.joyXCenter, INPUT_SETTINGS.joyXMin, INPUT_SETTINGS.joyXMax);
    }

    if (INPUT_SETTINGS.joyYEnabled) {
        int raw = 0;
        adc_oneshot_read(m_adcHandle, INPUT_SETTINGS.joyYChannel, &raw);
        m_axisY = normalizeAxis(raw, INPUT_SETTINGS.joyYCenter, INPUT_SETTINGS.joyYMin, INPUT_SETTINGS.joyYMax);
    }
}

// ─────────────────────────────────────────────────────────────
//  readRaw
// ─────────────────────────────────────────────────────────────

bool InputManager::readRaw(Button btn) const {
    int idx = static_cast<int>(btn);

    if (INPUT_SETTINGS.gpioPins[idx] != -1) {
        int level = gpio_get_level(static_cast<gpio_num_t>(INPUT_SETTINGS.gpioPins[idx]));
        return INPUT_SETTINGS.activeLow ? level == 0 : level == 1;
    }

    if constexpr (INPUT_SETTINGS.pcf8574Addr != -1) {
        if (INPUT_SETTINGS.expanderPins[idx] != -1) {
            int level = m_expander.pinRead(static_cast<uint8_t>(INPUT_SETTINGS.expanderPins[idx]));
            if (level < 0) return false;
            return INPUT_SETTINGS.activeLow ? level == 0 : level == 1;
        }
    }

    return false;
}

// ─────────────────────────────────────────────────────────────
//  normalizeAxis
// ─────────────────────────────────────────────────────────────

float InputManager::normalizeAxis(int raw, int centre, int minVal, int maxVal) const {
    float normalized;

    if (raw >= centre) {
        int range = maxVal - centre;
        normalized = (range > 0) ? static_cast<float>(raw - centre) / range : 0.0f;
    } else {
        int range = centre - minVal;
        normalized = (range > 0) ? static_cast<float>(raw - centre) / range : 0.0f;
    }

    if (normalized > -INPUT_SETTINGS.joyDeadzone && normalized < INPUT_SETTINGS.joyDeadzone)
        return 0.0f;

    if (normalized >  1.0f) return  1.0f;
    if (normalized < -1.0f) return -1.0f;

    return normalized;
}

// ─────────────────────────────────────────────────────────────
//  Public query methods
// ─────────────────────────────────────────────────────────────

bool InputManager::getButton(Button btn) const {
    return m_currState[static_cast<int>(btn)];
}

bool InputManager::getButtonDown(Button btn) const {
    int i = static_cast<int>(btn);
    return m_currState[i] && !m_prevState[i];
}

bool InputManager::getButtonUp(Button btn) const {
    int i = static_cast<int>(btn);
    return !m_currState[i] && m_prevState[i];
}

float InputManager::getAxis(JoyAxis axis) const {
    return (axis == JoyAxis::X) ? m_axisX : m_axisY;
}

