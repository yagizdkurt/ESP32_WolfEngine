#include "WolfEngine.hpp"
#include "esp_timer.h"
#include "WolfEngine/Utilities/WE_I2C.hpp"

void WolfEngine::StartEngine() {
    // Driver initialization
    I2CManager::begin();

    // Engine CORE subsystem initialization
    m_InputManager.init();
    m_renderer.initialize();
    m_Camera.initialize();
    m_UIManager.initialize(m_renderer.m_framebuffer);
    m_SoundManager.Initialize();

    // Engine Modules initialization
    ModuleRegistry::InitAll();

    // default initializations for convenience
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

    // Update engine modules
    ModuleRegistry::UpdateAll();

    // Update component ticks of each object before Update() so that components can modify object state before game logic runs
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->componentTick();

    // Update logic of each object
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->Update();

    // Evaluate collider interactions after movement/logic updates.
    m_ColliderManager.tick();

    // Update camera after game logic so follow targets are at their new position
    m_Camera.followTick();

    // Render the scene
    m_renderer.render();
}
