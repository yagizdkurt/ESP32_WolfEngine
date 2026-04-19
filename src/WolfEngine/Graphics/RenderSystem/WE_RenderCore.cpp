#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/WolfEngine.hpp"
#include "esp_log.h"

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

void Renderer::initialize() { m_driver->initialize(); }


// -------------------------------------------------------------
//  submitDrawCommand
//  Called by SpriteRenderer (and future components) during the
//  component-tick phase to enqueue a draw operation for this frame.
//  Overflow is loud: dropped commands are counted and logged.
// -------------------------------------------------------------
bool Renderer::submitDrawCommand(const DrawCommand& cmd) {
    if (m_commandCount >= MAX_DRAW_COMMANDS) {
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
                                   const uint8_t*  pixels,
                                   const uint16_t* palette,
                                   int size, Rotation rotation) {
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

            // Skip transparent pixels (index 0)
            uint8_t paletteIndex = pixels[srcIndex];
            if (paletteIndex == 0) continue;

            // Calculate screen pixel position
            int drawX = x + px;
            int drawY = y + py;

            // Per-pixel bounds check — clip to game region
            if (drawX < RENDER_SETTINGS.gameRegion.x1 || drawX >= RENDER_SETTINGS.gameRegion.x2) continue;
            if (drawY < RENDER_SETTINGS.gameRegion.y1 || drawY >= RENDER_SETTINGS.gameRegion.y2) continue;

            // Palette lookup and write to framebuffer
            uint16_t color = palette[paletteIndex];
            if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);
            m_framebuffer[drawY * RENDER_SCREEN_WIDTH + drawX] = color;
        }
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
            // future types added here
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
    if constexpr (RENDER_SETTINGS.cleanFramebufferEachFrame) {
        constexpr uint16_t bg         = RENDER_SETTINGS.defaultBackgroundPixel;
        constexpr uint16_t bgSwapped  = (bg >> 8) | (bg << 8);
        const uint16_t fill = m_driver->requiresByteSwap ? bgSwapped : bg;
        std::fill( m_framebuffer, m_framebuffer + m_driver->screenWidth * m_driver->screenHeight, fill );
    }
}


// -------------------------------------------------------------
//  executeAndFlush
//  Sorts and executes all buffered DrawCommands, renders UI if
//  dirty, flushes the framebuffer to the display, then resets
//  the command buffer and updates the peak diagnostic.
// -------------------------------------------------------------
void Renderer::executeAndFlush() {
    sortCommands();
    executeCommands();

    m_diagnostics.peakCommandCount = (m_commandCount > m_diagnostics.peakCommandCount) ? m_commandCount : m_diagnostics.peakCommandCount;
    m_commandCount = 0;
    // commandsSubmitted / commandsDropped / commandsExecuted are running lifetime totals — not reset here.

    // UI region — only render and flush when dirty
    bool uiDirty = UI().isDirty();
    if (uiDirty) UI().render();

    // Flush to display
    if (uiDirty) {
        m_driver->flush(m_framebuffer, 0, 0,
                        m_driver->screenWidth, m_driver->screenHeight);
    } else {
        m_driver->flush(m_framebuffer,
                        RENDER_SETTINGS.gameRegion.x1, RENDER_SETTINGS.gameRegion.y1,
                        RENDER_SETTINGS.gameRegion.x2, RENDER_SETTINGS.gameRegion.y2);
    }
}


// -------------------------------------------------------------
//  render
//  Master render function called every frame by WolfEngine.
// -------------------------------------------------------------
void Renderer::render() {
    beginFrame();
    executeAndFlush();
}
