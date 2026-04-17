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

    // ---------------------------------------------------------
    //  elapsed
    //  Returns true if at least durationMs has passed since
    //  the timer was last reset. Does not reset automatically.
    //      if (WETime::elapsed(m_timer, 500)) { ... }
    // ---------------------------------------------------------
    static bool elapsed  (int64_t timer, int64_t durationMs) { return (now() - timer) >= durationMs; }
    static bool elapsedUs(int64_t timer, int64_t durationUs) { return (nowUs() - timer) >= durationUs; }

    // ---------------------------------------------------------
    //  check
    //  Returns true if durationMs has passed and resets the
    //  timer automatically. Ideal for cooldowns and repeating
    //  events.
    //      if (WETime::check(m_shootCooldown, 500)) { shoot(); }
    // ---------------------------------------------------------
    static bool check  (int64_t& timer, int64_t durationMs);
    static bool checkUs(int64_t& timer, int64_t durationUs);

    // ---------------------------------------------------------
    //  timeout
    //  Returns true while durationMs has NOT yet passed.
    //  Useful for showing something for a fixed duration:
    //      if (WETime::timeout(m_showTimer, 2000)) { renderMessage(); }
    // ---------------------------------------------------------
    static bool timeout  (int64_t timer, int64_t durationMs) { return (now() - timer) < durationMs;   }
    static bool timeoutUs(int64_t timer, int64_t durationUs) { return (nowUs() - timer) < durationUs; }

    // ---------------------------------------------------------
    //  reset
    //  Sets the timer to current game time.
    //      WETime::reset(m_spawnTimer);
    // ---------------------------------------------------------
    static void reset  (int64_t& timer) { timer = now();   }
    static void resetUs(int64_t& timer) { timer = nowUs(); }


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