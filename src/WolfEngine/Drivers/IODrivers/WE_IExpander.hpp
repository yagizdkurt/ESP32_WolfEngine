#pragma once
#include <stdint.h>
#include "esp_err.h"

// =============================================================
//  IExpander
//  Abstract interface for I2C I/O expander chips.
//  Implement this interface to add support for a new expander.
//
//  Controller talks exclusively through this interface —
//  it has no knowledge of the specific chip underneath.
// =============================================================
class IExpander {
public:
    // Initialize the chip. Called once during Controller init.
    virtual esp_err_t begin() = 0;

    // Read a single pin. Returns 0 or 1, negative on error.
    virtual int pinRead(uint8_t pin) = 0;

    virtual ~IExpander() = default;
};