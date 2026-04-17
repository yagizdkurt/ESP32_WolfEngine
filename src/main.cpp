#include "WolfEngine/WolfEngine.hpp"
#include "testgameobject.hpp"

extern "C" void app_main() {
Engine().StartEngine();

GameObject::Create<TestGameObject>();

Engine().StartGame();
}