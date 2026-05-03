#include "WolfEngine/GameFlow/WE_GameFlowManager.hpp"
#include "WolfEngine/Utilities/Debug/WE_Debug.hpp"

void GameFlowManager::init() {
    m_top    = -1;
    pushState(GameFlowState::Running);
}

void GameFlowManager::pushState(GameFlowState state) {
    WE_ASSERT(m_top < Settings.limits.maxFlowStackDepth - 1, "GameFlowManager: state stack overflow");
    m_stateStack[++m_top] = state;
    m_activeMask = maskForState(state);
}

void GameFlowManager::popState() {
    if (m_top <= 0) return;  // Never pop the root Running state
    --m_top;
    m_activeMask = maskForState(m_stateStack[m_top]);
}

GameFlowState GameFlowManager::currentState() const {
    return m_stateStack[m_top];
}

uint16_t GameFlowManager::activeMask() const {
    return m_activeMask;
}

bool GameFlowManager::isLayerActive(uint16_t layer) const {
    return (m_activeMask & layer) != 0;
}

uint16_t GameFlowManager::maskForState(GameFlowState state) const {
    switch (state) {
        case GameFlowState::Running:  return Settings.gameFlow.masks.running;
        case GameFlowState::Dialogue: return Settings.gameFlow.masks.dialogue;
        case GameFlowState::Menu:     return Settings.gameFlow.masks.menu;
        case GameFlowState::Cutscene: return Settings.gameFlow.masks.cutscene;
        case GameFlowState::Custom1:  return Settings.gameFlow.masks.custom1;
        case GameFlowState::Custom2:  return Settings.gameFlow.masks.custom2;
        case GameFlowState::Custom3:  return Settings.gameFlow.masks.custom3;
        case GameFlowState::Custom4:  return Settings.gameFlow.masks.custom4;
        default:                      return 0;
    }
}
