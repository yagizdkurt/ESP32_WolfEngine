#pragma once
#include "WolfEngine/Utilities/WE_I2C.hpp"
#include "WE_IExpander.hpp"

// ─────────────────────────────────────────────────────────────
//  PCF8575 — 16-bit I/O expander
//
//  Address: 0x20–0x27  (A2 A1 A0 pins)
//  Same protocol as PCF8574 but 16 pins — two bytes per transaction.
//  Pins are quasi-bidirectional: write 1 to read, write 0 to drive low.
//  P0–P7 = low byte, P8–P15 = high byte.
// ─────────────────────────────────────────────────────────────

class PCF8575 : public IExpander {
public:
    explicit PCF8575(uint8_t addr = 0) : _addr(addr), _state(0xFFFF) {}

    // Initialize the chip — drives all 16 pins high for input mode.
    esp_err_t begin() override {
        return write(0xFFFF);
    }

    // Write all 16 pins at once
    esp_err_t write(uint16_t pins) {
        _state = pins;
        uint8_t buf[2] = { 
            static_cast<uint8_t>(_state & 0xFF),         // low byte  P0–P7
            static_cast<uint8_t>((_state >> 8) & 0xFF)  // high byte P8–P15
        };
        return I2CManager::write(_addr, buf, 2);
    }

    // Read all 16 pins
    esp_err_t read(uint16_t& pins) {
        uint8_t buf[2] = {};
        esp_err_t err = I2CManager::read(_addr, buf, 2);
        if (err != ESP_OK) return err;
        pins = buf[0] | (buf[1] << 8);
        return ESP_OK;
    }

    // Set individual pin HIGH (input/high-Z, readable)
    esp_err_t pinHigh(uint8_t pin) {
        _state |=  (1 << pin);
        return write(_state);
    }

    // Drive individual pin LOW (output)
    esp_err_t pinLow(uint8_t pin) {
        _state &= ~(1 << pin);
        return write(_state);
    }

    // Toggle individual pin
    esp_err_t pinToggle(uint8_t pin) {
        _state ^= (1 << pin);
        return write(_state);
    }

    // Read a single pin (returns 0 or 1, negative on error)
    int pinRead(uint8_t pin) override {
        pinHigh(pin);
        uint16_t val = 0;
        esp_err_t err = read(val);
        if (err != ESP_OK) return -1;
        return (val >> pin) & 1;
    }

    uint16_t cachedState() const { return _state; }

private:
    uint8_t  _addr;
    uint16_t _state; // 16-bit shadow register
};