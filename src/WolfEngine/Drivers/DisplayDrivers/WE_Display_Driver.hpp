#pragma once
#include <stdint.h>
#include <stdbool.h>

class DisplayDriver {
public:
    uint8_t screenWidth;
    uint8_t screenHeight;
    virtual void initialize() = 0;
    virtual void flush(const uint16_t* framebuffer, int x1, int y1, int x2, int y2) = 0;
    virtual void setBacklight(uint8_t brightness) {}  // optional, default no-op
    virtual void sleep(bool enable) {}                // optional, default no-op
    virtual ~DisplayDriver() = default;
    bool requiresByteSwap = false; // set true in driver if display needs RGB565 byte swap
};