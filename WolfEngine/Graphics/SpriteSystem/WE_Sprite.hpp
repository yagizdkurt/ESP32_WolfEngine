#pragma once
#include <stdint.h>

// -------------------------------------------------------------
//  constexpr integer square root helper — compile time only
// -------------------------------------------------------------
constexpr int _isqrt(int n) {
    for (int i = 1; i <= n; i++)
        if (i * i == n) return i;
    return -1;  // not a perfect square
}

// =============================================================
//  WE_Sprite.hpp
//  A Sprite is a pure data asset — pixel indices and size.
//
//  USAGE:
//      constexpr uint8_t playerPixels[7 * 7] = { ... };
//      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);
//
//  Sprite::Create automatically deduces the size from the array,
//  enforces that it is a perfect square, and enforces that
//  the side length is a valid size (1, 3, 5, 7, ... 65).
// =============================================================
struct Sprite {
    const uint8_t* pixels;
    int            size;
    // ---------------------------------------------------------
    //  Safe factory method. Deduces size from the array,
    //  validates it is a perfect square and a supported size.
    //  Use this instead of constructing Sprite directly.
    //
    //  VALID SIZES: 1x1, 3x3, 5x5, 7x7, ... 65x65
    //
    //  EXAMPLE:
    //      constexpr uint8_t playerPixels[7 * 7] = { ... };
    //      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);
    // ---------------------------------------------------------
    template<int ArraySize>
    static constexpr Sprite Create(const uint8_t (&pixels)[ArraySize]) {
        constexpr int side = _isqrt(ArraySize);
        static_assert(side != -1,
            "Sprite pixel array must be a perfect square (e.g. 1*1, 3*3, 5*5)");
        static_assert(
            side == 1  || side == 3  || side == 5  || side == 7  || side == 9  || 
            side == 11 || side == 13 || side == 15 || side == 17 || side == 19 || 
            side == 21 || side == 23 || side == 25 || side == 27 || side == 29 || 
            side == 31 || side == 33 || side == 35 || side == 37 || side == 39 || 
            side == 41 || side == 43 || side == 45 || side == 47 || side == 49 || 
            side == 51 || side == 53 || side == 55 || side == 57 || side == 59 || 
            side == 61 || side == 63,
            "Sprite size must be an odd number between and including (1,65)");
        return { pixels, side };
    }
};