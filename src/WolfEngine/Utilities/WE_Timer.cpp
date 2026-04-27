#include "WolfEngine/Utilities/WE_Timer.hpp"
#include "WolfEngine/Utilities/WE_Debug.hpp"

void Timer::start() {
    m_active    = true;
    m_timestamp = WETime::now();
}

void Timer::stop() {
    m_active = false;
}

void Timer::reset() {
    m_timestamp = WETime::now();
}

bool Timer::elapsed(int64_t durationMs) const {
    WE_ASSERT(durationMs >= 0, "Timer duration must be >= 0");
    if (!m_active) return false;
    return (WETime::now() - m_timestamp) >= durationMs;
}

bool Timer::check(int64_t durationMs) {
    WE_ASSERT(durationMs >= 0, "Timer duration must be >= 0");
    if (!m_active) return false;
    int64_t t = WETime::now();
    if ((t - m_timestamp) >= durationMs) {
        m_timestamp += durationMs;
        return true;
    }
    return false;
}

bool Timer::timeout(int64_t durationMs) const {
    WE_ASSERT(durationMs >= 0, "Timer duration must be >= 0");
    if (!m_active) return false;
    return (WETime::now() - m_timestamp) < durationMs;
}

bool Timer::isActive() const { return m_active; }
