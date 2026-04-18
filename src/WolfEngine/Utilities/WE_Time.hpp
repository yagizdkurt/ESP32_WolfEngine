// WE_Time.hpp
#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "esp_timer.h"

// =============================================================
//  WETime — static time utility class
//  Time is measured in game time — paused duration is
//  automatically excluded from all calculations.
// =============================================================
class WETime {
public:
    //  Fixed time step in seconds, calculated from TARGET_FRAME_TIME_US.
    //  Use for frame-rate independent movement
    static constexpr float DELTA_TIME = TARGET_FRAME_TIME_US / 1000000.0f;

    static int64_t realNow();    // get real life time in milliseconds
    static int64_t realNowUs();  // get real life time in microseconds
    static int64_t now();        // get game time in milliseconds (real - paused)
    static int64_t nowUs();      // get game time in microseconds (real - paused)

    // ---------------------------------------------------------
    //  since / sinceUs
    //  How much game time has passed since a recorded timestamp.
    //      int64_t spawnTime = WETime::now();
    //      int64_t aliveMs   = WETime::since(spawnTime);
    // ---------------------------------------------------------
    static int64_t since  (int64_t timestamp) { return now()   - timestamp; }
    static int64_t sinceUs(int64_t timestamp) { return nowUs() - timestamp; }

    // Total number of frames since engine start, excluding paused time. Useful for random seed, animation timing, etc.
    static uint32_t frameCount() { return s_frameCount; }

    // ---------------------------------------------------------
    //  pause / resume
    //  Freezes game time. All elapsed/check/timeout calculations
    //  exclude time spent paused automatically.
    // ---------------------------------------------------------
    static void pause();
    static void resume();
    static bool isPaused() { return s_paused; }

private:

    static bool     s_paused;
    static int64_t  s_pauseOffset;
    static int64_t  s_pauseStart;
    static uint32_t s_frameCount;

    WETime() = delete;

    friend class WolfEngine;
    static void incrementFrameCount() { if (!s_paused) s_frameCount++; }
};