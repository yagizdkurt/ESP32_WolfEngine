#pragma once
#include <atomic>
//============================================================================================
//                        WOLF ENGINE CORE HEADER FILE
//============================================================================================

// =============== ENGINE INCLUDES ================
// ======= Settings ==========
#include "WolfEngine/Settings/WE_Settings.hpp"

// ======= Registries ========
#include "WolfEngine/GameObjectSystem/WE_GORegistry.hpp"
#include "WolfEngine/Modules/WE_ModuleSystem.hpp"

// ======= SUBSYSTEMS ========
#include "WolfEngine/Utilities/WE_Timer.hpp"
#include "WolfEngine/InputSystem/WE_InputManager.hpp"
#include "WolfEngine/Sound/WE_SoundManager.hpp"
#include "WolfEngine/InputSystem/WE_InputManager.hpp"

// ======= Component System ========
#include "WolfEngine/ComponentSystem/Components/WE_Components.hpp"

// ======= Graphics System ========
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"


class WolfEngine final {
public:
    static WolfEngine& getInstance() { static WolfEngine instance; return instance; }

    /**
     * @brief Initializes all engine subsystems. Must be called once before StartGame().
     */
    void StartEngine();

    /**
     * @brief Starts the game loop. Blocks until RequestQuit() is called.
     * @note Must be called after StartEngine().
     */
    void StartGame();

    /**
     * @brief Signals the game loop to exit. Safe to call from any thread.
     */
    void RequestQuit() { m_isRunning.store(false, std::memory_order_relaxed); }

    /**
     * @brief Returns true while the game loop is running. Safe to call from any thread.
     */
    bool IsRunning() { return m_isRunning.load(std::memory_order_relaxed); }
    
    Renderer m_renderer;
    Camera m_Camera;
    InputManager m_InputManager;
    UIManager m_UIManager;
    SoundManager m_SoundManager;

private:
    std::atomic<bool> m_isRunning{false};
    GameObjectRegistry m_GameObjectRegistry = {};
    int64_t lastFrameTime = 0;
    
    WolfEngine() :
    m_renderer(GetDriver()),
    m_Camera(),
    m_InputManager(),
    m_SoundManager()
    {}

    void gameLoop();
    void gameTick();

    friend class GameObject;
};

// Global accessors for convenience.
inline WolfEngine& Engine()                 { return WolfEngine::getInstance(); }
inline Camera& MainCamera()                 { return WolfEngine::getInstance().m_Camera; }
inline InputManager& Input()                { return WolfEngine::getInstance().m_InputManager; }
inline UIManager& UI()                      { return WolfEngine::getInstance().m_UIManager; }
inline SoundManager& Sound()                { return WolfEngine::getInstance().m_SoundManager; }
inline Renderer& RenderSys()                { return WolfEngine::getInstance().m_renderer; }
