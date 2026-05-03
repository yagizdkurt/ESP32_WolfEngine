#include "WolfEngine/Utilities/Debug/WE_Debug.hpp"
#include "WE_I2C.hpp"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include <string.h>

i2c_master_bus_handle_t I2CManager::s_busHandle = nullptr;
SemaphoreHandle_t       I2CManager::s_busMutex  = nullptr;

// ── Lifecycle ──────────────────────────────────────────────

esp_err_t I2CManager::begin() {
    WE_LOGI("I2C", "begin: SDA=%d SCL=%d freq=%lukHz",
            (int)PIN_SDA, (int)PIN_SCL, (unsigned long)(FREQ_HZ / 1000));

    // 1. Release bus
    gpio_set_direction((gpio_num_t)PIN_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_direction((gpio_num_t)PIN_SCL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level((gpio_num_t)PIN_SDA, 1);
    gpio_set_level((gpio_num_t)PIN_SCL, 1);
    esp_rom_delay_us(5);

    // 2. Clock out stuck slave state
    for (int i = 0; i < 9; i++) {
        gpio_set_level((gpio_num_t)PIN_SCL, 0);
        esp_rom_delay_us(5);
        gpio_set_level((gpio_num_t)PIN_SCL, 1);
        esp_rom_delay_us(5);
    }

    // 3. Force STOP condition
    gpio_set_level((gpio_num_t)PIN_SDA, 0);
    esp_rom_delay_us(5);
    gpio_set_level((gpio_num_t)PIN_SCL, 1);
    esp_rom_delay_us(5);
    gpio_set_level((gpio_num_t)PIN_SDA, 1);
    esp_rom_delay_us(5);

    WE_LOGI("I2C", "begin: bus recovery done (SDA=%d SCL=%d)",
            gpio_get_level((gpio_num_t)PIN_SDA),
            gpio_get_level((gpio_num_t)PIN_SCL));

    // 4. Install new master bus driver
    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port                     = I2C_NUM_0;
    cfg.sda_io_num                   = PIN_SDA;
    cfg.scl_io_num                   = PIN_SCL;
    cfg.clk_source                   = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt            = 7;
    cfg.flags.enable_internal_pullup = true;

    esp_err_t err = i2c_new_master_bus(&cfg, &s_busHandle);
    if (err != ESP_OK) {
        WE_LOGE("I2C", "begin: i2c_new_master_bus failed: %s", esp_err_to_name(err));
        return err;
    }

    s_busMutex = xSemaphoreCreateMutex();
    assert(s_busMutex != nullptr);

    WE_LOGI("I2C", "begin: OK");
    return ESP_OK;
}

void I2CManager::end() {
    WE_LOGI("I2C", "end: deleting master bus");
    i2c_del_master_bus(s_busHandle);
    s_busHandle = nullptr;
    if (s_busMutex) {
        vSemaphoreDelete(s_busMutex);
        s_busMutex = nullptr;
    }
}

// ── Device registration ────────────────────────────────────

i2c_master_dev_handle_t I2CManager::addDevice(uint8_t addr) {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        WE_LOGE("I2C", "addDevice 0x%02X: mutex timeout", addr);
        return nullptr;
    }

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address  = addr;
    dev_cfg.scl_speed_hz    = FREQ_HZ;

    i2c_master_dev_handle_t handle = nullptr;
    esp_err_t err = i2c_master_bus_add_device(s_busHandle, &dev_cfg, &handle);
    xSemaphoreGive(s_busMutex);

    if (err != ESP_OK) {
        WE_LOGE("I2C", "addDevice 0x%02X failed: %s", addr, esp_err_to_name(err));
        return nullptr;
    }
    WE_LOGI("I2C", "addDevice 0x%02X OK", addr);
    return handle;
}

// ── Retry helper ──────────────────────────────────────────

bool I2CManager::isRetryable(esp_err_t err) {
    return err == ESP_ERR_TIMEOUT || err == ESP_FAIL;
}

// ── Low-level primitives ───────────────────────────────────

esp_err_t I2CManager::write(i2c_master_dev_handle_t dev, const uint8_t* data, size_t len) {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
#if WE_I2C_DEBUG_VERBOSE
    WE_LOGI("I2C", "write  len=%u", (unsigned)len);
#endif
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        err = i2c_master_transmit(dev, data, len, TIMEOUT_MS);
        if (err == ESP_OK || !isRetryable(err)) break;
        if (attempt + 1 < MAX_RETRIES)
            vTaskDelay(pdMS_TO_TICKS((1 << attempt) * BACKOFF_BASE_MS));
    }
    xSemaphoreGive(s_busMutex);
    if (err != ESP_OK)
        WE_LOGE("I2C", "write len=%u FAILED: %s", (unsigned)len, esp_err_to_name(err));
    return err;
}

esp_err_t I2CManager::read(i2c_master_dev_handle_t dev, uint8_t* data, size_t len) {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
#if WE_I2C_DEBUG_VERBOSE
    WE_LOGI("I2C", "read   len=%u", (unsigned)len);
#endif
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        err = i2c_master_receive(dev, data, len, TIMEOUT_MS);
        if (err == ESP_OK || !isRetryable(err)) break;
        if (attempt + 1 < MAX_RETRIES)
            vTaskDelay(pdMS_TO_TICKS((1 << attempt) * BACKOFF_BASE_MS));
    }
    xSemaphoreGive(s_busMutex);
    if (err != ESP_OK)
        WE_LOGE("I2C", "read  len=%u FAILED: %s", (unsigned)len, esp_err_to_name(err));
    return err;
}

esp_err_t I2CManager::writeReg(i2c_master_dev_handle_t dev, uint8_t reg,
                                const uint8_t* data, size_t len) {
    // Prepend reg byte to data in a stack buffer.
    // 64 bytes covers all in-engine single-register writes.
    uint8_t buf[64];
    if (len + 1 > sizeof(buf)) {
        WE_LOGE("I2C", "writeReg: len=%u exceeds stack buffer", (unsigned)len);
        return ESP_ERR_INVALID_SIZE;
    }
    buf[0] = reg;
    if (len > 0) memcpy(buf + 1, data, len);

    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
#if WE_I2C_DEBUG_VERBOSE
    WE_LOGI("I2C", "writeReg reg=0x%02X len=%u", reg, (unsigned)len);
#endif
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        err = i2c_master_transmit(dev, buf, len + 1, TIMEOUT_MS);
        if (err == ESP_OK || !isRetryable(err)) break;
        if (attempt + 1 < MAX_RETRIES)
            vTaskDelay(pdMS_TO_TICKS((1 << attempt) * BACKOFF_BASE_MS));
    }
    xSemaphoreGive(s_busMutex);
    if (err != ESP_OK)
        WE_LOGE("I2C", "writeReg reg=0x%02X len=%u FAILED: %s", reg, (unsigned)len, esp_err_to_name(err));
    return err;
}

esp_err_t I2CManager::readReg(i2c_master_dev_handle_t dev, uint8_t reg,
                               uint8_t* data, size_t len) {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
#if WE_I2C_DEBUG_VERBOSE
    WE_LOGI("I2C", "readReg  reg=0x%02X len=%u", reg, (unsigned)len);
#endif
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        err = i2c_master_transmit_receive(dev, &reg, 1, data, len, TIMEOUT_MS);
        if (err == ESP_OK || !isRetryable(err)) break;
        if (attempt + 1 < MAX_RETRIES)
            vTaskDelay(pdMS_TO_TICKS((1 << attempt) * BACKOFF_BASE_MS));
    }
    xSemaphoreGive(s_busMutex);
    if (err != ESP_OK)
        WE_LOGE("I2C", "readReg  reg=0x%02X len=%u FAILED: %s", reg, (unsigned)len, esp_err_to_name(err));
    return err;
}

esp_err_t I2CManager::transmitReceive(i2c_master_dev_handle_t dev,
                                       const uint8_t* tx, size_t tx_len,
                                       uint8_t* rx, size_t rx_len) {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
#if WE_I2C_DEBUG_VERBOSE
    WE_LOGI("I2C", "transmitReceive tx=%u rx=%u", (unsigned)tx_len, (unsigned)rx_len);
#endif
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        err = i2c_master_transmit_receive(dev, tx, tx_len, rx, rx_len, TIMEOUT_MS);
        if (err == ESP_OK || !isRetryable(err)) break;
        if (attempt + 1 < MAX_RETRIES)
            vTaskDelay(pdMS_TO_TICKS((1 << attempt) * BACKOFF_BASE_MS));
    }
    xSemaphoreGive(s_busMutex);
    if (err != ESP_OK)
        WE_LOGE("I2C", "transmitReceive tx=%u rx=%u FAILED: %s",
                (unsigned)tx_len, (unsigned)rx_len, esp_err_to_name(err));
    return err;
}

// ── Probe ──────────────────────────────────────────────────

esp_err_t I2CManager::probe(uint8_t addr) {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        err = i2c_master_probe(s_busHandle, addr, TIMEOUT_MS);
        if (err == ESP_OK || !isRetryable(err)) break;
        if (attempt + 1 < MAX_RETRIES)
            vTaskDelay(pdMS_TO_TICKS((1 << attempt) * BACKOFF_BASE_MS));
    }
    xSemaphoreGive(s_busMutex);
    return err;
}

// ── Scan ───────────────────────────────────────────────────

void I2CManager::scan() {
    if (xSemaphoreTake(s_busMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        WE_LOGE("I2C", "scan: mutex timeout");
        return;
    }
    WE_LOGI("I2C", "scan: probing addresses 0x01-0x7E...");
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (i2c_master_probe(s_busHandle, addr, TIMEOUT_MS) == ESP_OK) {
            WE_LOGI("I2C", "  device at 0x%02X", addr);
            ++found;
        }
    }
    WE_LOGI("I2C", "scan: done — %d device(s) found", found);
    xSemaphoreGive(s_busMutex);
}

// ── Diagnostics ────────────────────────────────────────────

void I2CManager::diagBusState() {
    int sda = gpio_get_level((gpio_num_t)PIN_SDA);
    int scl = gpio_get_level((gpio_num_t)PIN_SCL);
    WE_LOGI("I2C", "diagBusState: SDA(pin%d)=%d SCL(pin%d)=%d bus=%s",
            (int)PIN_SDA, sda,
            (int)PIN_SCL, scl,
            s_busHandle ? "active" : "NOT active");
    if (!sda) WE_LOGW("I2C", "  SDA is LOW — bus may be stuck or slave holding line");
    if (!scl) WE_LOGW("I2C", "  SCL is LOW — possible clock stretch or wiring fault");
}
