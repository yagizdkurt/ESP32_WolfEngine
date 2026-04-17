#include "WE_SDLManager.hpp"
#include "stubs/WE_Display_SDL3.hpp"
// WE_SDLInputDriver.hpp is included here for pollEvents().
// SDLInputDriver is injected into InputManager separately in main_desktop.cpp
// after StartEngine() — SDLManager does not own the provider lifetime.
#include "WE_SDLInputDriver.hpp"
#include <cstdio>
#include "WolfEngine/WolfEngine.hpp"

// Defined in stubs/WE_Display_SDL3.cpp — returns the concrete driver instance.
SDL3DisplayDriver& GetSDLDriver();

bool SDLManager::init(const SDLManagerConfig& config) {
    // Initalize SDL video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO)) { fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return false; }

    // Create SDL window
    window = SDL_CreateWindow( config.title, config.logicalWidth * config.scale, config.logicalHeight * config.scale, 0 );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Create SDL renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Set up logical presentation with letterboxing to maintain aspect ratio.
    SDL_SetRenderLogicalPresentation( renderer, config.logicalWidth, config.logicalHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX );
    // Apply brightness scaling to renderer (affects all rendering operations, including the display driver texture).
    SDL_SetRenderColorScale(renderer, config.brightness);
    // Pass the renderer to the display driver so it can create its texture.
    GetSDLDriver().setRenderer(renderer);
    return true;
}

// Keyboard Events are polled here
bool SDLManager::pollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        SDLInputDriver::processEvent(e);
        if (e.type == SDL_EVENT_QUIT) return false;
        if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) return false;
    }
    SDL_Delay(16);  // ~60 Hz event polling, no busy-spin
    return true;
}

void SDLManager::shutdown(std::thread *engineThread, WolfEngine* engine) {
    if (engine) { engine->RequestQuit(); }
    if (engineThread && engineThread->joinable()) { engineThread->join(); }
    GetSDLDriver().destroy();   // texture → renderer
    SDL_DestroyWindow(window);
    SDL_Quit();
}