// WE_Timer.hpp
#pragma once
#include <stdint.h>
#include "WolfEngine/Utilities/WE_Time.hpp"

// =============================================================
//  Timer
//  A lightweight timer struct that wraps an int64_t timestamp
//  and delegates to WETime:: internally.
//
//  Must be started with start() before any queries return
//  meaningful results. All queries return safe defaults when
//  inactive.
//
//  EXAMPLE:
//      Timer m_shootCooldown;
//
//      void Start() override {
//          m_shootCooldown.start();
//      }
//
//      void Update() override {
//          if (m_shootCooldown.check(500)) {
//              shoot();
//          }
//      }
// =============================================================
// =============================================================
//  Timer
//  Lightweight timer struct. Must be started with start()
//  before any queries return meaningful results.
// =============================================================
struct Timer {

    // Activates the timer and resets the timestamp to now.
    void start();

    // Deactivates the timer. All queries return false.
    void stop();

    // Resets the timestamp to now without changing active state.
    void reset();

    // Returns true if durationMs has passed. Does not auto-reset.
    bool elapsed(int64_t durationMs) const;

    // Returns true if durationMs has passed and auto-resets.
    bool check(int64_t durationMs);

    // Returns true while durationMs has NOT yet passed.
    bool timeout(int64_t durationMs) const;

    // Returns true if the timer has been started and not stopped.
    bool isActive() const;

private:
    int64_t m_timestamp = 0;
    bool    m_active    = false;
};