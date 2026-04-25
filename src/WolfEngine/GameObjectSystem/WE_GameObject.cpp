#include "WE_GameObject.hpp"
#include "WolfEngine/WolfEngine.hpp"

bool GameObject::CreateObject() {
    GameObjectRegistry& reg = Engine().m_GameObjectRegistry;
    if (reg.count >= Settings.limits.maxGameObjects) return false;
    for (int i = 0; i < Settings.limits.maxGameObjects; i++) {
        if (reg.gameObjects[i] == nullptr) {
            reg.gameObjects[i] = this;
            reg.count++;
            id = i;
            if (Engine().IsRunning()) {
                reg.pendingStart[reg.pendingStartCount++] = this;
            }
            return true;
        }
    }
    return false; // Didnt find an empty slot. For whatever reason.
}

GameObject::GameObject() {}
GameObject::~GameObject() {}

void GameObject::DestroyGameObject(GameObject *gameObject) {
    if (gameObject == nullptr) return;
    GameObjectRegistry& reg = Engine().m_GameObjectRegistry;
    if (gameObject->id >= Settings.limits.maxGameObjects) return;
    if (reg.gameObjects[gameObject->id] != gameObject) return;
    gameObject->isDead = true;
    reg.gameObjects[gameObject->id] = nullptr;
    reg.count--;
    reg.pendingDestroy[reg.pendingDestroyCount++] = gameObject;
}


void GameObject::callStart() {
    if (!hasStarted) {
        for (int i = 0; i < m_componentCount; i++) if (m_components[i]) {
            m_components[i]->onReferenceCollection();
            m_components[i]->onStart();
        }
        Start();
        hasStarted = true;
    }
}

void GameObject::registerComponent(Component* comp) {
    if (m_componentCount >= MAX_COMPONENTS_PER_OBJECT) return;
    m_components[m_componentCount++] = comp;
}

void GameObject::earlyComponentTick() {
    for (int i = 0; i < m_componentCount; i++) 
        if (m_components[i] && m_components[i]->earlyTickEnabled) 
            m_components[i]->earlyTick();
}

void GameObject::componentTick() {
    for (int i = 0; i < m_componentCount; i++) 
        if (m_components[i] && m_components[i]->tickEnabled) 
            m_components[i]->tick();
}

void GameObject::lateComponentTick() {
    for (int i = 0; i < m_componentCount; i++)
        if (m_components[i] && m_components[i]->lateTickEnabled) 
            m_components[i]->lateTick();
}

void GameObject::preRenderComponentTick() {
    for (int i = 0; i < m_componentCount; i++)
        if (m_components[i] && m_components[i]->preRenderTickEnabled) 
            m_components[i]->preRenderTick();
}