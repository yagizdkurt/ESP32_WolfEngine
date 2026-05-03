#include "WolfEngine.hpp"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "WolfEngine/Utilities/WE_I2C.hpp"
#include "WolfEngine/Utilities/Debug/WE_Debug.hpp"

void WolfEngine::Shutdown() {
#if WE_DUAL_CORE_RENDER
    m_renderer.renderShutDown();
#endif
    I2CManager::end();
}

// =================== Initialization Steps ===================
void WolfEngine::StartEngine() {
    initializeDrivers();    // Driver initialization
    initializeSubsystems(); // Engine CORE subsystem initialization
    initializeModules();    // Module initialization
}

void WolfEngine::initializeDrivers() {
    esp_err_t err = I2CManager::begin();
    if (err != ESP_OK) {
        WE_LOGE("Boot", "I2CManager::begin() failed: %s", esp_err_to_name(err));
        abort();
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // bus must stabilize before first device access
    WE_LOGI("Boot", "I2C driver ready");
    m_InputManager.HW_init(); // GPIO + ADC — no I2C dependency, safe here
    vTaskDelay(pdMS_TO_TICKS(50));
}

void WolfEngine::initializeSubsystems() {
    m_renderer.initialize();
    m_Camera.initialize();
    m_UIManager.initialize(m_renderer.m_framebuffer);
    m_SoundManager.Initialize();
    m_InputManager.init();
    m_flowManager.init();
    WE_LOGI("Boot", "Subsystems ready");
}

void WolfEngine::initializeModules() {
    ModuleSystem::InitAll();
}

void WolfEngine::StartGame() {
    m_isRunning = true;
    lastFrameTime = esp_timer_get_time();
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj) obj->callStart();

    // Main game loop
    while (IsRunning()) gameLoop();

    // Shutdown sequence
    Shutdown();
}

// =================== Main Game Loop ===================
// The main game loop is divided into several phases to control the order of operations.

void WolfEngine::gameLoop() {
    int64_t now     = esp_timer_get_time();
    int64_t elapsed = now - lastFrameTime;

    m_SoundManager.update();
    ModuleSystem::FreeUpdate();

    if (elapsed >= Settings.render.targetFrameTimeUs) { //30fps frame
        lastFrameTime = now;
        gameTick();
    } else {
        vTaskDelay(1); // yield to idle task; keeps TWDT fed between frames
    }
}

// Main Game Loop Tick - called every frame
void WolfEngine::gameTick() {
    auto tickStart = WE_DiagBegin();

    // ---- Pending Start Drain ----
    for (uint16_t i = 0; i < m_GameObjectRegistry.pendingStartCount; i++)
        if (m_GameObjectRegistry.pendingStart[i])
            m_GameObjectRegistry.pendingStart[i]->callStart();
    m_GameObjectRegistry.pendingStartCount = 0;

    // ---- Early Phase ----
    m_InputManager.tick(); // Update input states first

    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // EarlyUpdate for game objects before main logic
        if (obj && (Flow().isLayerActive(obj->m_updateLayer) && obj->isUpdatable())) obj->EarlyUpdate();

    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // Early tick for components before game logic
        if (obj && (Flow().isLayerActive(obj->m_updateLayer) && obj->isUpdatable())) obj->earlyComponentTick();

    ModuleSystem::EarlyUpdate(); // Early update for modules before game logic

    // ---- Main Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // Main update for game objects
        if (obj && (Flow().isLayerActive(obj->m_updateLayer) && obj->isUpdatable())) obj->Update();

    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // Main tick for components
        if (obj && (Flow().isLayerActive(obj->m_updateLayer) && obj->isUpdatable())) obj->componentTick();

    ModuleSystem::Update(); // Main update for modules after game logic

    // ---- Late Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // LateUpdate for game objects after main logic
        if (obj && (Flow().isLayerActive(obj->m_updateLayer) && obj->isUpdatable())) obj->LateUpdate();

    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // Late tick for components after main logic
        if (obj && (Flow().isLayerActive(obj->m_updateLayer) && obj->isUpdatable())) obj->lateComponentTick();

    ModuleSystem::LateUpdate(); // Late update for modules after game logic

    if (Flow().isLayerActive(UL_UPDATE_GAMEPLAY)) m_Camera.followTick(); // Update camera after game logic so follow targets are at their new position

    // ---- End Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) // PreRender tick for components before rendering
        if (obj && obj->isUpdatable()) obj->preRenderComponentTick();

    ModuleSystem::PreRender(); // PreRender for modules before rendering

    // Rendering Starts here — diagnostics measured separately to exclude module PreRender time
    auto renderStart = WE_DiagBegin();
    m_engineDiag.updatePhaseUs = WE_DiagElapsedUs(tickStart);

    m_renderer.render(); // Render the scene

    m_engineDiag.renderPhaseUs = WE_DiagElapsedUs(renderStart);
    m_engineDiag.frameTotalUs  = WE_DiagElapsedUs(tickStart);

    uint32_t fpsElapsed = static_cast<uint32_t>(esp_timer_get_time() - m_fpsWindowStart);
    m_fpsFrameCount++;
    if (fpsElapsed >= 1000000u) {
        m_engineDiag.fpsAvg1s = static_cast<uint16_t>(
            (m_fpsFrameCount * 1000000u) / fpsElapsed);
        m_fpsFrameCount  = 0;
        m_fpsWindowStart = esp_timer_get_time();
    }

    if (++m_diagFrameCount >= WE_DIAG_LOG_INTERVAL_FRAMES) {
        WE_LOGI("DIAG", "frame=%luus update=%luus render=%luus fps=%u",
            m_engineDiag.frameTotalUs,
            m_engineDiag.updatePhaseUs,
            m_engineDiag.renderPhaseUs,
            m_engineDiag.fpsAvg1s);
        m_diagFrameCount = 0;
    }

    WETime::incrementFrameCount(); // Increment frame count at the end of the tick so it reflects completed frames

    // ---- Destroy Drain ----
    for (uint16_t i = 0; i < m_GameObjectRegistry.pendingDestroyCount; i++)
        delete m_GameObjectRegistry.pendingDestroy[i];
    m_GameObjectRegistry.pendingDestroyCount = 0;
}