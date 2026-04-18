# WolfEngine

> A lightweight 2D game engine for ESP32 — built for tiny screens, big ideas, and people who like soldering things at 2am.

![WolfEngine](IconsPhotosEtc/icon.png)

WolfEngine is a from-scratch C++ game engine targeting the ESP32 microcontroller. It ships with a built-in ST7735 TFT driver but is designed around a clean display driver interface — plug in your own driver and it works with any SPI display. It gives you a proper game loop, sprite rendering, palette-based colors, a UI system, camera, and input handling — all in a package that fits on a $5 microcontroller with 520KB of RAM.

Built with PlatformIO and ESP-IDF. No Unity. No Godot. Just you, your ESP32, and a tiny screen showing your tiny game.

---

## ✨ What It Can Do

- 🎮 **GameObject & Component system** — component-based GameObjects with a clean `Start()` / `Update()` lifecycle. Transform is built in, attach components to build your dreams.
- 🖼️ **Sprite rendering** — palette-indexed pixel art with rotation, layer-based draw order, and automatic registration with the renderer. Four snap rotations (0°, 90°, 180°, 270°) applied per-pixel at draw time with zero matrix math overhead. Sprites outside the visible area are culled before drawing.
- 🎬 **Animation** — frame-by-frame sprite animation via the `Animator` component. Supports pause, resume, runtime frame swapping, and configurable playback speed — all at zero heap cost.
- 🎨 **Palette system** — 5 built-in palettes (Grayscale, Warm, Cool, GameBoy, Sunset), each with 31 usable colors. Swap palettes at runtime for damage flash, color variants, day/night effects — it's just a pointer swap. Custom palettes are supported!
- 🕹️ **Input & Multiplayer** — up to 4 simultaneous local controllers, each independently configured with its own buttons, joystick, and I2C expander. Buttons via direct GPIO or expanders (PCF8574, PCF8575, MCP23017), analog joystick with calibration and dead zone, software debouncing built in.
- 🔊 **Sound** — supports 2 seperate buzzers for music and sfx. Has note system built in. Using PWM to generate 8bit style musics and sfx. Looping and callbacks included!
- 💾 Save/Load System (EEPROM) — More info at modules section.
- 📷 **Camera** — smooth follow targeting, configurable follow speed, zoom, and world↔screen coordinate conversion. Sprites outside the visible area are culled before drawing.
- 🖥️ **UI system** — text labels anchored to a dedicated UI region at the bottom of the screen. Dirty-flag system means the UI region is only redrawn and flushed when something actually changes.
- ⚡ **Partial screen flush** — when the UI hasn't changed, only the game region is sent over SPI. Less data, faster frames.
- 🔌 **Custom display driver** — built-in ST7735 support out of the box, but the renderer talks to a `DisplayDriver` interface. Swap in your own driver for any display you like.
- 🧠 **Flash-friendly memory model** — sprite pixel data and palettes live in flash as `constexpr` arrays. Zero RAM cost for graphics data.
- 🧩 **Module System** — optional engine subsystems you opt into at compile time. Pay only for what you use.

## 🧩 Current Modules
- 💾 Save/Load System (EEPROM) — flexible EEPROM-backed persistence with support for custom partition tables. Define your own data layouts using structs, and let the SaveLoadManager handle serialization and deserialization automatically. Designed for predictable memory usage and fast access, with full control over how and where your data is stored.

---

## 📖 Documentation

Full documentation is available on the https://yagizdkurt.github.io/WolfEngine/
---

## 📊 Memory Usage

WolfEngine is designed to be memory efficient. Below is the memory footprint on a standard ESP32 (320KB usable RAM, 1MB app partition).

### Empty Project vs With Engine

| | Empty Project | With WolfEngine | Engine Cost |
|---|---|---|---|
| RAM | 10.8 KB (3.3%) | 55.6 KB (17%) | +44.8 KB |
| Flash | 160 KB (15.3%) | 240 KB (23%) | +80 KB |

Empty build with just empty main:
![Empty RAM](IconsPhotosEtc/EmptyRam.png)

Build with Wolfengine initialization:
![Engine RAM](IconsPhotosEtc/EngineRam.png)

### Where does the RAM go?

Of the 44.8KB the engine adds, **42.4KB is the framebuffer** (`128 × 160 × 2 bytes`). This is the unavoidable cost of driving a color display — every pixel on screen needs to live somewhere in RAM. The engine itself, all its systems, registries, and state combined cost only **~2.4KB**.

### Flash-Friendly by Design

Sprite pixel data, palettes, animations, and settings are all declared `constexpr` — they live in flash, not RAM. No matter how many sprites or palettes your game has, the RAM cost stays flat. Only runtime state (game objects, component data, timers) uses RAM. 

### Plenty of Headroom - From 1 MB to 16 MB of flash

With 17% RAM used and 23% flash used, there is substantial room for game content, assets, and logic. At 30fps with a full sprite system, input handling, UI, sound, and camera — WolfEngine leaves ~270KB of RAM free for your game. Also consider testing esp only has 1 MB of flash which most esp32 boards have 4 to 16, this leaves you alot of room for game assets.

---

## 🗺️ Roadmap

- [x] Multiplayer support  
  - Multiple IO expanders for multiple controllers
  - 4 controller support
  - More IO Drivers!

- [ ] More UI Elements
  - More UI elements like textbox, circle panel, scrolls.
  - Menu UI support like selectable buttons.
  - UI edge helper for better looking game/UI screen split.

- [X] Save/Load manager
  - Saving and loading support with EEPROM.

- [ ] Colliders and detection
  - Collision detection with OnCollide() checkers.
  - Collider element that stops objects from passing each other.

- [ ] Hardware settings
  - Most common ESP-32 boards have around 5kb ram and 4mb flash. Which is the "default of engine". This is why we mostly used palettes, static sprites etc. Yet some boards have larger ram and you may want to develop using dynamic variables. Thus we want to add a compiler spesified setting like RAM_LOW RAM_HIGH definition to make dynamic sprites etc possible.

- [ ] More screen drivers
  - At the moment the engine only have 1 default driver with support to user coded drivers. We want to expand the drivers. Which is easy to code but I only have one physical screen at hand so yea...

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

