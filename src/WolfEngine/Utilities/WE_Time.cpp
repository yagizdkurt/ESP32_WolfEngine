#include "WolfEngine/Utilities/WE_Time.hpp"

bool     WETime::s_paused      = false;
int64_t  WETime::s_pauseOffset = 0;
int64_t  WETime::s_pauseStart  = 0;
uint32_t WETime::s_frameCount  = 0;


int64_t WETime::realNow()   { return esp_timer_get_time() / 1000LL; }
int64_t WETime::realNowUs() { return esp_timer_get_time(); }

int64_t WETime::now() {
    if (s_paused) return s_pauseStart / 1000LL - s_pauseOffset / 1000LL;
    return esp_timer_get_time() / 1000LL - s_pauseOffset / 1000LL;
}

int64_t WETime::nowUs() {
    if (s_paused) return s_pauseStart - s_pauseOffset;
    return esp_timer_get_time() - s_pauseOffset;
}

void WETime::pause() {
    if (s_paused) return;
    s_paused     = true;
    s_pauseStart = esp_timer_get_time();        // µs
}

void WETime::resume() {
    if (!s_paused) return;
    s_paused      = false;
    s_pauseOffset += esp_timer_get_time() - s_pauseStart;  // accumulate µs
}