#pragma once
#include "WolfEngine/Drivers/EepromDrivers/WE_IEEPROMDriver.hpp"
#include "WolfEngine/Utilities/WE_I2C.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
//  EEPROM24LC512 — 512 Kbit (64 KB) I²C EEPROM
//
//  Address: 0x50–0x57  (A2 A1 A0 pins)
//  Page size: 128 bytes
//  Write cycle time: 5 ms nominal (ACK-polled, up to 20 ms worst-case)
// ─────────────────────────────────────────────────────────────────────────────

class EEPROM24LC512 : public WE_IEEPROMDriver {
public:
    static constexpr uint16_t PAGE_SIZE   = 128;
    static constexpr uint32_t TOTAL_BYTES = 65536;  // 64 KB

    explicit EEPROM24LC512(uint8_t addr = 0x50) : _addr(addr) {}

    // ── WE_IEEPROMDriver interface ────────────────────────────────────────────

    esp_err_t begin() override {
        _handle = I2CManager::addDevice(_addr);
        return _handle ? ESP_OK : ESP_ERR_NOT_FOUND;
    }

    esp_err_t writeBytes(uint16_t memAddr, const uint8_t* buf, size_t len) override {
        if (memAddr + len > TOTAL_BYTES) return ESP_ERR_INVALID_ARG;

        size_t written = 0;
        while (written < len) {
            uint16_t currentAddr = memAddr + written;
            uint16_t pageOffset  = currentAddr % PAGE_SIZE;
            size_t   chunk       = PAGE_SIZE - pageOffset;
            if (chunk > len - written) chunk = len - written;

            esp_err_t err = writePage(currentAddr, buf + written, chunk);
            if (err != ESP_OK) return err;

            written += chunk;
        }
        return ESP_OK;
    }

    esp_err_t readBytes(uint16_t memAddr, uint8_t* buf, size_t len) override {
        if (memAddr + len > TOTAL_BYTES) return ESP_ERR_INVALID_ARG;

        uint8_t addrBuf[2] = { uint8_t(memAddr >> 8), uint8_t(memAddr & 0xFF) };
        return I2CManager::transmitReceive(_handle, addrBuf, 2, buf, len);
    }

    esp_err_t eraseAll() override {
        uint8_t blank[PAGE_SIZE];
        memset(blank, 0xFF, PAGE_SIZE);
        for (uint16_t addr = 0; addr < TOTAL_BYTES; addr += PAGE_SIZE) {
            esp_err_t err = writeBytes(addr, blank, PAGE_SIZE);
            if (err != ESP_OK) return err;
        }
        return ESP_OK;
    }

    uint32_t totalBytes() const override { return TOTAL_BYTES; }

    // ── Concrete-only extras ──────────────────────────────────────────────────

    esp_err_t writeByte(uint16_t memAddr, uint8_t value) {
        return writeBytes(memAddr, &value, 1);
    }

    esp_err_t readByte(uint16_t memAddr, uint8_t& value) {
        return readBytes(memAddr, &value, 1);
    }

private:
    uint8_t                 _addr;
    i2c_master_dev_handle_t _handle = nullptr;

    esp_err_t writePage(uint16_t memAddr, const uint8_t* buf, size_t len) {
        // Prepend 2-byte memory address to the page data (max 128 + 2 = 130 bytes).
        uint8_t txBuf[2 + PAGE_SIZE];
        txBuf[0] = uint8_t(memAddr >> 8);
        txBuf[1] = uint8_t(memAddr & 0xFF);
        memcpy(txBuf + 2, buf, len);
        esp_err_t err = I2CManager::write(_handle, txBuf, len + 2);
        if (err != ESP_OK) return err;
        return pollAck();
    }

    esp_err_t pollAck() {
        constexpr int MAX_POLLS = 20;
        for (int i = 0; i < MAX_POLLS; i++) {
            if (I2CManager::probe(_addr) == ESP_OK) return ESP_OK;
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        return ESP_ERR_TIMEOUT;
    }
};
