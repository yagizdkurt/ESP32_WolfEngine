#include "WE_GameObject.hpp"
#include "WolfEngine/WolfEngine.hpp"

bool GameObject::CreateObject() {
    GameObjectRegistry& reg = Engine().m_GameObjectRegistry;
    if (reg.count >= MAX_GAME_OBJECTS) return false;
    for (int i = 0; i < MAX_GAME_OBJECTS; i++) {
        if (reg.gameObjects[i] == nullptr) {
            reg.gameObjects[i] = this;
            reg.count++;
            return true;
        }
    }
    return false; // Didnt find an empty slot. For whatever reason.
}

GameObject::GameObject(){}
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