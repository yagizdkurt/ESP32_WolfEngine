#include "testgameobject.hpp"
// Desktop entry point.
//
// Mirrors app_main() from src/main.cpp:
//   Engine().StartEngine()  — initialises all subsystems
//   Engine().StartGame()    — runs the 30 fps game loop, exits when RequestQuit() is called
//
// StartGame() runs on a background thread so the SDL3 event loop can own the main
// thread (required on macOS and most Linux WMs).
// On ESC or window-close: RequestQuit() signals the engine, join() waits for it to
// finish, then SDL objects are destroyed in order before main() returns normally.

#include <thread>
#include "WolfEngine/WolfEngine.hpp"
#include "WE_SDLManager.hpp"

int main(int, char*[]) {
    SDLManager sdl;
    if (!sdl.init({ 
        .title = "WolfEngine Desktop", 
        .logicalWidth  = 128,
        .logicalHeight = 160,
        .scale = 4,
        .brightness = 3.0f })
    ) return 1;

    Engine().StartEngine();

    // Game Logic Here:
    GameObject::Create<TestGameObject>();

    std::thread engineThread([] { Engine().StartGame(); });
    while (sdl.pollEvents()) {}

    Engine().RequestQuit();
    engineThread.join();
    sdl.shutdown();
    return 0;
}
