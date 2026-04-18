#include "WE_Display_ST7735.hpp"
#include "WolfEngine/Settings/WE_PINDEFS.hpp"
#include "esp_lcd_st7735.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define MODULE_DEBUG_ENABLED
#include "WolfEngine/Utilities/WE_Debug.h"

static const char *TAG = "ST7735";

#define ST7735_SPI_CLOCK_HZ     (40 * 1000 * 1000)
#define ST7735_SPI_QUEUE_DEPTH  (7)
#define ST7735_CMD_BITS         (8)
#define ST7735_PARAM_BITS       (8)
#define ST7735_BITS_PER_PIXEL   (16)

// ─────────────────────────────────────────────────────────────
//  Flush completion semaphore
//  Given by flushDoneCallback when DMA transfer completes.
//  Taken by flush() before sending the buffer.
// ─────────────────────────────────────────────────────────────
static SemaphoreHandle_t s_flushSem = nullptr;

static bool flushDoneCallback(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t*, void*) {
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(s_flushSem, &woken);
    return woken == pdTRUE;
}

class ST7735Driver : public DisplayDriver {
public:
    ST7735Driver() {
        screenWidth      = 128;
        screenHeight     = 160;
        requiresByteSwap = true;
    }

    void initialize() override {
        if (m_initialized) { DebugLog(TAG, "Display already initialized"); return; }

        esp_err_t ret;
        gpio_config_t io_conf                = {};
        spi_bus_config_t bus_cfg             = {};
        esp_lcd_panel_io_spi_config_t io_cfg = {};
        esp_lcd_panel_dev_config_t panel_cfg = {};

        // --- Semaphore ---
        s_flushSem = xSemaphoreCreateBinary();
        xSemaphoreGive(s_flushSem); // start as available

        // --- GPIO ---
        io_conf.pin_bit_mask = (1ULL << RENDER_PIN_DATACOMMAND) | (1ULL << RENDER_PIN_RESET);
        io_conf.mode         = GPIO_MODE_OUTPUT;
        io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type    = GPIO_INTR_DISABLE;
        ret = gpio_config(&io_conf);
        if (ret != ESP_OK) { DebugErr(TAG, "GPIO config failed: %s", esp_err_to_name(ret)); goto err_gpio; }

        // --- SPI bus ---
        bus_cfg.mosi_io_num     = SPI_PIN_MOSI;
        bus_cfg.miso_io_num     = SPI_PIN_MISO;
        bus_cfg.sclk_io_num     = SPI_PIN_SCLK;
        bus_cfg.quadwp_io_num   = -1;
        bus_cfg.quadhd_io_num   = -1;
        bus_cfg.max_transfer_sz = screenWidth * screenHeight * (ST7735_BITS_PER_PIXEL / 8);
        ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) { DebugErr(TAG, "SPI bus init failed: %s", esp_err_to_name(ret)); goto err_spi_bus; }

        // --- LCD panel IO ---
        io_cfg.dc_gpio_num         = RENDER_PIN_DATACOMMAND;
        io_cfg.cs_gpio_num         = RENDER_PIN_CHIPSELECT;
        io_cfg.pclk_hz             = ST7735_SPI_CLOCK_HZ;
        io_cfg.lcd_cmd_bits        = ST7735_CMD_BITS;
        io_cfg.lcd_param_bits      = ST7735_PARAM_BITS;
        io_cfg.spi_mode            = 0;
        io_cfg.trans_queue_depth   = ST7735_SPI_QUEUE_DEPTH;
        io_cfg.on_color_trans_done = flushDoneCallback;
        io_cfg.user_ctx            = nullptr;
        ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_cfg, &m_io);
        if (ret != ESP_OK) { DebugErr(TAG, "Panel IO init failed: %s", esp_err_to_name(ret)); goto err_panel_io; }

        // --- ST7735 panel ---
        panel_cfg.reset_gpio_num = RENDER_PIN_RESET;
        panel_cfg.rgb_endian     = LCD_RGB_ENDIAN_RGB;
        panel_cfg.bits_per_pixel = ST7735_BITS_PER_PIXEL;
        ret = esp_lcd_new_panel_st7735(m_io, &panel_cfg, &m_panel);
        if (ret != ESP_OK) { DebugErr(TAG, "Panel create failed: %s", esp_err_to_name(ret)); goto err_panel; }

        // --- Panel bring-up ---
        ret = esp_lcd_panel_reset(m_panel);
        if (ret != ESP_OK) { DebugErr(TAG, "Panel reset failed: %s", esp_err_to_name(ret)); goto err_panel; }

        ret = esp_lcd_panel_init(m_panel);
        if (ret != ESP_OK) { DebugErr(TAG, "Panel init failed: %s", esp_err_to_name(ret)); goto err_panel; }

        esp_lcd_panel_invert_color(m_panel, false);
        esp_lcd_panel_mirror(m_panel, true, true);
        esp_lcd_panel_set_gap(m_panel, 0, 0);
        esp_lcd_panel_disp_on_off(m_panel, true);

        m_initialized = true;
        DebugLog(TAG, "ST7735 initialized successfully");
        return;

        // --- Cleanup on failure ---
        err_panel:
            esp_lcd_panel_del(m_panel);  m_panel = NULL;
            esp_lcd_panel_io_del(m_io);  m_io    = NULL;
        err_panel_io:
            spi_bus_free(SPI2_HOST);
        err_spi_bus:
        err_gpio:
            return;
    }

    void flush(const uint16_t* framebuffer, int x1, int y1, int x2, int y2) override {
        if (!m_initialized) { DebugErr(TAG, "Flush called before initialization"); return; }
        if (!framebuffer)   { DebugErr(TAG, "Framebuffer is NULL"); return; }

        xSemaphoreTake(s_flushSem, portMAX_DELAY);
        uint16_t* buf = const_cast<uint16_t*>(framebuffer) + y1 * screenWidth + x1;
        esp_err_t ret = esp_lcd_panel_draw_bitmap(m_panel, x1, y1, x2, y2, buf);
        if (ret != ESP_OK) { DebugErr(TAG, "draw_bitmap failed: %s", esp_err_to_name(ret)); }
    }

    void sleep(bool enable) override {
        if (!m_initialized) { DebugErr(TAG, "Sleep called before initialization"); return; }

        esp_err_t ret = esp_lcd_panel_disp_on_off(m_panel, !enable);
        if (ret != ESP_OK) { DebugErr(TAG, "disp_on_off failed: %s", esp_err_to_name(ret)); return; }

        DebugLog(TAG, "Display sleep %s", enable ? "enabled" : "disabled");
    }

    ~ST7735Driver() {
        if (m_panel)    { esp_lcd_panel_del(m_panel); }
        if (m_io)       { esp_lcd_panel_io_del(m_io); spi_bus_free(SPI2_HOST); }
        if (s_flushSem) { vSemaphoreDelete(s_flushSem); }
    }

private:
    esp_lcd_panel_handle_t    m_panel       = NULL;
    esp_lcd_panel_io_handle_t m_io          = NULL;
    bool                      m_initialized = false;
};

DisplayDriver* GetDriver() {
    static ST7735Driver instance;
    return &instance;
}
