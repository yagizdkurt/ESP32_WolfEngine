#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/InputSystem/WE_Controller.hpp"
#include "WolfEngine/InputSystem/WE_IInputProvider.hpp"
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

    /// @brief Override the hardware polling path with a custom input provider.
    /// @param provider Pointer to the provider called each tick. Pass nullptr to
    ///                 restore the default hardware GPIO/ADC path.
    /// @note Must be called before StartGame().
    void setInputProvider(IInputProvider* provider);

    /// @brief Control whether getController(0) bypasses the settings enabled check.
    /// @param value When true, controller 0 is always returned even if its settings
    ///              mark it as disabled. Use on platforms where settings are not
    ///              configured for hardware input.
    void setAlwaysEnableController0(bool value);
private:
    static constexpr int MAX_CONTROLLERS = 4;
    Controller                m_controllers[MAX_CONTROLLERS];
    adc_oneshot_unit_handle_t m_adcHandle = nullptr;
    IInputProvider*           m_inputProvider = nullptr;
    bool                      m_alwaysEnableController0 = false;
    void init();
    void tick();
    friend class WolfEngine;
    InputManager() = default;
};