#pragma once
// Phase 2 SDL3 DisplayDriver implementation.
// initialize() creates the streaming texture.
// flush() uploads the full 128x160 framebuffer and presents it scaled to the window.

#include "WolfEngine/Drivers/DisplayDrivers/WE_Display_Driver.hpp"
#include <SDL3/SDL.h>
#include <cstring>
#include <cstdio>

class SDL3DisplayDriver : public DisplayDriver {
public:
    SDL3DisplayDriver() {
        screenWidth      = 128;
        screenHeight     = 160;
        requiresByteSwap = false;
    }

    // Called from main_desktop.cpp after SDL_CreateRenderer, before StartEngine().
    void setRenderer(SDL_Renderer* r) { m_renderer = r; }

    void initialize() override {
        if (!m_renderer) {
            fprintf(stderr, "SDL3DisplayDriver::initialize — no renderer set\n");
            return;
        }
        m_texture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_RGB565,
            SDL_TEXTUREACCESS_STREAMING,
            128, 160
        );
        if (!m_texture) {
            fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        }
    }

    // Uploads the full 128x160 framebuffer every call.
    // The dirty rect (x1,y1,x2,y2) is intentionally ignored in Phase 2 —
    // the full buffer is always valid and partial uploads are a later optimisation.
    void flush(const uint16_t* fb, int, int, int, int) override {
        if (!m_renderer || !m_texture) return;

        void* pixels = nullptr;
        int   pitch  = 0;
        if (!SDL_LockTexture(m_texture, nullptr, &pixels, &pitch)) return;

        // pitch may be wider than 128*2 due to alignment — copy row by row.
        const auto* src = reinterpret_cast<const uint8_t*>(fb);
              auto* dst = static_cast<uint8_t*>(pixels);
        for (int y = 0; y < 160; y++) {
            memcpy(dst + y * pitch, src + y * 128 * 2, 128 * 2);
        }
        SDL_UnlockTexture(m_texture);

        SDL_RenderTexture(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }

    // Called by main_desktop.cpp after engineThread.join(), before SDL_DestroyWindow.
    void destroy() {
        if (m_texture)  { SDL_DestroyTexture(m_texture);   m_texture  = nullptr; }
        if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    }

    void setBacklight(uint8_t) override {}
    void sleep(bool)           override {}

private:
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;
};
