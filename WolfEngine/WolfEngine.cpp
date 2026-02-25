#include "WolfEngine.hpp"
#include "esp_timer.h"

void WolfEngine::StartEngine() {
    renderer.initialize();
    m_Camera.initialize(RENDER_SCREEN_WIDTH, RENDER_UI_START_ROW);
    m_UIManager.initialize(renderer.m_framebuffer, RENDER_SCREEN_WIDTH, RENDER_SCREEN_HEIGHT);
    m_SoundManager.Initialize();
    BaseUIElement **defaultUI = new BaseUIElement*[1] { nullptr };
    UI().setElements(defaultUI);
}

void WolfEngine::StartGame() {
    m_isRunning = true;
    int64_t lastFrameTime = esp_timer_get_time();
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj) obj->callStart();

    while (true) {
        int64_t now     = esp_timer_get_time();
        int64_t elapsed = now - lastFrameTime;

        m_SoundManager.update();

        // Only tick if enough time has passed for a 30fps frame
        if (elapsed >= TARGET_FRAME_TIME_US) {
            lastFrameTime = now;
            gameTick();
        }
    }
}

// Main Game Loop Tick - called every frame
void WolfEngine::gameTick() {
    // Update input states first
    m_InputManager.tick();

    // Update logic of each object
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) {
        if (obj && obj->isActive) {
            obj->Update();
        }
    }

    // Update camera after game logic so follow targets are at their new position
    m_Camera.followTick();

    // Render the scene
    renderer.render();
}