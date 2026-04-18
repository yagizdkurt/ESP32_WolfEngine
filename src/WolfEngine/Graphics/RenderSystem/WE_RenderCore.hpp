#pragma once

// =============================================================
//  WE_RenderCore.hpp
//  WolfEngine Render Core
//
//  The Renderer manages the framebuffer and a per-frame DrawCommand
//  buffer. SpriteRenderer components submit DrawCommands during
//  the component-tick phase. At the end of the frame, the renderer
//  sorts by layer+sortKey, executes each command, and flushes to
//  the display driver.
//
//  RENDER LOOP (called every frame by WolfEngine)
//  -----------------------------------------------
//  1. beginFrame()      — clear framebuffer
//  2. [componentTick]   — SpriteRenderers submit DrawCommands
//  3. executeAndFlush() — sort, execute, UI, flush, reset buffer
//
//  SCREEN REGIONS
//  --------------
//  Game region — rows 0 to RENDER_UI_START_ROW-1 (128x128)
//                Flushed every frame.
//  UI region   — rows RENDER_UI_START_ROW to screenHeight-1 (128x32)
//                Only flushed when markUIDirty() has been called.
//                Retains last drawn content until marked dirty again.
// =============================================================


// =============== Engine Settings ================
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Settings/WE_PINDEFS.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_DrawCommand.hpp"

// =============== Driver Selection ================
#include "WolfEngine/Drivers/DisplayDrivers/WE_Display_Driver.hpp"
#if defined(DISPLAY_ST7735)
    #include "esp_lcd_st7735.h"
    #include "WolfEngine/Drivers/DisplayDrivers/WE_Display_ST7735.hpp"
#elif defined(DISPLAY_SDL)
    #include "Display_SDL.h"
#endif


class Renderer {
public:
    uint16_t* getCanvas() { return m_framebuffer; }
    static constexpr int SCREEN_WIDTH  = RENDER_SCREEN_WIDTH;
    static constexpr int SCREEN_HEIGHT = RENDER_SCREEN_HEIGHT;

    bool submitDrawCommand(const DrawCommand& cmd);
    const FrameDiagnostics& getDiagnostics() const { return m_diagnostics; }

private:
    Renderer(DisplayDriver* driver) : m_driver(driver) { }

    void initialize();
    void render();

    void beginFrame();
    void executeAndFlush();
    void sortCommands();
    void executeCommands();
    void drawSpriteInternal(int16_t x, int16_t y,
                            const uint8_t*  pixels,
                            const uint16_t* palette,
                            int size, Rotation rotation);

    DisplayDriver* m_driver;
    uint16_t m_framebuffer[RENDER_SCREEN_WIDTH * RENDER_SCREEN_HEIGHT];

    DrawCommand      m_commandBuffer[MAX_DRAW_COMMANDS];
    uint16_t         m_commandCount = 0;
    FrameDiagnostics m_diagnostics  = {};

    friend class WolfEngine;
};
