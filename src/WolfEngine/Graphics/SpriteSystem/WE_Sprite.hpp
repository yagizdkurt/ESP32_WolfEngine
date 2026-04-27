#pragma once
#include <stdint.h>

// =============================================================
//  WE_Sprite.hpp
//  A Sprite is a pure data asset — pixel indices, palette,
//  dimensions, and an explicit anchor point.
//
//  USAGE:
//      // Declare pixels as a 2D array [rows][cols] = [H][W]
//      constexpr uint8_t playerPixels[5][7] = { ... };
//
//      // Zero template args — H, W, anchorX, anchorY all deduced/defaulted:
//      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels, PALETTE_WARM);
//
//      // All four explicit — required when a custom anchor is needed:
//      constexpr Sprite SPRITE_PLAYER = Sprite::Create<5, 7, 3, 1>(playerPixels, PALETTE_WARM);
//
//  Sprite::Create() accepts a 2D array reference so the compiler
//  deduces width and height automatically. Rectangular sprites are
//  fully supported.
//
//  VALID SIZES: width 1–63, height 1–63 (independently)
//
//  NOTE: Pixel arrays must be declared as separate constexpr variables.
//  Inline initializer lists are not supported — the pixel pointer would
//  dangle and the dimensions cannot be deduced at compile time.
// =============================================================
struct Sprite {
    const uint8_t*  pixels;
    const uint16_t* palette;
    uint8_t         width;
    uint8_t         height;
    uint8_t         anchorX;   // default = width/2
    uint8_t         anchorY;   // default = height/2

    void setPalette(const uint16_t* newPalette) { palette = newPalette; }

    // ---------------------------------------------------------
    //  Safe factory method. Accepts a 2D array [H][W]; H and W
    //  are deduced by the compiler. AX and AY default to W/2
    //  and H/2 but can be overridden by providing all four
    //  template arguments explicitly.
    //
    //  REQUIREMENT: The pixel array must be a compile-time
    //  constant. Use this for constexpr sprite assets only.
    //
    //  EXAMPLE — zero args (default anchor):
    //      constexpr uint8_t px[5][7] = { ... };
    //      constexpr Sprite S = Sprite::Create(px, PALETTE_WARM);
    //
    //  EXAMPLE — all four explicit (custom anchor):
    //      constexpr Sprite S = Sprite::Create<5, 7, 3, 1>(px, PALETTE_WARM);
    // ---------------------------------------------------------
    template<uint8_t H, uint8_t W, uint8_t AX = W / 2, uint8_t AY = H / 2>
    static consteval Sprite Create(const uint8_t (&pixels)[H][W],
                                   const uint16_t* palette) {
        static_assert(W > 0 && W <= 63, "width must be between 1 and 63");
        static_assert(H > 0 && H <= 63, "height must be between 1 and 63");
        static_assert(AX < W,           "anchorX must be < width");
        static_assert(AY < H,           "anchorY must be < height");
        return { &pixels[0][0], palette, W, H, AX, AY };
    }

    // ---------------------------------------------------------
    //  Dangerous factory method. Accepts a 2D array [H][W]; H
    //  and W are deduced by the compiler. AX and AY default to
    //  W/2 and H/2 but can be overridden by providing all four
    //  template arguments explicitly.
    //
    //  WARNING: This creates sprites from non-constant data and
    //  is dangerous for fragmentation issues. It should be used
    //  cautiously, and only for dynamic sprite creation when
    //  compile-time sprite storage is not appropriate.
    //
    //  EXAMPLE — zero args (default anchor):
    //      uint8_t px[5][7] = { ... };
    //      Sprite S = Sprite::DynamicCreate(px, PALETTE_WARM);
    //
    //  EXAMPLE — all four explicit (custom anchor):
    //      Sprite S = Sprite::DynamicCreate<5, 7, 3, 1>(px, PALETTE_WARM);
    // ---------------------------------------------------------
    template<uint8_t H, uint8_t W, uint8_t AX = W / 2, uint8_t AY = H / 2>
    static constexpr Sprite DynamicCreate(const uint8_t (&pixels)[H][W],
                                          const uint16_t* palette) {
        static_assert(W > 0 && W <= 63, "width must be between 1 and 63");
        static_assert(H > 0 && H <= 63, "height must be between 1 and 63");
        static_assert(AX < W,           "anchorX must be < width");
        static_assert(AY < H,           "anchorY must be < height");
        return { &pixels[0][0], palette, W, H, AX, AY };
    }
};
