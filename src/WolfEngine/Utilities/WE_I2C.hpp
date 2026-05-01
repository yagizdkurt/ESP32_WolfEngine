#pragma once
#include <driver/i2c_master.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "WolfEngine/Settings/WE_Settings.hpp"

// ─────────────────────────────────────────────────────────────
//  I2CManager — static, thread-safe I2C bus manager
//
//  Provides device registration and low-level read/write
//  primitives for any I2C device. All public operations are
//  serialized via a FreeRTOS mutex; safe to call from multiple
//  tasks simultaneously.
//
//  Notes:
//  - Call only after WolfEngine has been initialized.
//  - Each device must call addDevice() once (typically in begin())
//    to obtain a handle used for all subsequent transactions.
// ─────────────────────────────────────────────────────────────

class I2CManager {
public:
    static constexpr gpio_num_t PIN_SDA    = gpio_num_t(Settings.hardware.i2c.sda);
    static constexpr gpio_num_t PIN_SCL    = gpio_num_t(Settings.hardware.i2c.scl);
    static constexpr uint32_t   FREQ_HZ    = 400000;

    // Timeout for any single bus transaction in milliseconds.
    // 10 ms is generous enough for clock-stretching devices while still catching hung buses.
    static constexpr int        TIMEOUT_MS = 10;

    // Timeout waiting to acquire the bus mutex. Must be greater than TIMEOUT_MS
    // so a task already holding the lock can always complete its transaction first.
    static constexpr int        MUTEX_TIMEOUT_MS = 100;

    // Maximum attempts per transaction before returning the final error.
    static constexpr int        MAX_RETRIES     = 3;
    // Base delay between retry attempts in ms; doubles each attempt (exponential backoff).
    static constexpr int        BACKOFF_BASE_MS = 1;

    // Register a device at the given 7-bit address at FREQ_HZ speed.
    // Call once per device in its begin() before any transactions.
    // Returns a handle used for all subsequent transactions with that device.
    static i2c_master_dev_handle_t addDevice(uint8_t addr);

    // Sends len bytes from data to the device.
    static esp_err_t write(i2c_master_dev_handle_t dev, const uint8_t* data, size_t len);

    // Reads len bytes from the device into data.
    static esp_err_t read(i2c_master_dev_handle_t dev, uint8_t* data, size_t len);

    // Writes a register byte then len data bytes in a single transaction.
    // Generates: START → address+W → reg → data bytes → STOP.
    static esp_err_t writeReg(i2c_master_dev_handle_t dev, uint8_t reg, const uint8_t* data, size_t len);

    // Writes reg byte then reads len bytes with a repeated START.
    // Generates: START → address+W → reg → REPEATED START → address+R → data → STOP.
    static esp_err_t readReg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t len);

    // Writes tx_len bytes then reads rx_len bytes with a repeated START between.
    // Used for devices with multi-byte register addresses (e.g. EEPROM with 16-bit address).
    static esp_err_t transmitReceive(i2c_master_dev_handle_t dev,
                                     const uint8_t* tx, size_t tx_len,
                                     uint8_t* rx, size_t rx_len);

    // Returns ESP_OK if the device at addr ACKs. Does not require a device handle.
    static esp_err_t probe(uint8_t addr);

    // Scans all 127 valid 7-bit addresses and logs any device that ACKs.
    // Should not be called in production/release builds.
    static void scan();

    // Logs bus state: SDA/SCL pin levels, bus handle state.
    // Call after begin() to confirm the bus is healthy before using devices.
    static void diagBusState();

// Set to 1 to log every I2C transaction (direction, length, result).
// Significant serial output — enable only when diagnosing bus issues.
#ifndef WE_I2C_DEBUG_VERBOSE
    #define WE_I2C_DEBUG_VERBOSE 1
#endif

private:
    friend class WolfEngine;
    static esp_err_t begin();
    static void end();
    static bool isRetryable(esp_err_t err);
    static i2c_master_bus_handle_t s_busHandle;
    static SemaphoreHandle_t       s_busMutex;
    I2CManager() = delete;
};
