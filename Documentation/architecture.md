# Architecture

This file provides contextual information to assist AI tools in understanding the project’s design decisions and constraints, helping ensure accurate and consistent contributions.
For comprehensive documentation intended for human readers, refer to documentation.md.

---

## 1. Overview

WolfEngine is a 2D game engine targeting the **ESP32** (Xtensa LX7).
No heap allocation (`new`/`delete` forbidden). No STL containers.
Single fixed-timestep busy-wait loop on one FreeRTOS task.

**Current state:** API not yet stable.

---

## 2. Technology Stack

| Layer | Technology | Version |
|---|---|---|
| MCU | ESP32-S (Xtensa LX7) | — |
| RTOS / SDK | ESP-IDF + FreeRTOS | — |
| Language | C++17 | — |
| I²C | ESP-IDF `i2c_master` | — |
| Audio | ESP-IDF `ledc` (PWM) | — |
| IO Expanders | PCF8574 / PCF8575 / MCP23017 | — |
| Build system | PlatformIO | — |
| Display | ST7735 RGB LCD 128×160 (default; swappable via `WE_Display_Driver`) | — |

---

## 3. Repository Structure

Project structure is in Structure.md file. Check it to understand project structure.

---

## 4. Architecture Style & Key Patterns

**Overall style:** Embedded layered monolith. Everything compiles into a single firmware
image. There are no processes, no IPC, no network services.

### Design patterns in use

| Pattern | Where | Detail |
|---|---|---|
| Singleton | `WolfEngine`, `WEInputManager`, `WECamera`, `WEUIManager`, `WESoundManager`, `WEI2C` | Accessed via global free functions (`Engine()`, `Input()`, etc.) |
| Entity-Component System | `GameObjectSystem` + `ComponentSystem` | Lightweight: no archetype tables, no dynamic dispatch via vtable arrays — components are owned by the GO and ticked via direct method calls |
| Factory method | `GameObject::Create<T>()`, `Collider::Box()`, `Collider::Circle()`, `Sprite::Create()` | Hides construction details; `Create<T>` placement-news into a registry slot |
| Abstract interface | `WE_Display_Driver`, `WE_IExpander` | Lets driver selection be a compile-time `#if` in settings |
| Dirty flag | `WEUIManager`, `BaseUIElement` | UI skips redraw when nothing has changed |
| Triangular bitmask | `WEColliderManager` | Tracks per-pair collision state in O(n²/2) bits without a hash map |
| Placement new | `WEController` for expander objects | Avoids heap; expander constructed in a `uint8_t` buffer sized to the largest concrete type |
| Constexpr validation | `Sprite::Create()` | Illegal sprite dimensions caught at compile time, not runtime |

### What is intentionally avoided (inferred)

- **Heap allocation:** No `new`/`delete` observed in engine code. All fixed-size arrays.
  Reason: heap fragmentation is fatal on embedded targets with limited SRAM.
- **STL containers:** No `std::vector`, `std::map`, etc. Reason: code-size and
  heap dependency.

---

## 5. Entry Points & Bootstrapping

**File:** [src/main.cpp](../main.cpp)

```
app_main()
  │
  ├─ Engine().StartEngine()
  │     ├─ WEI2C::getInstance().init()
  │     ├─ DisplayDriver::initialize()
  │     ├─ WEInputManager::init()
  │     ├─ WESoundManager::init()
  │     ├─ WECamera::init()
  │     └─ WEUIManager::initialize()
  │
  ├─ UI().setElements(uiElements[])
  │
  └─ Engine().StartGame()
```

---

## 6. Core Modules & Responsibilities

### 6.1 WolfEngine

**Public interface:**
```cpp
WolfEngine& Engine();          // global accessor
void StartEngine();            // one-time hardware init
void StartGame();              // blocking game loop
```

**Key dependencies:** All other subsystems (renderer, camera, input, UI, sound,
colliders).

---

### 6.2 RenderCore

**Public interface:**
```cpp
void registerSprite(SpriteData*, RenderLayer);
void unregisterSprite(SpriteData*);
void render();                 // composites all layers → calls display flush
```

**Key design choices:**
- Framebuffer is a flat `uint16_t` array of `width × height` pixels.
- Five fixed layers (`BackGround → World → Entities → Player → FX`) drawn in order;
  higher layers paint over lower.
- Index 0 in any palette is transparent — `drawSprite()` skips those pixels.
- Per-pixel bounds checking clips sprites to the camera's game region rectangle.
- Rotation (0/90/180/270°) implemented by remapping pixel coordinates at blit time —
  no intermediate buffer.


---

### 6.3 Camera

**Public interface:**
```cpp
WECamera& MainCamera();        // global accessor
void setPosition(Vec2);
void setFollowTarget(Vec2*);   // pointer to a Vec2 updated each frame
Vec2 worldToScreen(Vec2);
bool isVisible(Vec2, float w, float h);
```

**Key design choices:**
- Follow uses linear interpolation each frame (lerp factor hardcoded — not in settings).
- Culling check lets `RenderCore` skip sprites outside the viewport.


---

### 6.4 GameObjectSystem

**Public interface:**
```cpp
// Static factory — T must inherit GameObject
template<typename T, typename... Args>
static T* GameObject::Create(Args&&...);

void DestroyGameObject(GameObject*);  // deferred destruction

// Per-instance
virtual void Start();
virtual void Update();
bool isActive;

// Collision event hooks (override in subclass)
virtual void OnCollisionEnter(GameObject*);
virtual void OnCollisionStay(GameObject*);
virtual void OnCollisionExit(GameObject*);
virtual void OnTriggerEnter(GameObject*);
// ...etc
```

**Key design choices:**
- `Create<T>()` uses placement new into a slot in the global registry so allocation
  is deterministic.
- `Transform` is always present — it is a member, not an optional component.
- Component `tick()` is called from `GameObject::Update()` by iterating its own
  component list, not a global system.


---

### 6.5 ComponentSystem

**Components:**

| Component | Purpose |
|---|---|
| `TransformComponent` | Position + size. Always on every GO. |
| `SpriteRendererComponent` | Binds a `Sprite` asset + palette to a render slot. Auto-registers/deregisters with `RenderCore`. |
| `ColliderComponent` | Box or circle shape; collision + trigger layer bitmasks; offset from transform origin. |
| `AnimatorComponent` | Drives `SpriteRenderer` frame index over time; play/pause; per-animation frame duration. |


---

### 6.6 ColliderManager

**Public interface:**
```cpp
WEColliderManager& Colliders();   // global accessor (via Engine)
void registerCollider(ColliderComponent*);
void unregisterCollider(ColliderComponent*);
void check();                      // called once per frame by game loop
```

**Key design choices:**
- O(n²/2) pair iteration — fine for `MAX_COLLIDERS` (64).
- Layer filtering: a pair only interacts if `(layerA & maskB) != 0`.
- State machine per pair: Enter fires once, Stay fires every frame while overlapping,
  Exit fires once when separation occurs. Tracked via a triangular packed bitmask.
- Shape tests: Box–Box (AABB), Circle–Circle (distance²), Box–Circle (clamped point).


---

### 6.7 InputManager / Controller

**Public interface:**
```cpp
WEInputManager& Input();                 // global accessor
WEController& getController(uint8_t i);  // 0–3

// On WEController:
bool getButton(Button b);       // held
bool getButtonDown(Button b);   // pressed this frame
bool getButtonUp(Button b);     // released this frame
float getAxis(Axis a);          // -1.0 to 1.0
```

**Key design choices:**
- Each controller is configured entirely by `ControllerSettings` in `WE_InputSettings.hpp`
  — no code changes needed to remap buttons.
- Expander objects are placement-newed into a fixed buffer inside `WEController`;
  type is selected at runtime from `ExpanderType` enum.
- Debounce per-button via timestamp; poll interval is configurable.
- ADC joystick: raw reading normalised against calibration min/mid/max values defined
  in settings.


---

### 6.8 UIManager + UI Elements

**Public interface:**
```cpp
WEUIManager& UI();
void setElements(BaseUIElement** nullTerminated);
void render();   // called by engine each frame; skips if !dirty
```

**Element hierarchy:**

```
BaseUIElement  (show/hide, dirty flag, drawPixelRaw, UITransform)
├─ UILabel     (text string ≤32 chars, 5×7 font, palette color index)
├─ UIShape     (Rectangle / HLine / VLine, filled or outline)
└─ UIPanel     (container with optional background; translates child coords)
```

**Key design choices:**
- `UITransform` uses a `UIAnchor` enum (9 positions) + pixel offset. `resolveLayout()`
  converts anchor + margin to absolute screen coordinates at render time.
- Dirty flag is per-element but UIManager currently re-renders **all** elements when
  any one is dirty (see §12).
- Font is a static 5×7 bitmap array (`WE_Font.hpp`) covering ASCII 32–126.


---

### 6.9 SoundManager

**Public interface:**
```cpp
WESoundManager& Sound();
void playMusic(SoundClip* clips, uint16_t count, bool loop);
void playSFX(SoundClip* clips, uint16_t count, bool loop, void(*onComplete)());
void stopMusic();
void stopSFX();
bool isMusicPlaying();
bool isSFXPlaying();
```

**Key design choices:**
- `SoundClip` is `{Note_t frequency, uint16_t durationMs}`.
- Music and SFX play concurrently on separate LEDC channels (pins defined in
  `WE_PINDEFS.hpp`).
- Sequencing is managed by checking elapsed time each `tick()` call — no RTOS timer.
- No volume control, no waveform mixing (see §12).


---

### 6.10 Display Drivers

**Abstract interface (`WE_Display_Driver.hpp`):**
```cpp
virtual void initialize() = 0;
virtual void flush(uint16_t* framebuffer) = 0;
virtual void setBacklight(uint8_t) {}   // optional
virtual void sleep() {}                 // optional
uint16_t screenWidth, screenHeight;
bool requiresByteSwap;
```

**Concrete: ST7735**
- SPI bus at 40 MHz, configured via `WE_PINDEFS.hpp`.
- Uses ESP-IDF `esp_lcd_panel_io` + `esp_lcd_panel_st7735`.
- `flush()` triggers a DMA transfer; completion is signalled via a FreeRTOS binary
  semaphore so the CPU isn't spinning.
- `requiresByteSwap = true` — the panel expects big-endian RGB565.


---

### 6.11 IO Expander Drivers

| Driver | Chip | Pins | Notes |
|---|---|---|---|
| `WE_PCF8574` | PCF8574 | 8 | Quasi-bidirectional; write-then-read protocol |
| `WE_PCF8575` | PCF8575 | 16 | Same protocol, `uint16_t` data width |
| `WE_MCP23017` | MCP23017 | 16 | Register-based; supports direction + pull-up config |

All implement `WE_IExpander` (`begin()`, `pinRead()`). The concrete type is selected
from `ExpanderSettings.type` at controller init time.

---

### 6.12 Utilities

| Utility | Purpose |
|---|---|
| `WEI2C` | Singleton I²C bus (I2C_NUM_0, 400 kHz). All expander and EEPROM traffic goes through here. |
| `WETime` | Wall clock + pause-aware game clock. `since()`, `elapsed()`, `check()` (auto-reset). |
| `WETimer` | Per-object timer wrapping `WETime`. Use instead of comparing raw microseconds. |
| `Vec2` / `IntVec2` | 2D math. `Vec2` is float; `IntVec2` is integer. `toPixel()` converts between. |
| `WE_EEPROM24LC512` | 64 KB I²C EEPROM. Handles page-boundary writes automatically. Use for save data. |
| `WE_Debug.h` | `DebugLog`/`DebugErr` macros. Zero-cost when `MODULE_DEBUG_ENABLED` is not defined. |

---

## 8. Data Layer

| Storage | Technology | Usage |
|---|---|---|
| Persistent save data | 24LC512 I²C EEPROM | 64 KB, page-safe writes via `WE_EEPROM24LC512` |
| Framebuffer | Static `uint16_t[]` | RGB565, `screenWidth × screenHeight` words |

**EEPROM:** `writeBytes()` splits at 128-byte page boundaries with a 5 ms blocking delay per page. Avoid calling during active gameplay.


## 10. Configuration & Environment

All configuration is **compile-time**. There are no runtime environment variables,
no config files read from flash, and no over-the-air configuration.

### Settings files (all in `Settings/`)

| File | Controls | Change requires |
|---|---|---|
| `WE_Settings.hpp` | Frame rate, MAX_GAME_OBJECTS, display target selection, feature flags | Recompile |
| `WE_PINDEFS.hpp` | All GPIO numbers: SPI, I²C, audio, display DC/Reset/CS | Recompile |
| `WE_InputSettings.hpp` | Per-controller button→pin map, expander type & address, joystick ADC channel & calibration | Recompile |
| `WE_RenderSettings.hpp` | Background color (RGB565), game region rect, framebuffer clear flag | Recompile |
| `WE_Layers.hpp` | `RenderLayer` enum values, `CollisionLayer` bitmask values | Recompile + update all layer assignments |

### Feature flags (in `WE_Settings.hpp`)

| Flag | Effect when defined |
|---|---|
| `WE_USE_ST7735` | Selects ST7735 as display driver (vs custom) |
| `WE_SPRITE_SYSTEM_ENABLED` | Includes sprite registration in render loop |
| `WE_CLEAR_FRAMEBUFFER` | Zeroes framebuffer each frame (disable for overdraw optimisation) |
| `MODULE_DEBUG_ENABLED` (per-file) | Enables `DebugLog`/`DebugErr` output via `esp_log` |

---

## 11. Known Issues, Gaps & Technical Debt

Issues have been moved into Issues.md file.