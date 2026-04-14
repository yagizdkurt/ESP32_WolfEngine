#pragma once
#include <SDL3/SDL.h>

struct SDLManagerConfig {
    const char* title         = "WolfEngine Desktop";
    int         logicalWidth  = 128;
    int         logicalHeight = 160;
    int         scale         = 4;
};

class SDLManager {
public:
    bool init(const SDLManagerConfig& config);
    bool pollEvents();
    void shutdown();

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};
