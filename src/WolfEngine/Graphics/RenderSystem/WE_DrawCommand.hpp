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

struct FrameDiagnostics {
    uint16_t commandsSubmitted;  // running total — diff between frames for per-frame count
    uint16_t commandsDropped;    // running total — incremented on buffer overflow
    uint16_t commandsExecuted;   // running total — incremented per executed command
    uint16_t peakCommandCount;   // high watermark across all frames — never resets
};
