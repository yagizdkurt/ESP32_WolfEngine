# Pin Definitions

Some GPIO pin assignments live in `WolfEngine/Settings/WE_PINDEFS.hpp`. Edit this file to match your physical wiring before building.

---

## Display (SPI)

```cpp
#define RENDER_PIN_CHIPSELECT   17
#define RENDER_PIN_RESET         4
#define RENDER_PIN_DATACOMMAND  16
```

These are screen control pins. CS, RST, and DC are required — without them the display won't initialize. Keep in mind that different screens use different pins but mostly SPI communication holds.

---

## SPI Bus

```cpp
#define SPI_PIN_MOSI  23
#define SPI_PIN_MISO  19
#define SPI_PIN_SCLK  18
```

Standard SPI bus pins shared by the display.

---

## I2C Bus

```cpp
#define I2C_PIN_SDA  21
#define I2C_PIN_SCL  22
```

Used by I2C devices such as the PCF8574 expander. These are the ESP32 default I2C pins and work well in most cases.

---

## Input Pins

Button GPIO assignments have their own file — see [Input Settings](input.md).

---

## Suggestions

- Pins 21 and 22 are the ESP32 hardware I2C pins — stick with these unless you have a reason to remap.
- Avoid GPIO 6–11 — these are connected to the internal flash on most ESP32 modules and will cause crashes if used.
- Avoid GPIO 34–39 for SPI and I2C — they are input-only and cannot be driven as outputs.
- If you change SPI pins, make sure they are not already claimed by another peripheral.
