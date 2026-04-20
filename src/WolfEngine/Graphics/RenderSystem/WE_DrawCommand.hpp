#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Layers.hpp"
#include "WolfEngine/Graphics/SpriteSystem/WE_SpriteRotation.hpp"

// =============================================================
//  DrawCommand Memory Budget (DO NOT FORGET)
//
//  Hard limit:
//    sizeof(DrawCommand) == 20 bytes  (enforced by static_assert)
//
//  Layout:
//    - Header (type/flags/x/y/sortKey) = 8 bytes
//    - Union payload                    = 12 bytes
//
//  Rules when adding new union variants:
//    1) Keep every union member exactly 12 bytes.
//    2) Use explicit _free bytes — no silent compiler padding.
//    3) Put wider fields (uint16_t, pointers) before narrow ones to
//       avoid alignment holes; adjust _free count to reach 12 exactly.
//    4) If size must increase, update the limit intentionally and
//       re-evaluate RAM cost: MAX_DRAW_COMMANDS * sizeof(DrawCommand).
// =============================================================
enum class DrawCommandType : uint8_t {
    Sprite,
    FillRect,
    Line,
    Circle,
    TextRun,
    Count   // sentinel — always last
};

struct DrawCommand {
    DrawCommandType type;       // offset 0  — 1 byte
    uint8_t         flags;      // offset 1  — 1 byte
                                //   bits 7-6 : Rotation (00=R0 01=R90 10=R180 11=R270)
                                //   bit  5   : reserved (isUI — for migration)
                                //   bits 4-0 : free

    int16_t         x;          // offset 2  — screen-space, already transformed
    int16_t         y;          // offset 4
    uint16_t        sortKey;    // offset 6  — high byte: RenderLayer, low byte: screenY
    // ── 8 byte header ──

    union {
        struct {
            const uint8_t*  pixels;
            const uint16_t* palette;
            uint8_t  size;
            uint8_t  _free[3];  // 3 explicit free bytes — no silent padding
        } sprite;                // 12 bytes

        struct {
            uint16_t color;     // pre-resolved RGB565 at submit time
            uint8_t  w;
            uint8_t  h;
            uint8_t  _free[8];
        } fillRect;             // 12 bytes

        struct {
            int16_t  x2;        // end point — x1/y1 come from cmd.x, cmd.y
            int16_t  y2;
            uint16_t color;     // pre-resolved RGB565 at submit time
            uint8_t  _free[6];
        } line;                 // 12 bytes

        struct {
            uint16_t color;     // pre-resolved RGB565 at submit time (color before radius to avoid padding)
            uint8_t  radius;
            uint8_t  filled;    // 0 = outline, 1 = filled
            uint8_t  _free[8];
        } circle;               // 12 bytes

        struct {
            const char* text;   // pointer to UILabelState::text — stable for frame lifetime
            uint16_t    color;  // pre-resolved RGB565 at submit time
            uint8_t     maxWidth; // clip width in pixels, 0 = no clip
            uint8_t     _free[5];
        } textRun;              // 12 bytes
    };
};

constexpr Rotation    cmdGetRotation(uint8_t flags) {
    return static_cast<Rotation>(flags >> 6);
}
constexpr uint8_t     cmdSetRotation(uint8_t flags, Rotation r) {
    return (flags & 0x3F) | (static_cast<uint8_t>(r) << 6);
}
constexpr RenderLayer cmdGetLayer(uint16_t sortKey) {
    return static_cast<RenderLayer>(sortKey >> 8);
}
constexpr uint16_t    cmdMakeSortKey(RenderLayer layer, uint8_t screenY) {
    return (static_cast<uint16_t>(layer) << 8) | screenY;
}

#if UINTPTR_MAX == 0xFFFFFFFF
static_assert(sizeof(DrawCommand) == 20, "DrawCommand must be exactly 20 bytes — check field ordering and padding");
#endif
static_assert(std::is_trivially_copyable_v<DrawCommand>);
static_assert(alignof(DrawCommand) <= 4);

struct FrameDiagnostics {
    uint16_t commandsSubmitted;  // per-frame — diff between frames for per-frame count
    uint16_t commandsDropped;    // per-frame — incremented on buffer overflow
    uint16_t commandsExecuted;   // per-frame — incremented per executed command
    uint16_t peakCommandCount;   // high watermark across all frames — never resets
};
