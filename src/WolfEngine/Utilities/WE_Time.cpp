#include "WolfEngine/Utilities/WE_Time.hpp"

bool     WETime::s_paused      = false;
int64_t  WETime::s_pauseOffset = 0;
int64_t  WETime::s_pauseStart  = 0;
uint32_t WETime::s_frameCount  = 0;


int64_t WETime::realNow()   { return esp_timer_get_time() / 1000LL; }
int64_t WETime::realNowUs() { return esp_timer_get_time(); }
int64_t WETime::now()   { return esp_timer_get_time() / 1000LL  - s_pauseOffset; }
int64_t WETime::nowUs() { return esp_timer_get_time()           - s_pauseOffset * 1000LL; }

bool WETime::check(int64_t& timer, int64_t durationMs) {
    if ((now() - timer) >= durationMs) {
        timer = now();
        return true;
    }
    return false;
}

bool WETime::checkUs(int64_t& timer, int64_t durationUs) {
    if ((nowUs() - timer) >= durationUs) {
        timer = nowUs();
        return true;
    }
    return false;
}

void WETime::pause() {
    if (s_paused) return;
    s_paused     = true;
    s_pauseStart = esp_timer_get_time() / 1000LL;
}

void WETime::resume() {
    if (!s_paused) return;
    s_paused      = false;
    s_pauseOffset += (esp_timer_get_time() / 1000LL - s_pauseStart);
}