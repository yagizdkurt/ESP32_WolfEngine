#pragma once
#include <stdint.h>
#include "esp_err.h"

// ─────────────────────────────────────────────────────────────────────────────
//  WE_IEEPROMDriver — Abstract EEPROM driver interface
//
//  All EEPROM drivers must implement this interface so that WE_SaveManager
//  can communicate with any chip without knowing its concrete type.
//
//  Pattern mirrors WE_IExpander (abstract IO expander interface).
// ─────────────────────────────────────────────────────────────────────────────

class WE_IEEPROMDriver {
public:
    // Register the device on the I2C bus. Call once after I2CManager::begin().
    virtual esp_err_t begin() { return ESP_OK; }

    // Write len bytes from buf to the chip starting at addr.
    // Implementations must handle page-boundary splits internally.
    // Blocks until the write cycle completes (5–20 ms per page).
    virtual esp_err_t writeBytes(uint16_t addr, const uint8_t* buf, size_t len) = 0;

    // Read len bytes from the chip starting at addr into buf.
    // Reads complete immediately (no write-cycle delay).
    virtual esp_err_t readBytes(uint16_t addr, uint8_t* buf, size_t len) = 0;

    // Fill the entire chip with 0xFF. Very slow — do not call during gameplay.
    virtual esp_err_t eraseAll() = 0;

    // Return the total addressable byte capacity of this chip.
    virtual uint32_t totalBytes() const = 0;

    virtual ~WE_IEEPROMDriver() = default;
};
