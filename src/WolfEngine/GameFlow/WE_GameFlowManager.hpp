#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"

// =============================================================
//  GameFlowManager
//  Plain subsystem class (not IModule) owned by WolfEngine.
//  Manages a push-down stack of GameFlowStates and derives an
//  active UpdateLayer bitmask from the current state.
//  No heap allocation; stack depth is fixed at 8.
// =============================================================
class GameFlowManager {
public:
    void          init();
    void          pushState(GameFlowState state);
    void          popState();
    GameFlowState currentState() const;
    uint16_t      activeMask()   const;
    bool          isLayerActive(uint16_t layer) const;

private:
    GameFlowState         m_stateStack[Settings.limits.maxFlowStackDepth];
    int8_t                m_top        = -1;
    uint16_t              m_activeMask = 0;

    uint16_t maskForState(GameFlowState state) const;
};
