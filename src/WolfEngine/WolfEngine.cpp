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

    // Module initialization
    ModuleSystem::InitAll();

    // default initializations for convenience
    BaseUIElement **defaultUI = new BaseUIElement*[1] { nullptr };
    UI().setElements(defaultUI);
}

void WolfEngine::StartGame() {
    m_isRunning = true;
    lastFrameTime = esp_timer_get_time();
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj) obj->callStart();

    // Main game loop
    while (IsRunning()) gameLoop();
}

void WolfEngine::gameLoop() {
    int64_t now     = esp_timer_get_time();
    int64_t elapsed = now - lastFrameTime;

    m_SoundManager.update();
    ModuleSystem::FreeUpdate();

    if (elapsed >= Settings.render.targetFrameTimeUs) { //30fps frame
        lastFrameTime = now;
        gameTick();
    }
}

// Main Game Loop Tick - called every frame
void WolfEngine::gameTick() {
    // ---- Early Phase ----
    m_InputManager.tick(); // Update input states first
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->EarlyUpdate(); // EarlyUpdate for game objects before main logic
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->earlyComponentTick(); // Early tick for components before game logic
    ModuleSystem::EarlyUpdate(); // Early update for modules before game logic

    // ---- Main Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->Update(); // Main update for game objects
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->componentTick(); // Main tick for components
    ModuleSystem::Update(); // Main update for modules after game logic

    // ---- Late Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->LateUpdate(); // LateUpdate for game objects after main logic
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->lateComponentTick(); // Late tick for components after main logic
    ModuleSystem::LateUpdate(); // Late update for modules after game logic
    m_Camera.followTick(); // Update camera after game logic so follow targets are at their new position

    // ---- End Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isActive) obj->preRenderComponentTick(); // PreRender tick for components before rendering
    ModuleSystem::PreRender(); // PreRender for modules before rendering
    m_renderer.render(); // Render the scene
    WETime::incrementFrameCount(); // Increment frame count at the end of the tick so it reflects completed frames
}
