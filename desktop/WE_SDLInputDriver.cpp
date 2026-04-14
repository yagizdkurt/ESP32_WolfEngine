#ifdef WE_PLATFORM_SDL
#include "WE_SDLInputDriver.hpp"

// ── Static member definitions ─────────────────────────────────
std::atomic<uint32_t> SDLInputDriver::s_buttons{0};
std::atomic<uint8_t>  SDLInputDriver::s_joyKeys{0};

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────

static int buttonIndex(SDL_Keycode key) {
    const SDLKeyMap& m = DEFAULT_KEY_MAP;
    const SDL_Keycode table[10] = {
        m.buttonA, m.buttonB, m.buttonC, m.buttonD, m.buttonE,
        m.buttonF, m.buttonG, m.buttonH, m.buttonI, m.buttonJ
    };
    for (int i = 0; i < 10; i++)
        if (table[i] == key) return i;
    return -1;
}

// ─────────────────────────────────────────────────────────────
//  processEvent — main SDL thread
// ─────────────────────────────────────────────────────────────

void SDLInputDriver::processEvent(const SDL_Event& e) {
    const bool down = (e.type == SDL_EVENT_KEY_DOWN);
    const bool up   = (e.type == SDL_EVENT_KEY_UP);
    if (!down && !up) return;

    SDL_Keycode k = e.key.key;
    const SDLKeyMap& m = DEFAULT_KEY_MAP;

    // Buttons A–J
    int idx = buttonIndex(k);
    if (idx >= 0) {
        uint32_t mask = 1u << idx;
        if (down) s_buttons.fetch_or (mask,  std::memory_order_relaxed);
        else      s_buttons.fetch_and(~mask, std::memory_order_relaxed);
        return;
    }

    // Joystick direction keys
    uint8_t bit = 0;
    if      (k == m.joyXPos) bit = 0x01;
    else if (k == m.joyXNeg) bit = 0x02;
    else if (k == m.joyYPos) bit = 0x04;
    else if (k == m.joyYNeg) bit = 0x08;
    if (!bit) return;

    if (down) s_joyKeys.fetch_or (bit,  std::memory_order_relaxed);
    else      s_joyKeys.fetch_and(~bit, std::memory_order_relaxed);
}

// ─────────────────────────────────────────────────────────────
//  flush — engine thread (called every frame)
// ─────────────────────────────────────────────────────────────

void SDLInputDriver::flush(Controller& c) {
    // Push button state for every button so prevState stays in sync.
    uint32_t btns = s_buttons.load(std::memory_order_relaxed);
    for (int i = 0; i < 10; i++)
        c.simulateButton(static_cast<Button>(i), (btns >> i) & 1u);

    // Derive axis values from held direction keys.
    uint8_t jk = s_joyKeys.load(std::memory_order_relaxed);
    float x = 0.0f, y = 0.0f;
    if (jk & 0x01) x += 1.0f;
    if (jk & 0x02) x -= 1.0f;
    if (jk & 0x04) y += 1.0f;
    if (jk & 0x08) y -= 1.0f;
    c.simulateJoystick(JoyAxis::X, x);
    c.simulateJoystick(JoyAxis::Y, y);
}

// ─────────────────────────────────────────────────────────────
//  SDLInput_flush — free function declared in stubs/Input_SDL.h,
//  called by InputManager::tick() on the SDL platform.
// ─────────────────────────────────────────────────────────────
void SDLInput_flush(Controller& c) { SDLInputDriver::flush(c); }

#endif // WE_PLATFORM_SDL
