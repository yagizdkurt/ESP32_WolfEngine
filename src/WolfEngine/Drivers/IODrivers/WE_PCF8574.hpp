#pragma once
#include "WolfEngine/Utilities/WE_I2C.hpp"
#include "WE_IExpander.hpp"

// ─────────────────────────────────────────────────────────────
//  PCF8574 — 8-bit I/O expander
//
//  Address: 0x20–0x27  (A2 A1 A0 pins)
//  The chip has no registers: one byte = port state.
//  Pins are quasi-bidirectional: write 1 to read, write 0 to drive low.
// ─────────────────────────────────────────────────────────────

class PCF8574 : public IExpander {
public:
    explicit PCF8574(uint8_t addr = 0) : _addr(addr), _state(0xFF) {}

    // Initialize the chip — drives all pins high for quasi-bidirectional input mode.
    esp_err_t begin() override {
        _handle = I2CManager::addDevice(_addr);
        if (!_handle) return ESP_ERR_NOT_FOUND;
        return write(0xFF);
    }

    // Write all 8 pins at once
    esp_err_t write(uint8_t pins) {
        _state = pins;
        return I2CManager::write(_handle, &_state, 1);
    }

    // Read all 8 pins (pull pins high first to read them)
    esp_err_t read(uint8_t& pins) {
        return I2CManager::read(_handle, &pins, 1);
    }

    // Set individual pin HIGH (1 = input/high-Z, readable)
    esp_err_t pinHigh(uint8_t pin) {
        _state |=  (1 << pin);
        return I2CManager::write(_handle, &_state, 1);
    }

    // Drive individual pin LOW (output)
    esp_err_t pinLow(uint8_t pin) {
        _state &= ~(1 << pin);
        return I2CManager::write(_handle, &_state, 1);
    }

    // Toggle individual pin
    esp_err_t pinToggle(uint8_t pin) {
        _state ^= (1 << pin);
        return I2CManager::write(_handle, &_state, 1);
    }

    // Read a single pin (returns 0 or 1, negative on error)
    int pinRead(uint8_t pin) override {
        pinHigh(pin);
        uint8_t val = 0;
        esp_err_t err = I2CManager::read(_handle, &val, 1);
        if (err != ESP_OK) return -1;
        return (val >> pin) & 1;
    }

    uint8_t cachedState() const { return _state; }

private:
    uint8_t                 _addr;
    uint8_t                 _state;
    i2c_master_dev_handle_t _handle = nullptr;
};