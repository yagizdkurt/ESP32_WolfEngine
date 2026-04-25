#pragma once

// =============================================================
//  WolfEngine Color Palette System — Master Include & Documentation
// =============================================================
//
//  OVERVIEW
//  --------
//  The palette system is built around the RGB565 color format,
//  which is a 16-bit color encoding used natively by the ST7735
//  and most small SPI displays. Each color is packed as:
//
//      Bit  15-11 : Red   (5 bits, 0-31)
//      Bit  10-5  : Green (6 bits, 0-63)
//      Bit  4-0   : Blue  (5 bits, 0-31)
//
//  Each palette is an array of exactly 32 RGB565 color entries.
//  Sprites store pixel data as one byte per pixel, where the byte
//  value is an index (0-31) into the palette array. This keeps
//  sprite data simple and pixel access completely straightforward.
//
//  INDEX 0 — TRANSPARENT (RESERVED)
//  ---------------------------------
//  Index 0 is always reserved as the transparent color across
//  ALL palettes. When the renderer encounters a pixel with index
//  0, it skips drawing that pixel entirely, letting whatever is
//  on the layer below show through. The actual RGB565 value stored
//  at index 0 does not matter and will never be drawn.
//  Do NOT use index 0 as a visible color in any palette.
//
//  BUILT-IN PALETTES
//  -----------------
//  PALETTE_GRAYSCALE — 31 shades from near-black to white.
//                      Good for UI elements, shadows, monochrome
//                      sprites, and debug visuals.
//
//  PALETTE_WARM      — Reds, oranges, yellows, and earth tones.
//                      Good for fire, lava, desert, and warm scenes.
//
//  PALETTE_COOL      — Blues, greens, and teals.
//                      Good for water, ice, forests, and night scenes.
//
//  PALETTE_GAMEBOY   — Inspired by the original DMG Game Boy LCD.
//                      Classic green-tinted shades for retro aesthetics.
//
//  PALETTE_SUNSET    — Deep purples, magentas, and warm pinks.
//                      Good for evening skies and synthwave aesthetics.
//
//  PALETTE SWAPPING
//  ----------------
//  Since sprites hold a pointer to their palette rather than a
//  copy, swapping the palette at runtime is essentially free —
//  just reassign the pointer. This enables tricks like:
//
//      - Color variants  : Same sprite, different palette = different
//                          character skin, team color, enemy type, etc.
//
//      - Damage flash    : Temporarily swap to an all-red palette
//                          for one frame when a character is hit.
//
//      - Day/night cycle : Swap to a darker, cooler palette at night.
//
//  Example:
//      mySprite.setPalette(PALETTE_WARM);   // normal state
//      mySprite.setPalette(PALETTE_SUNSET); // hit flash
//
// -------------------------------------------------------------
//  CREATING YOUR OWN PALETTE
//  -------------------------
//  Create a .hpp file anywhere in your project and define a
//  constexpr uint16_t array of exactly 32 entries.
//  Remember: index 0 must always be 0x0000 (transparent).
//
//      #pragma once
//      #include <stdint.h>
//
//      constexpr uint16_t MY_PALETTE[32] = {
//          0x0000,  // 0  - Transparent (always required)
//          0x....,  // 1
//          0x....,  // 2
//          ...
//          0x....,  // 31
//      };
//
//  Then include your file and assign it to a sprite:
//      #include "MyPalette.hpp"
//      mySprite.setPalette(MY_PALETTE);
//
//  SPRITE DATA FORMAT
//  ------------------
//  Sprite pixel arrays use one byte per pixel. Each byte is a
//  palette index (0-31). Row-major order, left to right, top to
//  bottom. Index 0 is always transparent and will not be drawn.
//
//  Convention for writing sprite arrays in code — use a comment
//  map above each row so the data is human readable:
//
//      // 0 = transparent, 1 = outline, 2 = fill, 3 = highlight
//      constexpr uint8_t MY_SPRITE[64] = { // 8x8
//          // 0 0 0 1 1 0 0 0
//             0,0,0,1,1,0,0,0,
//          // 0 0 1 2 2 1 0 0
//             0,0,1,2,2,1,0,0,
//          ...
//      };
//
//  TIPS
//  ----
//  - Define palettes as constexpr so they live in flash, not RAM.
//  - Keep palette files focused — one palette per file is cleaner.
//  - Name your palette clearly to reflect its use or theme.
//  - Test colors on the actual display — RGB565 on an ST7735 can
//    look different from what you see on a PC monitor, especially
//    greens and blues.
//  - Don't forget the byte-swap. Seriously.
// =============================================================

#include "WE_Palette_Grayscale.hpp"
#include "WE_Palette_Warm.hpp"
#include "WE_Palette_Cool.hpp"
#include "WE_Palette_GameBoy.hpp"
#include "WE_Palette_Sunset.hpp"


/**
 * @brief Resolves a palette index into an RGB565 color with engine-safe semantics.
 *
 * This helper centralizes palette lookup rules used by sprite/UI rendering:
 * - Index `0` is always treated as transparent and returns `0x0000`.
 * - Non-zero indices are expected to be within configured palette bounds.
 * - Out-of-bounds indices are treated as transparent, logged, and asserted in debug paths.
 *
 * @param palette Pointer to a palette table containing RGB565 entries.
 * @param index Palette index encoded in sprite/UI pixel data.
 *
 * @return RGB565 color value for valid non-zero indices.
 * @return `0x0000` when index is transparent (`0`) or invalid/out-of-range.
 *
 * @note This function is intentionally defensive: it prevents undefined reads when
 * invalid asset data slips through and makes the error visible via logging/assert.
 */
inline uint16_t ResolvePaletteColor(const uint16_t* palette, uint8_t index) {
    if (index == 0) return 0x0000; // transparent
    if (index >= Settings.limits.maxPaletteInexes){
        ESP_LOGW("Renderer:", "Palette index %u out of bounds — treated as transparent", index);
        assert(false); // catch invalid palette indexes in testing — likely a bug in client code
        return 0x0000; // treat out-of-bounds indexes as transparent
    }
    return palette[index];
}