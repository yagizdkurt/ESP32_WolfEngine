# Installation & Setup

WolfEngine runs on ESP32 with a display, built with PlatformIO and the ESP-IDF framework. Although you can code and use any display driver, ST7735 TFT 128x160 driver is the engine default.

---

## Requirements

| Tool        | Recommended      |
|-------------|-----------------|
| IDE         | VS Code         |
| Build system| PlatformIO      |
| Framework   | ESP-IDF         |
| Board       | ESP32           |
| Display     | ST7735 TFT 128x160 |

---

## Your First Project

Every WolfEngine project follows the same startup structure:

1. Define your gameobjects, systems, UI etc. → check [GameObject](../gameobjects-and-components/gameobject.md)
2. Initialize your engine.
3. Register/Create your objects/systems.
4. Start the game.

```cpp
#include "WolfEngine/WolfEngine.hpp"

extern "C" void app_main() {
    Engine().StartEngine(); // Engine creation

    // 2. Register your objects, systems, ui etc here -----
 
    Engine().StartGame(); // 3. Start the engine — this call never returns
}
```

The engine handles the game loop, rendering, input polling, and display flushing automatically once `StartGame()` is called.

---

## Next Steps

| | |
|---|---|
| ⚙️ **[Fast Tutorial](first-gameobject.md)** | For people who just want to rush B |
| ⏩ **[Engine Settings](../core-systems/engine.md)** | Configure frame rate, background color, and UI region |
| 📌 **[Pin Setup](../pin-definitions.md)** | Set up your GPIO and SPI wiring |
| 🛰️ **[About GameObjects](../gameobjects-and-components/gameobject.md)** | Learn everything about GameObjects |
| 🎮 **[Inputs!](../core-systems/input.md)** | How to get inputs from the controllers? |
| 🚗 **[Your First Game Object!](first-gameobject.md)** | Learn how to create and move objects |
| 🎨 **[How To Set Up Graphics?](../graphics/how-to-setup-graphics.md)** | Get graphics on screen |
| 🔉  **[How to pew pew?](../core-systems/audio.md)** | Make the buzzer go brr |
