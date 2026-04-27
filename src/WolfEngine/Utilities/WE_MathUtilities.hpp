#pragma once
#include <stdint.h>

namespace WE_Math {

/**
 * @brief Clamps a signed 16-bit value to the byte range [0, 255].
 *
 * Utility for safely narrowing intermediate math values to uint8_t without
 * wrap-around. Negative values are saturated to 0 and values above 255 are
 * saturated to 255.
 *
 * @param v Input value to clamp.
 * @return uint8_t Clamped value in [0, 255].
 */
inline uint8_t clampToByte(int16_t v) {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return static_cast<uint8_t>(v);
}

} // namespace WE_Math