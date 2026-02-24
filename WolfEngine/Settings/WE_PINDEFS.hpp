#pragma once

//==========================
// DEVICE PIN CONFIGURATIONS
//==========================

// Render spesific
#define RENDER_PIN_CHIPSELECT 17
#define RENDER_PIN_RESET 4
#define RENDER_PIN_DATACOMMAND 16

// spi bus pins
#define SPI_PIN_MOSI 23
#define SPI_PIN_MISO 19
#define SPI_PIN_SCLK 18

// Input pins — GPIO numbers TBD, assign when wiring
// Input pins are moved to WE_Input_Settings.hpp for better organization.

// I2C bus pins
#define I2C_PIN_SDA 21
#define I2C_PIN_SCL 22

// Sound pins
#define MUSIC_PIN 14
#define SFX_PIN 32
