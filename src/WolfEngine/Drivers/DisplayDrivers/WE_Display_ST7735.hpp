#pragma once
#include "WE_Display_Driver.hpp"

// =============== ST7735 Driver Implementation ===============
// This driver is for the ST7735 128x160 SPI display. It uses the ESP-IDF LCD API for interfacing with the display.
#define RENDER_SCREEN_WIDTH  128
#define RENDER_SCREEN_HEIGHT 160

// Returns a pointer to the ST7735 driver
DisplayDriver* GetDriver();