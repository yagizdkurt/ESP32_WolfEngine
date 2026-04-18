#pragma once
#include <atomic>
#include <SDL3/SDL_events.h>
#include "WolfEngine/InputSystem/WE_IInputProvider.hpp"
#include "WolfEngine/InputSystem/WE_Controller.hpp"
#include "WE_SDLKeyMap.hpp"

// =============================================================
//  SDLInputDriver
//  Bridges SDL3 keyboard events to the WolfEngine Controller API.
//
//  Threading model:
//    processEvent() — called from the main SDL thread's event loop.
//    flush()        — called from the engine background thread via
//                     InputManager::tick() each frame.
//
//  std::atomic provides the visibility guarantee between threads
//  without requiring a mutex.
//
//  Limitations: see SDL_ISSUES.md.
// =============================================================
class SDLInputDriver : public IInputProvider {
public:
    // Process one SDL event. Call inside the SDL_PollEvent loop in
    // main_desktop.cpp for every event before other handlers.
    static void processEvent(const SDL_Event& e);

    // Read shared atomic state and push it into the controllers via
    // simulateButton() / simulateJoystick(). Must be called every
    // frame for correct edge-detection (getButtonDown / getButtonUp).
    void flush(Controller* controllers, int count) override;

private:
    static std::atomic<uint32_t> s_buttons;  // bit i = Button(i) held
    static std::atomic<uint8_t>  s_joyKeys;  // bit 0=X+  1=X-  2=Y+  3=Y-
};
