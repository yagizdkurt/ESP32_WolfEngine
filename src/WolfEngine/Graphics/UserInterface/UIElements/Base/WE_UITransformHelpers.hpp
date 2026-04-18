#include <stdint.h>

// =============================================================
//  floorDiv2
//  Returns floor(value / 2) using integer-only math.
//
//  WHY: C++ signed integer division truncates toward zero, not
//  toward negative infinity. For positive numbers these agree,
//  but for negative odd values they differ by 1:
//
//      -3 / 2  →  -1   (C++ truncation)
//      floor(-3 / 2.0) = floor(-1.5)  →  -2   (correct floor)
//
//  This matters for centering: the formula is
//  floor((SCREEN_WIDTH - elemW) / 2). If an element is wider
//  than the screen, that difference goes negative and C++
//  truncation would place the element 1px off-center.
//
//  WHY IT RARELY TRIGGERS: On a 128x160 screen all normal UI
//  elements are smaller than the screen, so the difference is
//  always positive and truncation == floor. The correction only
//  fires for elements wider/taller than the screen, which should
//  never happen in valid usage.
//
//  Takes int32_t so callers can safely widen before passing in
//  without a narrowing mismatch at the call site.
// =============================================================
[[nodiscard]] constexpr inline int16_t floorDiv2(int32_t value) noexcept
{
    // C++ signed division truncates toward zero; subtract 1 for negative odd values to match floor().
    const int32_t truncated = value / 2;
    const bool isNegativeOdd = (value < 0) && ((value % 2) != 0);
    return static_cast<int16_t>(truncated - (isNegativeOdd ? 1 : 0));
}