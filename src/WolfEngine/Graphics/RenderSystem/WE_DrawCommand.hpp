#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Layers.hpp"
#include "WolfEngine/Graphics/SpriteSystem/WE_SpriteRotation.hpp"

enum class DrawCommandType : uint8_t {
    Sprite,
    // leave room for future: TileMap, Text, Rect, Circle, Line, Pixel
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
            uint8_t  _free[3];  // 3 explicit free bytes (size@8, _free@9-11) — no silent padding
        } sprite;

        // future union members go here
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

static_assert(sizeof(DrawCommand) <= 24, "DrawCommand exceeds 24 bytes — check field ordering and padding");
static_assert(std::is_trivially_copyable_v<DrawCommand>);
static_assert(alignof(DrawCommand) <= 4);

struct FrameDiagnostics {
    uint16_t commandsSubmitted;  // per-frame — diff between frames for per-frame count
    uint16_t commandsDropped;    // per-frame — incremented on buffer overflow
    uint16_t commandsExecuted;   // per-frame — incremented per executed command
    uint16_t peakCommandCount;   // high watermark across all frames — never resets
};
