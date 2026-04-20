# Input Settings

All input configuration lives in `WolfEngine/Settings/WE_Settings.hpp`. Edit this file to match your hardware wiring before building.

---

## The Controller

WolfEngine's input system is based on per-controller settings. These are the settings per controller:

### 1. General Setup
* **`enabled`**: Set to `false` to skip polling for a specific slot.
* **`activeLow`**: 
    * `true`: Buttons connect to **GND** (Uses ESP32 internal pull-ups).
    * `false`: Buttons connect to **VCC** (Requires external pull-down resistors).

### 2. Button Mapping (A–J)
WolfEngine supports **10 buttons** per controller. You can map them via direct GPIO or via an I2C Expander.

| Button Slot | Index | Order |
| :--- | :--- | :--- |
| **A – J** | 0 – 9 | `{ A, B, C, D, E, F, G, H, I, J }` |

* **`gpioPins[10]`**: Assign the ESP32 GPIO number. Use `-1` if the button is on an expander or unused.
* **`expander`**: Configure the I/O driver for this controller.
    * **Types**: `ExpanderType::None`, `ExpanderType::PCF8574` (8-bit), `ExpanderType::PCF8575` (16-bit), or `ExpanderType::MCP23017` (16-bit).
    * **`pins[10]`**: Map the physical pin on the expander (e.g., P0-P15) to the engine's A-J slots.

---

## 🕹️ Analog Joysticks
Each controller supports a 2-axis analog stick. Note that joysticks **must** be connected to ADC1 pins to work alongside Wi-Fi.

### ADC Channel Mapping
Use the following `adc_channel_t` values based on your GPIO wiring:

| GPIO | ADC1 Channel |
| :--- | :--- |
| 36 | `ADC_CHANNEL_0` |
| 37 | `ADC_CHANNEL_1` |
| 38 | `ADC_CHANNEL_2` |
| 39 | `ADC_CHANNEL_3` |
| 32 | `ADC_CHANNEL_4` |
| 33 | `ADC_CHANNEL_5` |
| 34 | `ADC_CHANNEL_6` |
| 35 | `ADC_CHANNEL_7` |

### Calibration & Deadzone
Set the `Min`, `Max`, and `Center` values based on raw 12-bit ADC readings (0–4095).
* **`joyDeadzone`**: A float (0.0 to 1.0). If set to `0.1`, any movement within the inner 10% of the stick's range is ignored to prevent drifting.

---

## The INPUT_SETTINGS Struct

Every setting about input lives in this struct. You can edit it at `WolfEngine/Settings/WE_Settings.hpp`.

```cpp
constexpr InputSettings INPUT_SETTINGS = {
    .debounceMs     = 20,
    .pollIntervalMs = 10,
    // ----------------- CONTROLLERS -----------------
    // Configuration for up to 4 controllers (players 1–4).
    .controllers    = {
        {   // -------------- CONTROLLER 1 --------------
            .enabled     = true,
                           // A    B    C    D    E    F    G    H    I    J
            .gpioPins    = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
            .expander    = { ExpanderType::None, -1, {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} },
            .joyXEnabled = true,
            .joyYEnabled = true,
            .joyXChannel = ADC_CHANNEL_0,
            .joyYChannel = ADC_CHANNEL_1,
            .joyXMin = 0, .joyXMax = 4095, .joyXCenter = 2048,
            .joyYMin = 0, .joyYMax = 4095, .joyYCenter = 2048,
            .joyDeadzone = 0.1f,
            .activeLow   = true,
        },
        {   // ------------- CONTROLLER 2 --------------
            .enabled     = true,
                           // A    B    C    D    E    F    G    H    I    J
            .gpioPins    = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
            .expander    = { ExpanderType::None, -1, {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} },
            .joyXEnabled = false,
            .joyYEnabled = false,
            .joyXChannel = ADC_CHANNEL_0,
            .joyYChannel = ADC_CHANNEL_0,
            .joyXMin = 0, .joyXMax = 4095, .joyXCenter = 2048,
            .joyYMin = 0, .joyYMax = 4095, .joyYCenter = 2048,
            .joyDeadzone = 0.1f,
            .activeLow   = true,
        },
        // ...
    }
};
```

---

## 🔌 I/O Expanders

WolfEngine supports I2C expanders to offload button polling, which is essential for multi-controller setups. Expander settings are defined per-controller.

### Supported Hardware
* **PCF8574**: 8-bit (Pins 0–7)
* **PCF8575**: 16-bit (Pins 0–15)
* **MCP23017**: 16-bit (Pins 0–15)

### Address Configuration
The I2C address is determined by the physical wiring of the **A0, A1, and A2** pins on your chip. 

| A2 | A1 | A0 | I2C Address |
|:---:|:---:|:---:|:---:|
| GND | GND | GND | `0x20` (Default) |
| GND | GND | VCC | `0x21` |
| GND | VCC | GND | `0x22` |
| VCC | VCC | VCC | `0x27` |

> **Warning:** These are defaults for most expander chips. Make sure you look at your own chip for addresses.

### Expander Mapping
To use an expander, set the `.type` and `.addr` in the `expander` struct, then map the expander's physical pins to the engine's logical buttons (A–J).

```cpp
.expander = {
    .type    = ExpanderType::MCP23017, // Select your chip
    .addr    = 0x20,                   // Hardware address
    .pins    = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } // Maps expander P0-P9 to buttons A-J
}
```

> **Hybrid Wiring:** You can mix GPIO and Expander pins. If a button has both a gpioPins entry AND an expander.pins entry, the GPIO pin takes priority. Set unused slots to -1.

> **Current behavior note:** `pollIntervalMs` exists in `InputSettings`, but input polling currently runs once per engine tick.
