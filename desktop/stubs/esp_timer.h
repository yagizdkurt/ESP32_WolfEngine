#pragma once
// Desktop stub for esp_timer.h
// esp_timer_get_time() returns microseconds since program start.

#include <stdint.h>
#include <chrono>

inline int64_t esp_timer_get_time() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<int64_t>( std::chrono::duration_cast<std::chrono::microseconds>(now - start).count() );
}
