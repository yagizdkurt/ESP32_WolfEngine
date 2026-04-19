#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Layers.hpp"
#include "WolfEngine/Graphics/SpriteSystem/WE_SpriteRotation.hpp"

enum class DrawCommandType : uint8_t {
    Sprite,
    // leave room for future: TileMap, Text, Rect, Circle, Line, Pixel
};

struct DrawCommand {
    DrawCommandType type;
    RenderLayer     layer;
    int16_t         x;        // screen-space top-left corner (already transformed)
    int16_t         y;        // screen-space top-left corner
    int16_t         sortKey;  // secondary sort within layer (typically screenY)

    union {
        struct {
            const uint8_t*  pixels;
            const uint16_t* palette;
            int             size;
            Rotation        rotation;
        } sprite;

        // future union members go here
    };
};

static_assert(sizeof(DrawCommand) <= 24, "DrawCommand exceeds 24 bytes — check field ordering and padding");
static_assert(std::is_trivially_copyable_v<DrawCommand>);
static_assert(alignof(DrawCommand) <= 4);

struct FrameDiagnostics {
    uint16_t commandsSubmitted;  // per-frame — diff between frames for per-frame count
    uint16_t commandsDropped;    // per-frame — incremented on buffer overflow
    uint16_t commandsExecuted;   // per-frame — incremented per executed command
    uint16_t peakCommandCount;   // high watermark across all frames — never resets
};
