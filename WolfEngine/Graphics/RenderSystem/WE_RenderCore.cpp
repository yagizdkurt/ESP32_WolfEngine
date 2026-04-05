#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WolfEngine/Graphics/SpriteSystem/WE_SpriteData.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/WolfEngine.hpp"
#include <string.h>

void Renderer::initialize() {
    m_driver->initialize();
}

// -------------------------------------------------------------
//  registerSprite
//  Finds the first empty slot in the given layer and stores
//  the sprite pointer. Called automatically by Sprite constructor
//  via WolfEngine::getInstance().renderer.registerSprite()
// -------------------------------------------------------------
void Renderer::registerSprite(SpriteRenderer* sprite, int layer) {
    for (int i = 0; i < MAX_GAME_OBJECTS; i++) {
        if (m_layers[layer][i] == nullptr) {
            m_layers[layer][i] = sprite;
            return;
        }
    }
}

// -------------------------------------------------------------
//  unregisterSprite
//  Finds and nulls the sprite pointer in the given layer.
//  Called automatically by Sprite destructor
//  via WolfEngine::getInstance().renderer.unregisterSprite()
// -------------------------------------------------------------
void Renderer::unregisterSprite(SpriteRenderer* sprite, int layer) {
    for (int i = 0; i < MAX_GAME_OBJECTS; i++) {
        if (m_layers[layer][i] == sprite) {
            m_layers[layer][i] = nullptr;
            return;
        }
    }
}


// -------------------------------------------------------------
//  drawSprite
//  Internal helper — writes a single sprite into the framebuffer.
//  Handles rotation index math, transparency, and per-pixel
//  bounds checking to prevent out-of-bounds framebuffer writes.
// -------------------------------------------------------------
void Renderer::drawSprite(const SpriteData& data, int screenX, int screenY) {
    const int size = data.size;
    for (int py = 0; py < size; py++) {
        for (int px = 0; px < size; px++) {

            // Apply rotation — remap (px, py) to source pixel index
            int srcIndex;
            switch (data.rotation) {
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
            uint8_t paletteIndex = data.pixels[srcIndex];
            if (paletteIndex == 0) continue;

            // Calculate screen pixel position
            int drawX = screenX + px;
            int drawY = screenY + py;

            // Per-pixel bounds check — clip to game region
            if (drawX < RENDER_SETTINGS.gameRegion.x1 || drawX >= RENDER_SETTINGS.gameRegion.x2) continue;
            if (drawY < RENDER_SETTINGS.gameRegion.y1 || drawY >= RENDER_SETTINGS.gameRegion.y2) continue;

            // Palette lookup and write to framebuffer
            uint16_t color = data.palette[paletteIndex];
            if (m_driver->requiresByteSwap) color = (color >> 8) | (color << 8);
            m_framebuffer[drawY * RENDER_SCREEN_WIDTH + drawX] = color;
        }
    }
}


// -------------------------------------------------------------
//  drawGame
//  Draws all game layers (LAYER_BACKGROUND through LAYER_FX)
//  into the game region
//  World positions are converted to screen via camera.
// -------------------------------------------------------------
void Renderer::drawGame() {
    for (int layer = 0; layer < static_cast<int>(RenderLayer::MAX_LAYERS); layer++) {
        for (int i = 0; i < MAX_GAME_OBJECTS; i++) {

            SpriteRenderer* sprite = m_layers[layer][i];
            if (!sprite) continue;

            SpriteData data = sprite->getRenderData();
            if (!data.visible) continue;

            // Cull sprites fully outside the game region
            float margin = data.size * 0.5f;
            if (!MainCamera().isVisible({(float)data.x, (float)data.y}, margin))
                continue;

            // Convert world position to screen position via camera
            Vec2 screenPos = MainCamera().worldToScreen({(float)data.x, (float)data.y});

            // Center the sprite on its position
            int drawX = (int)screenPos.x - (data.size / 2);
            int drawY = (int)screenPos.y - (data.size / 2);

            drawSprite(data, drawX, drawY);
        }
    }
}

// -------------------------------------------------------------
//  render
//  Master render function called every frame by WolfEngine.
// -------------------------------------------------------------
void Renderer::render() {
    
    if constexpr (RENDER_SETTINGS.cleanFramebufferEachFrame) {
    uint16_t bg = RENDER_SETTINGS.defaultBackgroundPixel;
    if (m_driver->requiresByteSwap) bg = (bg >> 8) | (bg << 8);
    std::fill(m_framebuffer, m_framebuffer + m_driver->screenWidth * m_driver->screenHeight, bg);
    }
    
    if constexpr (RENDER_SETTINGS.spriteSystemEnabled) drawGame(); // 2. Draw game region

    // 3. Draw UI region — only if dirty
    bool uiDirty = UI().isDirty();
    if (uiDirty) UI().render();

    // 4. Flush to display
    if (uiDirty) { m_driver->flush(m_framebuffer, 0, 0, m_driver->screenWidth, m_driver->screenHeight); } 
    else { m_driver->flush(m_framebuffer, RENDER_SETTINGS.gameRegion.x1, RENDER_SETTINGS.gameRegion.y1, RENDER_SETTINGS.gameRegion.x2, RENDER_SETTINGS.gameRegion.y2); }
}