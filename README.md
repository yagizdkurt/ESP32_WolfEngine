# 🐺 WolfEngine

> A lightweight 2D game engine for ESP32 — built for tiny screens, big ideas, and people who like soldering things at 2am.

![WolfEngine](icon.png)

WolfEngine is a from-scratch C++ game engine targeting the ESP32 microcontroller. It ships with a built-in ST7735 TFT driver but is designed around a clean display driver interface — plug in your own driver and it works with any SPI display. It gives you a proper game loop, sprite rendering, palette-based colors, a UI system, camera, and input handling — all in a package that fits on a $5 microcontroller with 520KB of RAM.

Built with PlatformIO and ESP-IDF. No Unity. No Godot. Just you, your ESP32, and a tiny screen showing your tiny game.

---

## ✨ What It Can Do

- 🎮 **GameObject system** — component-based GameObjects with a clean `Start()` / `Update()` lifecycle. Attach sprites, define behavior, and let the engine drive it all.
- 🧩 **Component system** — build GameObjects by attaching components. Transform is built in, Sprite snaps on with one line, and the system is designed to grow with your game.
- 🖼️ **Sprite rendering** — palette-indexed pixel art with a full layer-based draw order. Sprites register and unregister with the renderer automatically.
- 🔄 **Sprite rotation** — four snap rotations (0°, 90°, 180°, 270°) applied per-pixel at draw time with zero matrix math overhead.
- 🎨 **Palette system** — 5 built-in palettes (Grayscale, Warm, Cool, GameBoy, Sunset), each with 31 usable colors. Swap palettes at runtime for damage flash, color variants, day/night effects — it's just a pointer swap. Custom palettes are a single `constexpr` array away.
- 🕹️ **Input** — buttons via direct GPIO or PCF8574 I2C expander, analog joystick with calibration and dead zone. Software debouncing built in. `getButton()`, `getButtonDown()`, `getButtonUp()` — works like you'd expect.
- 🔊 **Sound** — supports 2 seperate buzzers for music and sfx. Has note system built in. Using PWM to generate 8bit style musics and sfx. Looping and callbacks included!
- 📷 **Camera** — smooth follow targeting, configurable follow speed, zoom, and world↔screen coordinate conversion. Sprites outside the visible area are culled before drawing.
- 🖥️ **UI system** — text labels anchored to a dedicated UI region at the bottom of the screen. Dirty-flag system means the UI region is only redrawn and flushed when something actually changes.
- ⚡ **Partial screen flush** — when the UI hasn't changed, only the game region is sent over SPI. Less data, faster frames.
- 🔌 **Custom display driver** — built-in ST7735 support out of the box, but the renderer talks to a `DisplayDriver` interface. Swap in your own driver for any display you like.
- 🧠 **Flash-friendly memory model** — sprite pixel data and palettes live in flash as `constexpr` arrays. Zero RAM cost for graphics data.

---

## 📖 Documentation

Full documentation is available on the [Wiki](https://github.com/yagizdkurt/WolfEngine/wiki).

| | |
|---|---|
| 🚀 [Getting Started](https://github.com/yagizdkurt/WolfEngine/wiki/Installation-and-Setup) | Install, wire up, and run your first project |
| 🧱 [Game Objects](https://github.com/yagizdkurt/WolfEngine/wiki/Game-Objects) | Creating and managing game entities |
| 🎨 [Palette System](https://github.com/yagizdkurt/WolfEngine/wiki/Palette-System) | How indexed color works |
| 🖼️ [Sprites & Layers](https://github.com/yagizdkurt/WolfEngine/wiki/Sprites-and-Layers) | Getting graphics on screen |
| 🖥️ [UI System](https://github.com/yagizdkurt/WolfEngine/wiki/UIManager) | Labels, dirty rendering, UI region |
| ⚙️ [Settings](https://github.com/yagizdkurt/WolfEngine/wiki/General-Settings) | Configure pins, layers, frame rate |

---

## 🗂️ Project Structure

```
src/
├── main.cpp
└── WolfEngine/
    ├── WolfEngine.hpp
    ├── Core/
    ├── ComponentSystem/
    ├── GameObjectSystem/
    ├── Graphics/
    │   ├── ColorPalettes/
    │   ├── RenderSystem/
    │   └── UserInterface/
    ├── Settings/
    └── Sound/
```

---

## 📜 License

Apache 2.0 License — free to use, modify, and ship in your own projects, personal or commercial.
Just keep the credit. A mention in your readme or about screen is enough.

---

## 🤝 Contributing

Found a bug? Have an idea? PRs and issues are welcome.
This is a hobby project so don't expect enterprise response times — but do expect genuine interest.

---

*Made with too much coffee and a love for tiny hardware. — [@yagizdkurt](https://github.com/yagizdkurt)*

