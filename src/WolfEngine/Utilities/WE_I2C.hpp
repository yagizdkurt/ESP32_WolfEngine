#pragma once
#include <driver/i2c.h>
#include "WolfEngine/Settings/WE_PINDEFS.hpp"

// ─────────────────────────────────────────────────────────────
//  I2CManager — static I2C bus manager
//
//  Provides low-level read/write primitives for any I2C device.
//  Available to engine drivers and user code alike.
//
//  Notes:
//  - Call only after WolfEngine has been initialized.
//  - Do not call from multiple tasks simultaneously.
//  - To add a custom driver, use these primitives directly.
// ─────────────────────────────────────────────────────────────

class I2CManager {
public:
    static constexpr i2c_port_t PORT    = I2C_NUM_0;
    static constexpr gpio_num_t PIN_SDA = gpio_num_t(I2C_PIN_SDA);
    static constexpr gpio_num_t PIN_SCL = gpio_num_t(I2C_PIN_SCL);
    static constexpr uint32_t   FREQ_HZ = 400000;

    // Maximum time any single bus transaction is allowed to take
    // before it is considered failed. 10 ms is generous enough to
    // handle clock-stretching devices while still catching hung buses.
    static constexpr uint32_t   TIMEOUT = pdMS_TO_TICKS(10);

    // Sends `len` bytes from `data` to the device at 7-bit address `addr`.
    // Generates: START → address+W → data bytes → STOP.
    // Returns ESP_OK on ACK from device, ESP_FAIL or ESP_ERR_TIMEOUT otherwise.
    static esp_err_t write(uint8_t addr, const uint8_t* data, size_t len);

    // Reads `len` bytes from the device at 7-bit address `addr` into `data`.
    // Generates: START → address+R → data bytes (ACK each) → NACK → STOP.
    // The device must already have its internal pointer set (e.g. via a prior write).
    static esp_err_t read(uint8_t addr, uint8_t* data, size_t len);

    // Writes `len` bytes to a specific 8-bit register address on the device.
    // Generates: START → address+W → reg → data bytes → STOP.
    // Suitable for devices that use a single-byte register/command prefix.
    static esp_err_t writeReg(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len);

    // Reads `len` bytes from a specific 8-bit register address on the device.
    // Generates: START → address+W → reg → REPEATED START → address+R → data → STOP.
    // The repeated start (vs stop+start) keeps bus ownership during the turnaround,
    // which is required by some devices to guarantee atomic register reads.
    static esp_err_t readReg(uint8_t addr, uint8_t reg, uint8_t* data, size_t len);

    // Scans all 127 valid 7-bit addresses and logs any device that ACKs.
    // Useful during hardware bring-up to verify wiring and address configuration.
    // Should not be called in production/release builds.
    static void scan();

private:
    friend class WolfEngine;
    static esp_err_t begin();
    static void end();
    I2CManager() = delete;

};