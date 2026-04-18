#pragma once
#include "WolfEngine/Utilities/WE_I2C.hpp"
#include "WE_IExpander.hpp"

// ─────────────────────────────────────────────────────────────
//  MCP23017 — 16-bit I/O expander
//
//  Address: 0x20–0x27  (A2 A1 A0 pins)
//  Register-based. Port A = pins 0–7, Port B = pins 8–15.
//  Requires direction and pull-up configuration before reading.
// ─────────────────────────────────────────────────────────────

class MCP23017 : public IExpander {
public:
    explicit MCP23017(uint8_t addr = 0) : _addr(addr) {}

    // Initialize — set all pins as inputs with pull-ups enabled
    esp_err_t begin() override {
        // Set all pins as inputs
        uint8_t allInputs = 0xFF;
        esp_err_t err = I2CManager::writeReg(_addr, REG_IODIRA, &allInputs, 1);
        if (err != ESP_OK) return err;
        err = I2CManager::writeReg(_addr, REG_IODIRB, &allInputs, 1);
        if (err != ESP_OK) return err;

        // Enable pull-ups on all pins
        err = I2CManager::writeReg(_addr, REG_GPPUA, &allInputs, 1);
        if (err != ESP_OK) return err;
        err = I2CManager::writeReg(_addr, REG_GPPUB, &allInputs, 1);
        if (err != ESP_OK) return err;

        return ESP_OK;
    }

    // Read a single pin (returns 0 or 1, negative on error)
    int pinRead(uint8_t pin) override {
        uint8_t reg  = (pin < 8) ? REG_GPIOA : REG_GPIOB;
        uint8_t bit  = (pin < 8) ? pin : (pin - 8);
        uint8_t val  = 0;
        esp_err_t err = I2CManager::readReg(_addr, reg, &val, 1);
        if (err != ESP_OK) return -1;
        return (val >> bit) & 1;
    }

    // Read all 16 pins at once
    esp_err_t read(uint16_t& pins) {
        uint8_t portA = 0, portB = 0;
        esp_err_t err = I2CManager::readReg(_addr, REG_GPIOA, &portA, 1);
        if (err != ESP_OK) return err;
        err = I2CManager::readReg(_addr, REG_GPIOB, &portB, 1);
        if (err != ESP_OK) return err;
        pins = portA | (portB << 8);
        return ESP_OK;
    }

private:
    uint8_t _addr;

    // Register addresses
    static constexpr uint8_t REG_IODIRA = 0x00; // Direction port A
    static constexpr uint8_t REG_IODIRB = 0x01; // Direction port B
    static constexpr uint8_t REG_GPPUA  = 0x0C; // Pull-up port A
    static constexpr uint8_t REG_GPPUB  = 0x0D; // Pull-up port B
    static constexpr uint8_t REG_GPIOA  = 0x12; // GPIO read port A
    static constexpr uint8_t REG_GPIOB  = 0x13; // GPIO read port B
};