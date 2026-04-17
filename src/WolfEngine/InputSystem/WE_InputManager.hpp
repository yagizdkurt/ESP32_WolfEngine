#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/InputSystem/WE_Controller.hpp"
#include "esp_adc/adc_oneshot.h"

class WolfEngine;

// =============================================================
//  InputManager
//  Singleton — access via the free function Input().
//  Owns all Controller instances and the shared ADC handle.
//  Ticks all enabled controllers every frame.
// =============================================================
class InputManager {
public:
    Controller* getController(int index);
private:
    static constexpr int MAX_CONTROLLERS = 4;
    Controller                m_controllers[MAX_CONTROLLERS];
    adc_oneshot_unit_handle_t m_adcHandle = nullptr;
    void init();
    void tick();
    friend class WolfEngine;
    InputManager() = default;
};