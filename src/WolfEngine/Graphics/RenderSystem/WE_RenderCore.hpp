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
#include "WolfEngine/Graphics/RenderSystem/WE_DrawCommand.hpp"

// =============== Driver Selection ================
#include "WolfEngine/Drivers/DisplayDrivers/WE_Display_Driver.hpp"
#if defined(DISPLAY_ST7735)
    #include "esp_lcd_st7735.h"
    #include "WolfEngine/Drivers/DisplayDrivers/WE_Display_ST7735.hpp"
#elif defined(DISPLAY_SDL)
    #include "Display_SDL.h"
#endif

// gameRegion is a compile-time invariant for sprite safety:
// it must be a valid half-open rectangle fully contained in the framebuffer.
static_assert(Settings.render.gameRegion.x1 >= 0, "Settings.render.gameRegion.x1 must be >= 0");
static_assert(Settings.render.gameRegion.y1 >= 0, "Settings.render.gameRegion.y1 must be >= 0");
static_assert(Settings.render.gameRegion.x1 < Settings.render.gameRegion.x2, "Settings.render.gameRegion must satisfy x1 < x2");
static_assert(Settings.render.gameRegion.y1 < Settings.render.gameRegion.y2, "Settings.render.gameRegion must satisfy y1 < y2");
static_assert(Settings.render.gameRegion.x2 <= Settings.render.screenWidth, "Settings.render.gameRegion.x2 must be <= Settings.render.screenWidth");
static_assert(Settings.render.gameRegion.y2 <= Settings.render.screenHeight, "Settings.render.gameRegion.y2 must be <= Settings.render.screenHeight");


class Renderer {
public:
    uint16_t* getCanvas() { return m_framebuffer; }
    static constexpr int SCREEN_WIDTH  = Settings.render.screenWidth;
    static constexpr int SCREEN_HEIGHT = Settings.render.screenHeight;

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
    void drawSpriteInternal(int16_t x, int16_t y, const uint8_t*  pixels,
        const uint16_t* palette, int size, Rotation rotation);
        
    void clearCommands();
    void drawFillRectInternal(int16_t x, int16_t y, uint8_t w, uint8_t h, uint16_t color);
    void drawLineInternal(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawCircleInternal(int16_t cx, int16_t cy, uint8_t radius, uint16_t color, bool filled);
    void drawTextRunInternal(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t maxWidth);

    DisplayDriver* m_driver;
    uint16_t m_framebuffer[Settings.render.screenWidth * Settings.render.screenHeight];

    DrawCommand      m_commandBuffer[Settings.render.maxDrawCommands];
    uint16_t         m_commandCount = 0;
    FrameDiagnostics m_diagnostics  = {};

    friend class WolfEngine;
};
