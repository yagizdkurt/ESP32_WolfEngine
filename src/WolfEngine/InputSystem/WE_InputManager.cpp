#include "WolfEngine/InputSystem/WE_InputManager.hpp"
#include "WolfEngine/InputSystem/WE_IInputProvider.hpp"
#include "WolfEngine/Utilities/WE_Debug.hpp"
#include "esp_timer.h"
#include "driver/gpio.h"

void InputManager::setInputProvider(IInputProvider* provider) { m_inputProvider = provider; }
void InputManager::setAlwaysEnableController0(bool value) { m_alwaysEnableController0 = value; }

// setInputProvider and setAlwaysEnableController0 are intentionally separate.
// A future provider (e.g. replay system) may not require controller 0 to bypass
// the enabled check. The two concerns must remain independently controllable.
Controller* InputManager::getController(int index) {
    WE_ASSERT(index >= 0 && index < MAX_CONTROLLERS, "getController: index out of range");
    if (m_alwaysEnableController0 && index == 0) return &m_controllers[0];
    if (!Settings.input.controllers[index].enabled) return nullptr;
    return &m_controllers[index];
}

void InputManager::init() {

    // ── GPIO setup for all enabled controllers ────────────────
    uint64_t pinMask = 0;
    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!Settings.input.controllers[c].enabled) continue;
        for (int i = 0; i < Settings.input.buttonCount; i++) {
            if (Settings.input.controllers[c].gpioPins[i] != -1)
                pinMask |= (1ULL << Settings.input.controllers[c].gpioPins[i]);
        }
    }

    if (pinMask != 0) {
        // Use activeLow from first enabled controller — all controllers
        // should use the same electrical configuration on one device.
        bool activeLow = Settings.input.controllers[0].activeLow;
        for (int c = 0; c < MAX_CONTROLLERS; c++) {
            if (Settings.input.controllers[c].enabled) {
                activeLow = Settings.input.controllers[c].activeLow;
                break;
            }
        }

        gpio_config_t cfg = {};
        cfg.mode          = GPIO_MODE_INPUT;
        cfg.pull_up_en    = activeLow ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE;
        cfg.pull_down_en  = activeLow ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE;
        cfg.intr_type     = GPIO_INTR_DISABLE;
        cfg.pin_bit_mask  = pinMask;
        gpio_config(&cfg);
    }

    // ── ADC setup ─────────────────────────────────────────────
    bool needsAdc = false;
    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!Settings.input.controllers[c].enabled) continue;
        if (Settings.input.controllers[c].joyXEnabled ||
            Settings.input.controllers[c].joyYEnabled) {
            needsAdc = true;
            break;
        }
    }

    if (needsAdc) {
        adc_oneshot_unit_init_cfg_t unitCfg = {};
        unitCfg.unit_id = ADC_UNIT_1;
        adc_oneshot_new_unit(&unitCfg, &m_adcHandle);

        adc_oneshot_chan_cfg_t chanCfg = {};
        chanCfg.bitwidth = ADC_BITWIDTH_12;
        chanCfg.atten    = ADC_ATTEN_DB_12;

        for (int c = 0; c < MAX_CONTROLLERS; c++) {
            if (!Settings.input.controllers[c].enabled) continue;
            if (Settings.input.controllers[c].joyXEnabled)
                adc_oneshot_config_channel(m_adcHandle, Settings.input.controllers[c].joyXChannel, &chanCfg);
            if (Settings.input.controllers[c].joyYEnabled)
                adc_oneshot_config_channel(m_adcHandle, Settings.input.controllers[c].joyYChannel, &chanCfg);
        }
    }

    // ── Init each enabled controller ──────────────────────────
    int64_t now = esp_timer_get_time();
    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!Settings.input.controllers[c].enabled) continue;
        m_controllers[c].init(Settings.input.controllers[c], m_adcHandle, now);
    }
}

void InputManager::tick() {
    if (m_inputProvider) {
        m_inputProvider->flush(m_controllers, MAX_CONTROLLERS);
        return;
    }
    int64_t now        = esp_timer_get_time();
    int64_t debounceUs = static_cast<int64_t>(Settings.input.debounceMs) * 1000LL;

    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!Settings.input.controllers[c].enabled) continue;
        m_controllers[c].tick(now, debounceUs);
    }
}
