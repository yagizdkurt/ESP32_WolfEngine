#pragma once
#include "WolfEngine/Drivers/EepromDrivers/WE_IEEPROMDriver.hpp"
#include "WolfEngine/Utilities/WE_I2C.hpp"
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
    static constexpr uint16_t PAGE_SIZE      = 128;
    static constexpr uint32_t TOTAL_BYTES    = 65536;  // 64 KB

    explicit EEPROM24LC512(uint8_t addr = 0x50) : _addr(addr) {}

    // ── WE_IEEPROMDriver interface ────────────────────────────────────────────

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

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, addrBuf, 2, true);
        i2c_master_start(cmd);  // repeated start
        i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_READ, true);
        if (len > 1)
            i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(I2CManager::PORT, cmd, I2CManager::TIMEOUT);
        i2c_cmd_link_delete(cmd);
        return err;
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
    uint8_t _addr;

    esp_err_t writePage(uint16_t memAddr, const uint8_t* buf, size_t len) {
        uint8_t addrBuf[2] = { uint8_t(memAddr >> 8), uint8_t(memAddr & 0xFF) };

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, addrBuf, 2, true);
        i2c_master_write(cmd, buf, len, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(I2CManager::PORT, cmd, I2CManager::TIMEOUT);
        i2c_cmd_link_delete(cmd);
        if (err != ESP_OK) return err;

        return pollAck();
    }

    esp_err_t pollAck() {
        constexpr int MAX_POLLS = 20;
        for (int i = 0; i < MAX_POLLS; i++) {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);
            esp_err_t err = i2c_master_cmd_begin(I2CManager::PORT, cmd, I2CManager::TIMEOUT);
            i2c_cmd_link_delete(cmd);
            if (err == ESP_OK) return ESP_OK;
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        return ESP_ERR_TIMEOUT;
    }
};
