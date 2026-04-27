#include "WolfEngine/Utilities/WE_Debug.hpp"
#include "WE_I2C.hpp"
// ─────────────────────────────────────────────────────────────
// ── Lifecycle ──────────────────────────────────────────────

    esp_err_t I2CManager::begin() {
        i2c_config_t cfg = {};
        cfg.mode             = I2C_MODE_MASTER;
        cfg.sda_io_num       = PIN_SDA;
        cfg.scl_io_num       = PIN_SCL;
        cfg.sda_pullup_en    = GPIO_PULLUP_ENABLE;
        cfg.scl_pullup_en    = GPIO_PULLUP_ENABLE;
        cfg.master.clk_speed = FREQ_HZ;

        esp_err_t err = i2c_param_config(PORT, &cfg);
        if (err != ESP_OK) return err;

        return i2c_driver_install(PORT, cfg.mode, 0, 0, 0);
    }

    void I2CManager::end() { i2c_driver_delete(PORT); }

    // ── Low-level helpers ──────────────────────────────────────

    esp_err_t I2CManager::write(uint8_t addr, const uint8_t* data, size_t len) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        if (len > 0) i2c_master_write(cmd, data, len, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(PORT, cmd, TIMEOUT);
        i2c_cmd_link_delete(cmd);
        return err;
    }

    esp_err_t I2CManager::read(uint8_t addr, uint8_t* data, size_t len) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
        if (len > 1)
            i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(PORT, cmd, TIMEOUT);
        i2c_cmd_link_delete(cmd);
        return err;
    }

    // Write register(s): send reg byte first, then data
    esp_err_t I2CManager::writeReg(uint8_t addr, uint8_t reg,
                               const uint8_t* data, size_t len) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg, true);
        if (len > 0) i2c_master_write(cmd, data, len, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(PORT, cmd, TIMEOUT);
        i2c_cmd_link_delete(cmd);
        return err;
    }

    // Write then repeated-start read (reg-based devices)
    esp_err_t I2CManager::readReg(uint8_t addr, uint8_t reg,
                              uint8_t* data, size_t len) {
        // Write register address
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg, true);
        // Repeated start + read
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
        if (len > 1)
            i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(PORT, cmd, TIMEOUT);
        i2c_cmd_link_delete(cmd);
        return err;
    }

    // ── Scan ───────────────────────────────────────────────────

    void I2CManager::scan() {
        WE_LOGI("I2C", "Scanning bus...");
        for (uint8_t addr = 1; addr < 127; addr++) {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);
            esp_err_t err = i2c_master_cmd_begin(PORT, cmd, TIMEOUT);
            i2c_cmd_link_delete(cmd);
            if (err == ESP_OK)
                WE_LOGI("I2C", "  Found device at 0x%02X", addr);
        }
    }
