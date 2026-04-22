#include "WE_GameObject.hpp"
#include "WolfEngine/WolfEngine.hpp"

bool GameObject::CreateObject() {
    GameObjectRegistry& reg = Engine().m_GameObjectRegistry;
    if (reg.count >= Settings.limits.maxGameObjects) return false;
    for (int i = 0; i < Settings.limits.maxGameObjects; i++) {
        if (reg.gameObjects[i] == nullptr) {
            reg.gameObjects[i] = this;
            reg.count++;
            return true;
        }
    }
    return false; // Didnt find an empty slot. For whatever reason.
}

GameObject::GameObject() {}
GameObject::~GameObject() {}

void GameObject::DestroyGameObject(GameObject *gameObject) {
    GameObjectRegistry& reg = Engine().m_GameObjectRegistry;
    if (reg.gameObjects[gameObject->id] == gameObject) { // if its in the registry, free it and remove it from the registry
        reg.gameObjects[gameObject->id] = nullptr;
        reg.count--;
    }
    delete gameObject;
}


void GameObject::callStart() {
    if (!hasStarted) {
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
        if (m_components[i]) 
            m_components[i]->preRenderTick();
}