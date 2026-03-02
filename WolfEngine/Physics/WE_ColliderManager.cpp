#include "WE_ColliderManager.hpp"


void ColliderManager::registerCollider(Collider* collider) {
    for (int i = 0; i < MAX_GAME_OBJECTS * 3; ++i) {
        if (m_colliders[i] == nullptr) {
            m_colliders[i] = collider;
            return;
        }
    }
}

void ColliderManager::unregisterCollider(Collider* collider) {
    for (int i = 0; i < MAX_GAME_OBJECTS * 3; ++i) {
        if (m_colliders[i] == collider) {
            m_colliders[i] = nullptr;
            return;
        }
    }
}