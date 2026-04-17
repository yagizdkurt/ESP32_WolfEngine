#include "WE_SoundManager.hpp"
#include "WolfEngine/Settings/WE_PINDEFS.hpp"
#include "esp_timer.h"

// ─── Public ────────────────────────────────────────────────────────────────

void SoundManager::playMusic(const SoundClip* sound, bool loop) {
    if (sound == nullptr) return;
    _startChannel(_music, sound, loop, nullptr);
}

void SoundManager::playSFX(const SoundClip* sound, void (*onFinish)()) {
    if (sound == nullptr) return;
    _startChannel(_sfx, sound, false, onFinish);
}

void SoundManager::stopMusic()  { _stopChannel(_music, LEDC_CHANNEL_0); }
void SoundManager::stopSFX()    { _stopChannel(_sfx,   LEDC_CHANNEL_1); }
void SoundManager::stopAll()    { stopMusic(); stopSFX();                }

bool SoundManager::isMusicPlaying()            const { return _music.playing;                          }
bool SoundManager::isSFXPlaying()              const { return _sfx.playing;                            }
bool SoundManager::isAnyPlaying()              const { return _music.playing || _sfx.playing;          }

bool SoundManager::isPlaying(const SoundClip* sound) const {
    return (_music.playing && _music.sound == sound) ||
           (_sfx.playing   && _sfx.sound   == sound);
}


// ─── TIMER Helper ──────────────────────────────────────────────────────────

static inline uint32_t _millis() { return (uint32_t)(esp_timer_get_time() / 1000ULL); }

// ─── LEDC Helpers ──────────────────────────────────────────────────────────

static void _initChannel(uint8_t pin, ledc_channel_t channel) {
    ledc_timer_config_t timer = {};
    timer.speed_mode      = LEDC_LOW_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num       = LEDC_TIMER_0;
    timer.freq_hz         = 1000;
    timer.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&timer);

    ledc_channel_config_t cfg = {};
    cfg.gpio_num   = pin;
    cfg.speed_mode = LEDC_LOW_SPEED_MODE;
    cfg.channel    = channel;
    cfg.timer_sel  = LEDC_TIMER_0;
    cfg.duty       = 0;
    cfg.hpoint     = 0;
    ledc_channel_config(&cfg);
}

static void _tone(ledc_channel_t channel, uint32_t freq) {
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

static void _noTone(ledc_channel_t channel) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

// ─── Private ───────────────────────────────────────────────────────────────

void SoundManager::update() {
    _updateChannel(_music, LEDC_CHANNEL_0);
    _updateChannel(_sfx,   LEDC_CHANNEL_1);
}

void SoundManager::Initialize() {
    _initChannel(MUSIC_PIN, LEDC_CHANNEL_0);
    _initChannel(SFX_PIN,   LEDC_CHANNEL_1);
}

void SoundManager::_startChannel(Channel& ch, const SoundClip* sound, bool loop, void (*onFinish)()) {
    ch.sound    = sound;
    ch.index    = 0;
    ch.nextTick = _millis();
    ch.playing  = true;
    ch.loop     = loop;
    ch.onFinish = onFinish;
}

void SoundManager::_stopChannel(Channel& ch, ledc_channel_t channel) {
    _noTone(channel);
    void (*cb)() = ch.onFinish;  // grab before reset
    ch.sound    = nullptr;
    ch.index    = 0;
    ch.nextTick = 0;
    ch.playing  = false;
    ch.loop     = false;
    ch.onFinish = nullptr;
    if (cb) cb();                // fire after reset so state is clean when callback runs
}

void SoundManager::_updateChannel(Channel& ch, ledc_channel_t channel) {
    if (!ch.playing || ch.sound == nullptr) return;

    uint32_t now = _millis();
    if (now < ch.nextTick) return;

    const SoundClip& clip = ch.sound[ch.index];

    // Terminator — end of sound
    if (clip.frequency == 0 && clip.duration == 0) {
        if (ch.loop) {
            ch.index    = 0;            // restart from beginning
            ch.nextTick = now;          // play immediately
            return;
        }
        _stopChannel(ch, channel);
        return;
    }

    if (clip.frequency == 0) {
        _noTone(channel);
    } else {
        _tone(channel, clip.frequency);
    }

    ch.nextTick = now + clip.duration;
    ch.index++;
}