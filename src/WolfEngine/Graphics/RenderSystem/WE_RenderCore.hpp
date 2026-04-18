#pragma once

// =============================================================
//  WE_RenderCore.hpp
//  WolfEngine Render Core
//
//  The Renderer manages the framebuffer, sprite layer table,
//  and drives the full draw and flush cycle every frame.
//  Owned and driven by WolfEngine. Sprite components register
//  and unregister themselves via WolfEngine::getInstance().renderer
//
//  RENDER LOOP (called every frame by WolfEngine)
//  -----------------------------------------------
//  1. Clear framebuffer to DEFAULT_BACKGROUND_PIXEL
//  2. drawGame() — iterate layers LAYER_BACKGROUND to LAYER_FX
//  3. drawUI()   — only if m_uiDirty, iterate LAYER_UI
//  4. Flush      — full screen if UI was dirty, game region only otherwise
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
#include "WolfEngine/Settings/WE_Layers.hpp"

// =============== Sprite Component ================
#include "WolfEngine/ComponentSystem/Components/WE_Comp_SpriteRenderer.hpp"

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
    static constexpr int SCREEN_WIDTH = RENDER_SCREEN_WIDTH;
    static constexpr int SCREEN_HEIGHT = RENDER_SCREEN_HEIGHT;
private:
    Renderer(DisplayDriver* driver) : m_driver(driver) { }

    void initialize();
    void render();
    void drawGame();
    void drawSprite(const SpriteData& data, int screenX, int screenY);

    void registerSprite  (SpriteRenderer* sprite, int layer);
    void unregisterSprite(SpriteRenderer* sprite, int layer);

    DisplayDriver* m_driver;
    SpriteRenderer* m_layers[static_cast<uint16_t>(RenderLayer::MAX_LAYERS)][MAX_GAME_OBJECTS] = { };
    uint16_t m_framebuffer[RENDER_SCREEN_WIDTH * RENDER_SCREEN_HEIGHT];

    friend class WolfEngine;
    friend class SpriteRenderer;

};