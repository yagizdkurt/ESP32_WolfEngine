#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/Graphics/UserInterface/Fonts/WE_Font.hpp"
#include "WolfEngine/WolfEngine.hpp"
#include "esp_log.h"
#include <cassert>

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

void Renderer::initialize() {
    m_driver->initialize();
    assert(m_driver->screenWidth  == Settings.render.screenWidth  &&
           m_driver->screenHeight == Settings.render.screenHeight &&
           "Driver dimensions do not match Settings.render screen size");
}


// -------------------------------------------------------------
//  clearCommands
//  Resets the command buffer write pointer. Does not touch
//  diagnostics — those are managed by beginFrame() and
//  executeCommands() independently.
// -------------------------------------------------------------
void Renderer::clearCommands() {
    m_commandCount = 0;
}


// -------------------------------------------------------------
//  submitDrawCommand
//  Called by SpriteRenderer (and future components) during the
//  component-tick phase to enqueue a draw operation for this frame.
//  Overflow is loud: dropped commands are counted and logged.
// -------------------------------------------------------------
bool Renderer::submitDrawCommand(const DrawCommand& cmd) {
    if (m_commandCount >= Settings.render.maxDrawCommands) {
        if (m_diagnostics.commandsDropped == 0) {
            ESP_LOGW("Renderer", "Draw command buffer full — first drop this frame");
        }
        m_diagnostics.commandsDropped++;
        return false;
    }
    
    m_commandBuffer[m_commandCount++] = cmd;
    m_diagnostics.commandsSubmitted++;
    return true;
}


// -------------------------------------------------------------
//  drawSpriteInternal
//  Writes a single sprite into the framebuffer given unpacked
//  command fields. Handles rotation index math, transparency,
//  and per-pixel bounds checking against the game region.
// -------------------------------------------------------------
void IRAM_ATTR Renderer::drawSpriteInternal(int16_t x, int16_t y, 
    const uint8_t*  pixels, const uint16_t* palette, int size, Rotation rotation) 
    {
    for (int py = 0; py < size; py++) {
        for (int px = 0; px < size; px++) {

            // Apply rotation — remap (px, py) to source pixel index
            int srcIndex;
            switch (rotation) {
                case Rotation::R0:
                default:
                    srcIndex = py * size + px;
                    break;
                case Rotation::R90:
                    srcIndex = (size - 1 - px) * size + py;
                    break;
                case Rotation::R180:
                    srcIndex = (size - 1 - py) * size + (size - 1 - px);
                    break;
                case Rotation::R270:
                    srcIndex = px * size + (size - 1 - py);
                    break;
            }

            uint8_t paletteIndex = pixels[srcIndex];
            if (paletteIndex == 0) continue; // transparent pixel — skip

            // Calculate screen pixel position
            int drawX = x + px;
            int drawY = y + py;

            // Per-pixel bounds check: clip to gameRegion.
            // gameRegion/framebuffer compatibility is compile-time validated in WE_RenderCore.hpp.
            if (drawX < Settings.render.gameRegion.x1 || drawX >= Settings.render.gameRegion.x2) continue;
            if (drawY < Settings.render.gameRegion.y1 || drawY >= Settings.render.gameRegion.y2) continue;

            // Palette lookup (array bounds checked by resolver returns 0x0000 transparent if out of bounds)
            uint16_t color = ResolvePaletteColor(palette, paletteIndex);
            if (color == 0x0000) continue;

            // Byte swap if the display driver requires it (e.g. ST7735 expects RGB565 in BGR byte order)
            if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);
            // Write pixel to framebuffer
            m_framebuffer[drawY * Settings.render.screenWidth + drawX] = color;
        }
    }
}


// -------------------------------------------------------------
//  drawFillRectInternal
//  Fills a solid rectangle into the framebuffer.
//  Clips to screen bounds; no gameRegion restriction.
// -------------------------------------------------------------
void Renderer::drawFillRectInternal(int16_t x, int16_t y, uint8_t w, uint8_t h, uint16_t color) {
    if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);
    for (int py = 0; py < h; py++) {
        int drawY = y + py;
        if (drawY < 0 || drawY >= Settings.render.screenHeight) continue;
        for (int px = 0; px < w; px++) {
            int drawX = x + px;
            if (drawX < 0 || drawX >= Settings.render.screenWidth) continue;
            m_framebuffer[drawY * Settings.render.screenWidth + drawX] = color;
        }
    }
}


// -------------------------------------------------------------
//  drawLineInternal
//  Bresenham's integer line algorithm.
//  Clips each pixel to screen bounds before writing.
// -------------------------------------------------------------
void Renderer::drawLineInternal(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);

    int dx  =  (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy  = -((y2 > y1) ? (y2 - y1) : (y1 - y2));
    int sx  = (x1 < x2) ? 1 : -1;
    int sy  = (y1 < y2) ? 1 : -1;
    int err = dx + dy;

    int cx = x1, cy = y1;
    while (true) {
        if (cx >= 0 && cx < Settings.render.screenWidth && cy >= 0 && cy < Settings.render.screenHeight)
            m_framebuffer[cy * Settings.render.screenWidth + cx] = color;
        if (cx == x2 && cy == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; cx += sx; }
        if (e2 <= dx) { err += dx; cy += sy; }
    }
}


// -------------------------------------------------------------
//  drawCircleInternal
//  Midpoint circle algorithm.
//  Outline: 8-way symmetry, one pixel per symmetric point.
//  Filled: horizontal spans for each y offset pair.
// -------------------------------------------------------------
void Renderer::drawCircleInternal(int16_t cx, int16_t cy, uint8_t radius, uint16_t color, bool filled) {
    if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);

    auto plot = [&](int px, int py) {
        if (px >= 0 && px < Settings.render.screenWidth && py >= 0 && py < Settings.render.screenHeight)
            m_framebuffer[py * Settings.render.screenWidth + px] = color;
    };
    auto hspan = [&](int lx, int rx, int py) {
        if (py < 0 || py >= Settings.render.screenHeight) return;
        int x0 = (lx < 0) ? 0 : lx;
        int x1 = (rx >= Settings.render.screenWidth) ? Settings.render.screenWidth - 1 : rx;
        for (int px = x0; px <= x1; px++)
            m_framebuffer[py * Settings.render.screenWidth + px] = color;
    };

    int x = radius, y = 0, err = 0;
    while (x >= y) {
        if (filled) {
            hspan(cx - x, cx + x, cy + y);
            hspan(cx - x, cx + x, cy - y);
            hspan(cx - y, cx + y, cy + x);
            hspan(cx - y, cx + y, cy - x);
        } else {
            plot(cx + x, cy + y); plot(cx - x, cy + y);
            plot(cx + x, cy - y); plot(cx - x, cy - y);
            plot(cx + y, cy + x); plot(cx - y, cy + x);
            plot(cx + y, cy - x); plot(cx - y, cy - x);
        }
        y++;
        err += 2 * y + 1;
        if (2 * (err - x) + 1 > 0) { x--; err -= 2 * x + 1; }
    }
}


// -------------------------------------------------------------
//  drawTextRunInternal
//  Rasterises a null-terminated string using the 5x7 built-in font.
//  maxWidth clips total rendered width in pixels (0 = no clip).
//  Out-of-ASCII-range characters (< 32 or > 126) are skipped.
// -------------------------------------------------------------
void Renderer::drawTextRunInternal(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t maxWidth) {
    if (!text) return;
    if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);

    int16_t cursorX = x;
    for (const char* p = text; *p; p++) {
        uint8_t c = static_cast<uint8_t>(*p);
        if (c < 32 || c > 126) continue;
        if (maxWidth != 0 && (cursorX - x) + FONT_5x7_WIDTH > maxWidth) break;

        const uint8_t* glyph = FONT_5x7[c - 32];
        for (int col = 0; col < FONT_5x7_WIDTH; col++) {
            uint8_t colBits = glyph[col];
            for (int row = 0; row < FONT_5x7_HEIGHT; row++) {
                if (colBits & (1 << row)) {
                    int drawX = cursorX + col;
                    int drawY = y + row;
                    if (drawX >= 0 && drawX < Settings.render.screenWidth &&
                        drawY >= 0 && drawY < Settings.render.screenHeight)
                        m_framebuffer[drawY * Settings.render.screenWidth + drawX] = color;
                }
            }
        }
        cursorX += FONT_5x7_WIDTH + FONT_5x7_SPACING;
    }
}


// -------------------------------------------------------------
//  sortCommands
//  Insertion sort — O(N) on nearly-sorted input (expected case),
//  no heap allocation, safe on embedded targets.
//  Single key: sortKey (ascending) — layer in high byte, screenY in low byte.
// -------------------------------------------------------------
void Renderer::sortCommands() {
    for (int i = 1; i < m_commandCount; ++i) {
        DrawCommand key = m_commandBuffer[i];
        int j = i - 1;
        while (j >= 0 && m_commandBuffer[j].sortKey > key.sortKey) {
            m_commandBuffer[j + 1] = m_commandBuffer[j];
            j--;
        }
        m_commandBuffer[j + 1] = key;
    }
}


// -------------------------------------------------------------
//  executeCommands
//  Dispatches each buffered command to the appropriate draw call.
// -------------------------------------------------------------
void Renderer::executeCommands() {
    for (uint16_t i = 0; i < m_commandCount; ++i) {
        const DrawCommand& cmd = m_commandBuffer[i];
        switch (cmd.type) {
            case DrawCommandType::Sprite: {
                Rotation rot = cmdGetRotation(cmd.flags);
                drawSpriteInternal(cmd.x, cmd.y,
                                   cmd.sprite.pixels,
                                   cmd.sprite.palette,
                                   cmd.sprite.size,
                                   rot);
                m_diagnostics.commandsExecuted++;
                break;
            }
            case DrawCommandType::FillRect: {
                drawFillRectInternal(cmd.x, cmd.y,
                                     cmd.fillRect.w, cmd.fillRect.h, cmd.fillRect.color);
                m_diagnostics.commandsExecuted++;
                break;
            }
            case DrawCommandType::Line: {
                drawLineInternal(cmd.x, cmd.y,
                                 cmd.line.x2, cmd.line.y2, cmd.line.color);
                m_diagnostics.commandsExecuted++;
                break;
            }
            case DrawCommandType::Circle: {
                drawCircleInternal(cmd.x, cmd.y,
                                   cmd.circle.radius, cmd.circle.color,
                                   cmd.circle.filled != 0);
                m_diagnostics.commandsExecuted++;
                break;
            }
            case DrawCommandType::TextRun: {
                drawTextRunInternal(cmd.x, cmd.y,
                                    cmd.textRun.text, cmd.textRun.color,
                                    cmd.textRun.maxWidth);
                m_diagnostics.commandsExecuted++;
                break;
            }
            default:
                ESP_LOGW("Renderer", "Unknown DrawCommandType — command skipped");
                break;
        }
    }
}


// -------------------------------------------------------------
//  beginFrame
//  Clears the framebuffer to the background colour.
//  Called once at the start of each frame before component ticks.
// -------------------------------------------------------------
void Renderer::beginFrame() {
    // Reset diagnostics for this frame
    m_diagnostics.commandsSubmitted = 0;
    m_diagnostics.commandsDropped   = 0;
    m_diagnostics.commandsExecuted  = 0;

    // Clear framebuffer to background color if enabled in settings
    if constexpr (Settings.render.cleanFramebufferEachFrame) {
        constexpr uint16_t bg         = Settings.render.defaultBackgroundPixel;
        constexpr uint16_t bgSwapped  = (bg >> 8) | (bg << 8);
        const uint16_t fill = m_driver->requiresByteSwap ? bgSwapped : bg;
        std::fill( m_framebuffer, m_framebuffer + (Settings.render.screenWidth * Settings.render.screenHeight), fill );
    }
}


// -------------------------------------------------------------
//  executeAndFlush
//  Two-pass render: world pass then UI pass. Each pass sorts,
//  executes, updates peakCommandCount, and clears independently.
//  Flush is always full-screen.
// -------------------------------------------------------------
void Renderer::executeAndFlush() {
    // --- World pass ---
    sortCommands();
    executeCommands();
    m_diagnostics.peakCommandCount =
        (m_commandCount > m_diagnostics.peakCommandCount)
        ? m_commandCount : m_diagnostics.peakCommandCount;
    clearCommands();

    // --- UI pass ---
    UI().render();  // UI elements submit FillRect/Line/Circle/TextRun commands
    sortCommands();
    executeCommands();
    m_diagnostics.peakCommandCount =
        (m_commandCount > m_diagnostics.peakCommandCount)
        ? m_commandCount : m_diagnostics.peakCommandCount;
    clearCommands();

    // --- Flush: always full screen ---
    m_driver->flush(m_framebuffer, 0, 0,
                    m_driver->screenWidth, m_driver->screenHeight);
}


// -------------------------------------------------------------
//  render
//  Master render function called every frame by WolfEngine.
// -------------------------------------------------------------
void Renderer::render() {
    beginFrame();
    executeAndFlush();
}
