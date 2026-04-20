#pragma once
#include <stdint.h>
#include <utility>
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WE_UITransformHelpers.hpp"

// =============================================================
//  UIAnchor
//  Nine standard anchor presets that describe which corner or
//  edge of the screen a UITransform is positioned relative to.
//  x/y offsets in UITransform are applied AFTER anchor origin
//  and margin adjustments.
//
//  Screen layout:
//
//      TopLeft -------- TopCenter -------- TopRight
//         |                  |                  |
//      MidLeft  ---------- Center  ---------- MidRight
//         |                  |                  |
//      BotLeft -------- BotCenter -------- BotRight
//
//  EXAMPLE:
//      UIAnchor::BotLeft + x=0 + y=0 → bottom-left corner.
//      UIAnchor::Center  + x=4 + y=0 → 4px right of screen center.
// =============================================================
// WARNING: DO NOT CHANGE THE LAYOUT BECAUSE MATH DEPENDS ON IT.
// =============================================================
enum class UIAnchor : uint8_t {
    TopLeft   = 0, TopCenter = 1, TopRight  = 2,
    MidLeft   = 3, Center    = 4, MidRight  = 5,
    BotLeft   = 6, BotCenter = 7, BotRight  = 8,
};

// =============================================================
//  UIRect
//  Resolved, absolute rectangle on screen after all anchor,
//  offset, and margin calculations are applied.
//  Returned by resolveLayout(). Read-only — never stored as
//  persistent state, always recomputed from UITransform.
// =============================================================
struct UIRect {
    int16_t x;      // absolute left edge pixel
    int16_t y;      // absolute top edge pixel
    int16_t width;
    int16_t height;

    constexpr int16_t x2()      const { return static_cast<int16_t>(x + width);            }
    constexpr int16_t y2()      const { return static_cast<int16_t>(y + height);           }
    constexpr int16_t centerX() const { return static_cast<int16_t>(x + floorDiv2(width));  }
    constexpr int16_t centerY() const { return static_cast<int16_t>(y + floorDiv2(height)); }
    constexpr bool    isEmpty() const { return width <= 0 || height <= 0; }
};

// =============================================================
//  UITransform
//  Describes how a UI element is positioned and sized on screen.
//  x/y are pixel offsets applied AFTER the anchor origin and
//  margins. width/height define the element's bounding box.
//  Margins push the element away from its anchored edge;
//  center-anchored axes ignore margins.
//  Intended to be stored in flash (const / PROGMEM).
// =============================================================
struct UITransform {
    int16_t  x;
    int16_t  y;
    int16_t  width;
    int16_t  height;
    uint8_t  layer;
    UIAnchor anchor;
    int8_t   marginLeft;
    int8_t   marginRight;
    int8_t   marginTop;
    int8_t   marginBottom;

    constexpr UITransform(
        int16_t x_ = 0, int16_t y_ = 0,
        int16_t w_ = 0, int16_t h_ = 0,
        uint8_t layer_ = 0,
        UIAnchor a = UIAnchor::Center,
        int8_t ml = 0, int8_t mr = 0,
        int8_t mt = 0, int8_t mb = 0)
        : x(x_), y(y_), width(w_), height(h_),
          layer(layer_), anchor(a),
          marginLeft(ml), marginRight(mr),
          marginTop(mt), marginBottom(mb) {}
};

// =============================================================
//  sanitizeAnchorIndex
//  Guarantees anchor index is within [0..8].
//  Fallback strategy for invalid enum values: Center (index 4).
//  This keeps layout deterministic under bad casts/corruption.
// =============================================================
[[nodiscard]] constexpr inline uint8_t sanitizeAnchorIndex(UIAnchor anchor) noexcept
{
    const uint8_t anchorIndex = static_cast<uint8_t>(anchor);
    // Deterministic fallback for invalid enum values: treat as Center.
    return (anchorIndex <= 8u) ? anchorIndex : static_cast<uint8_t>(UIAnchor::Center);
}


// =============================================================
//  anchorOrigin
//  Returns the raw (x, y) pixel origin for an element of
//  (elemW x elemH) given an anchor preset. No offsets or margins
//  are applied — this is purely the snapped corner/edge/center
//  position on screen. Used internally by resolveAnchor and
//  resolveLayout. Invalid anchors are sanitized to Center first.
// =============================================================
[[nodiscard]] constexpr std::pair<int16_t, int16_t>
anchorOrigin(UIAnchor anchor, int16_t elemW, int16_t elemH) noexcept
{
    const uint8_t anchorIndex  = sanitizeAnchorIndex(anchor);
    const int16_t anchorCol    = static_cast<int16_t>(anchorIndex % 3); // 0=left, 1=center, 2=right
    const int16_t anchorRow    = static_cast<int16_t>(anchorIndex / 3); // 0=top,  1=middle, 2=bottom
    const int16_t centerX      = floorDiv2(static_cast<int32_t>(Renderer::SCREEN_WIDTH)  - elemW);
    const int16_t centerY      = floorDiv2(static_cast<int32_t>(Renderer::SCREEN_HEIGHT) - elemH);

    // Horizontal origin: left edge | horizontally centered | right edge
    const int16_t originX = (anchorCol == 0) ? static_cast<int16_t>(0) :
                            (anchorCol == 1) ? centerX :
                                               static_cast<int16_t>(Renderer::SCREEN_WIDTH - elemW);

    // Vertical origin: top edge | vertically centered | bottom edge
    const int16_t originY = (anchorRow == 0) ? static_cast<int16_t>(0) :
                            (anchorRow == 1) ? centerY :
                                               static_cast<int16_t>(Renderer::SCREEN_HEIGHT - elemH);

    return { originX, originY };
}

// =============================================================
//  resolveAnchor
//  Returns the absolute UIRect for an element of (elemW x elemH)
//  placed at its anchor origin with no x/y offset and no margins.
//  Useful when you only need the anchor-snapped position without
//  a full UITransform (e.g. measuring layout bounds).
// =============================================================
[[nodiscard]] constexpr
UIRect resolveAnchor(UIAnchor anchor, int16_t elemW, int16_t elemH) noexcept
{
    const auto [ox, oy] = anchorOrigin(anchor, elemW, elemH);
    return { ox, oy, elemW, elemH };
}

// =============================================================
//  resolveLayout
//  Converts a UITransform into an absolute UIRect by:
//    1. Computing the anchor origin (anchorOrigin).
//    2. Applying edge-aware margins (push away from anchored edge).
//    3. Adding the x/y pixel offset from the transform.
//  Invalid anchors use Center as deterministic fallback.
//  This is the primary function used by UI elements to determine
//  where they should be drawn on screen.
// =============================================================
[[nodiscard]] constexpr
UIRect resolveLayout(const UITransform& tf) noexcept
{
    const uint8_t anchorIndex = sanitizeAnchorIndex(tf.anchor);
    const int16_t anchorCol   = static_cast<int16_t>(anchorIndex % 3); // 0=left, 1=center, 2=right
    const int16_t anchorRow   = static_cast<int16_t>(anchorIndex / 3); // 0=top,  1=middle, 2=bottom
    const auto [originX, originY] = anchorOrigin(tf.anchor, tf.width, tf.height);

    // Margin nudges element away from the edge it is anchored to; center-anchored axes get no margin.
    const int16_t marginOffsetX = (anchorCol == 0) ?  tf.marginLeft  :  // anchored left   → push right
                                  (anchorCol == 2) ? -tf.marginRight :  // anchored right  → push left
                                                      0;                // center col      → no margin
    const int16_t marginOffsetY = (anchorRow == 0) ?  tf.marginTop    : // anchored top    → push down
                                  (anchorRow == 2) ? -tf.marginBottom : // anchored bottom → push up
                                                      0;                // center row      → no margin
    // final_position = anchor_origin + margin_offset + transform_offset
    return {
        static_cast<int16_t>(originX + marginOffsetX + tf.x),
        static_cast<int16_t>(originY + marginOffsetY + tf.y),
        tf.width,
        tf.height
    };
}