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
#include "WE_SDLInputDriver.hpp"

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

    // static — must outlive the engine. InputManager holds a raw pointer to this
    // and calls flush() every frame. A non-static local would be destroyed before
    // the engine thread stops, causing undefined behaviour.
    static SDLInputDriver inputDriver;
    Input().setInputProvider(&inputDriver);
    Input().setAlwaysEnableController0(true);  // explicit — desktop always has controller 0

    // Game Logic Here:


    // StartGame() runs the game loop on a background thread so the main thread can own the SDL event loop.
    std::thread engineThread([] { Engine().StartGame(); });
    while (sdl.pollEvents()) {} // PollEvents returns false on quit event, breaking the loop and proceeding to shutdown.
    sdl.shutdown(&engineThread, &Engine());
    return 0;
}
