# Audio System

The WolfEngine audio system provides a lightweight, non-blocking sound playback system designed for ESP32.  
It is built around simple `{frequency, duration}` sound clips and uses ESP-IDF’s **LEDC** hardware PWM for tone generation.

This system supports:
- 🎵 Music playback (loopable)
- 🔔 Sound effects (with optional callbacks)
- ⏱ Non-blocking timing
- 🔁 Looping tracks
- 🧠 Minimal memory usage

---

## Overview

Audio playback is handled by the `SoundManager` class and uses two independent channels:

| Channel   | Purpose        |
|-----------|---------------|
| Channel 0 | Music         |
| Channel 1 | Sound Effects |

Each channel plays an array of `SoundClip`.

---

## SoundClip Format

```cpp
struct SoundClip {
    uint16_t frequency;
    uint16_t duration;
};
```

| Field      | Description                  |
|------------|------------------------------|
| `frequency`| Note frequency in Hz (0 = silence) |
| `duration` | Duration in milliseconds     |

Sounds are terminated using a `{0, 0}` clip.

Example:
```cpp
const SoundClip beep[] = {
    { 440, 100 },
    {   0,  50 },
    { 880, 100 },
    {   0,   0 } // terminator
};
```

---

## SoundManager API

### Playback

```cpp
void playMusic(const SoundClip* sound, bool loop = false);
void playSFX(const SoundClip* sound, void (*onFinish)() = nullptr);
```

| Function    | Description                  |
|-------------|------------------------------|
| `playMusic` | Starts playing a sound as music |
| `loop`      | Restarts when finished       |
| `playSFX`   | Plays a one-shot sound effect |
| `onFinish`  | Optional callback when SFX ends |

---

## How Playback Works

Internally:
- Uses `esp_timer_get_time()` for millisecond timing
- Plays one `SoundClip` at a time
- Automatically advances through the array
- Uses `{0,0}` as end marker
- Looping restarts index to `0`

---

### Control
You can call these functions to control the flow of music.

```cpp
void stopMusic();
void stopSFX();
void stopAll();
```

---

## Wiring

Pins are defined in `WE_PINDEFS.hpp`:

```cpp
#define MUSIC_PIN 14
#define SFX_PIN   32
```

See:  
➡ [Pin Definitions](../pin-definitions.md)

---

## Note System (`Note_t`)

WolfEngine provides a predefined musical note table in `WE_Notes.hpp`:

```cpp
enum Note_t : uint16_t {
    REST = 0,
    C4 = 262,
    D4 = 294,
    E4 = 330,
    F4 = 349,
    G4 = 392,
    A4 = 440,
    B4 = 494,
    // ...
};
```

This allows writing melodies in a musical form:

```cpp
const SoundClip melody[] = {
    { C4, 200 },
    { D4, 200 },
    { E4, 200 },
    { REST, 100 },
    { G4, 400 },
    { 0, 0 }
};
```

---

### State Queries

```cpp
bool isMusicPlaying() const;
bool isSFXPlaying() const;
bool isAnyPlaying() const;
bool isPlaying(const SoundClip* sound) const;
```

---

## Limitations

- Square wave only (no PCM audio)
- Single tone per channel
- No volume control (yet)
- No polyphony
- No ADSR or envelopes

---

## Related Pages

- [Pin Definitions](../pin-definitions.md)
