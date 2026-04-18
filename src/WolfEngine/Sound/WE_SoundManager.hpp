#pragma once
#include "WolfEngine/Sound/WE_Notes.hpp"
#include <stdint.h>
#include "driver/ledc.h"

class SoundManager {
public:

    // Playback
    void playMusic(const SoundClip* sound, bool loop = false);
    void playSFX(const SoundClip* sound, void (*onFinish)() = nullptr);

    // Control
    void stopMusic();
    void stopSFX();
    void stopAll();

    // State
    bool isMusicPlaying()              const;
    bool isSFXPlaying()                const;
    bool isAnyPlaying()                const;
    bool isPlaying(const SoundClip* sound) const;

private:
    SoundManager() {}
    friend class WolfEngine;
    void Initialize();
    void update();

    struct Channel {
        const SoundClip* sound      = nullptr;
        uint16_t         index      = 0;
        uint32_t         nextTick   = 0;
        bool             playing    = false;
        bool             loop       = false;
        void             (*onFinish)() = nullptr;
    };

    Channel _music = {nullptr, 0, 0, false};
    Channel _sfx   = {nullptr, 0, 0, false};

    void _updateChannel(Channel& ch, ledc_channel_t channel);
    void _startChannel (Channel& ch, const SoundClip* sound, bool loop, void (*onFinish)());
    void _stopChannel  (Channel& ch, ledc_channel_t channel);
};