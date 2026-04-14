#pragma once
#include <SDL3/SDL_keycode.h>

// =============================================================
//  SDLKeyMap
//  Compile-time keyboard mapping for the SDL input emulation.
//  One SDL_Keycode per Button (A-J) and four keys for the
//  analog joystick axes (X+/X-/Y+/Y-).
//
//  To remap: copy DEFAULT_KEY_MAP and change the fields you need.
//  The SDLInputDriver reads DEFAULT_KEY_MAP at runtime.
// =============================================================

struct SDLKeyMap {
    SDL_Keycode buttonA;    // Button::A
    SDL_Keycode buttonB;    // Button::B
    SDL_Keycode buttonC;    // Button::C
    SDL_Keycode buttonD;    // Button::D
    SDL_Keycode buttonE;    // Button::E
    SDL_Keycode buttonF;    // Button::F
    SDL_Keycode buttonG;    // Button::G
    SDL_Keycode buttonH;    // Button::H
    SDL_Keycode buttonI;    // Button::I
    SDL_Keycode buttonJ;    // Button::J
    SDL_Keycode joyXPos;    // Joystick X+ (right)
    SDL_Keycode joyXNeg;    // Joystick X- (left)
    SDL_Keycode joyYPos;    // Joystick Y+ (down in screen space)
    SDL_Keycode joyYNeg;    // Joystick Y- (up in screen space)
};

// -------------------------------------------------------------
//  Default mapping
//
//  Buttons:
//    A = Z        action 1
//    B = X        action 2
//    C = C
//    D = V
//    E = Up       d-pad up
//    F = Down     d-pad down
//    G = Left     d-pad left
//    H = Right    d-pad right
//    I = LShift   shoulder / run
//    J = Space    jump / confirm
//
//  Joystick:
//    X axis — D (right) / A (left)
//    Y axis — S (down)  / W (up)
// -------------------------------------------------------------
inline constexpr SDLKeyMap DEFAULT_KEY_MAP = {
    .buttonA = SDLK_Z,
    .buttonB = SDLK_X,
    .buttonC = SDLK_C,
    .buttonD = SDLK_V,
    .buttonE = SDLK_UP,
    .buttonF = SDLK_DOWN,
    .buttonG = SDLK_LEFT,
    .buttonH = SDLK_RIGHT,
    .buttonI = SDLK_LSHIFT,
    .buttonJ = SDLK_SPACE,
    .joyXPos = SDLK_D,
    .joyXNeg = SDLK_A,
    .joyYPos = SDLK_S,
    .joyYNeg = SDLK_W,
};
