#include "WolfEngine/Utilities/WE_Timer.hpp"

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
    if (!m_active) return false;
    return (WETime::now() - m_timestamp) >= durationMs;
}

bool Timer::check(int64_t durationMs) {
    if (!m_active) return false;
    if ((WETime::now() - m_timestamp) >= durationMs) {
        m_timestamp = WETime::now();
        return true;
    }
    return false;
}

bool Timer::timeout(int64_t durationMs) const {
    if (!m_active) return false;
    return (WETime::now() - m_timestamp) < durationMs;
}

bool Timer::isActive() const { return m_active; }