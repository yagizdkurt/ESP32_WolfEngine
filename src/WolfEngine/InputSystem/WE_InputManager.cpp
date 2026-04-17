#include "WolfEngine/InputSystem/WE_InputManager.hpp"
#include "esp_timer.h"
#include "driver/gpio.h"

#ifdef WE_PLATFORM_SDL
#include "Input_SDL.h"
#endif

Controller* InputManager::getController(int index) {
#ifdef WE_PLATFORM_SDL
    if (index == 0) return &m_controllers[0];
#endif
    if (index < 0 || index >= MAX_CONTROLLERS) return nullptr;
    if (!INPUT_SETTINGS.controllers[index].enabled) return nullptr;
    return &m_controllers[index];
}

void InputManager::init() {

    // ── GPIO setup for all enabled controllers ────────────────
    uint64_t pinMask = 0;
    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!INPUT_SETTINGS.controllers[c].enabled) continue;
        for (int i = 0; i < BUTTON_COUNT; i++) {
            if (INPUT_SETTINGS.controllers[c].gpioPins[i] != -1)
                pinMask |= (1ULL << INPUT_SETTINGS.controllers[c].gpioPins[i]);
        }
    }

    if (pinMask != 0) {
        // Use activeLow from first enabled controller — all controllers
        // should use the same electrical configuration on one device.
        bool activeLow = INPUT_SETTINGS.controllers[0].activeLow;
        for (int c = 0; c < MAX_CONTROLLERS; c++) {
            if (INPUT_SETTINGS.controllers[c].enabled) {
                activeLow = INPUT_SETTINGS.controllers[c].activeLow;
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
        if (!INPUT_SETTINGS.controllers[c].enabled) continue;
        if (INPUT_SETTINGS.controllers[c].joyXEnabled ||
            INPUT_SETTINGS.controllers[c].joyYEnabled) {
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
            if (!INPUT_SETTINGS.controllers[c].enabled) continue;
            if (INPUT_SETTINGS.controllers[c].joyXEnabled)
                adc_oneshot_config_channel(m_adcHandle, INPUT_SETTINGS.controllers[c].joyXChannel, &chanCfg);
            if (INPUT_SETTINGS.controllers[c].joyYEnabled)
                adc_oneshot_config_channel(m_adcHandle, INPUT_SETTINGS.controllers[c].joyYChannel, &chanCfg);
        }
    }

    // ── Init each enabled controller ──────────────────────────
    int64_t now = esp_timer_get_time();
    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!INPUT_SETTINGS.controllers[c].enabled) continue;
        m_controllers[c].init(INPUT_SETTINGS.controllers[c], m_adcHandle, now);
    }
}

void InputManager::tick() {
#ifdef WE_PLATFORM_SDL
    Controller* c0 = getController(0);
    if (c0) SDLInput_flush(*c0);
#else
    int64_t now         = esp_timer_get_time();
    int64_t debounceUs  = static_cast<int64_t>(INPUT_SETTINGS.debounceMs) * 1000LL;

    for (int c = 0; c < MAX_CONTROLLERS; c++) {
        if (!INPUT_SETTINGS.controllers[c].enabled) continue;
        m_controllers[c].tick(now, debounceUs);
    }
#endif
}