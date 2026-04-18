#pragma once
#include <SDL3/SDL.h>
#include <thread>

class WolfEngine;
struct SDLManagerConfig {
    const char* title         = "WolfEngine Desktop";
    int         logicalWidth  = 128;
    int         logicalHeight = 160;
    int         scale         = 4;
    float       brightness    = 1.5f;
};

class SDLManager {
public:
    bool init(const SDLManagerConfig& config);
    bool pollEvents();
    void shutdown(std::thread *engineThread = nullptr, WolfEngine* engine = nullptr);

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};
