// Phase 2 desktop entry point.
//
// Mirrors app_main() from src/main.cpp:
//   Engine().StartEngine()  — initialises all subsystems
//   Engine().StartGame()    — runs the 30 fps game loop, exits when RequestQuit() is called
//
// StartGame() runs on a background thread so the SDL3 event loop can own the main
// thread (required on macOS and most Linux WMs).
// On ESC or window-close: RequestQuit() signals the engine, join() waits for it to
// finish, then SDL objects are destroyed in order before main() returns normally.

#include <SDL3/SDL.h>
#include <thread>
#include <cstdio>

#include "WolfEngine/WolfEngine.hpp"
#include "stubs/WE_Display_SDL3.hpp"

// Defined in WE_Display_SDL3.cpp — returns the concrete driver instance.
SDL3DisplayDriver& GetSDLDriver();

int main(int /*argc*/, char* /*argv*/[]) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        getchar();
        return 1;
    }

    // Window at 4x logical resolution (512x640 = 128x160 * 4).
    // Logical scaling is handled by the renderer, not the window size.
    SDL_Window* win = SDL_CreateWindow(
        "WolfEngine Desktop",
        512, 640,
        0
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(win, nullptr);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Scale the logical 128x160 canvas to fill the window; letterbox if aspect differs.
    SDL_SetRenderLogicalPresentation(renderer, 128, 160, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Pass the renderer to the driver before StartEngine() calls initialize().
    GetSDLDriver().setRenderer(renderer);

    // Initialise all engine subsystems.
    Engine().StartEngine();

    // StartGame() blocks until RequestQuit() — run it on a background thread.
    std::thread engineThread([] { Engine().StartGame(); });

    // Main thread: process SDL3 events so the window stays responsive.
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)                                   quit = true;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)  quit = true;
        }
        SDL_Delay(16);  // ~60 Hz event polling, no busy-spin
    }

    // Orderly shutdown:
    // 1. Signal the engine loop to exit after its current frame.
    Engine().RequestQuit();
    // 2. Wait for StartGame() to return — guarantees flush() will not run concurrently
    //    with the SDL object teardown below.
    engineThread.join();
    // 3. Destroy SDL objects in dependency order.
    GetSDLDriver().destroy();   // texture → renderer
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
