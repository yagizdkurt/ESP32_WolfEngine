# Architecture

---

## 1. Project Overview

WolfEngine is a hand-written 2D game engine for the **ESP32-S** microcontroller,
designed to drive a handheld game device built around a **128×160 ST7735 LCD**.
It provides the full vertical slice a game needs on constrained hardware: display,
input, audio, physics, UI, and entity management — all without dynamic memory
allocation and without an OS scheduler driving the game loop (it uses a
busy-wait fixed-timestep loop on a FreeRTOS task).

**Current state:** The API surface is not yet stable.

---

## 2. Technology Stack

| Layer | Technology | Version |
|---|---|---|
| MCU | ESP32-S (Xtensa LX7) | — |
| RTOS / SDK | ESP-IDF + FreeRTOS | — |
| Language | C++17 | — |
| Display | ST7735 RGB LCD 128×160 | — |
| Display HAL | ESP-IDF `esp_lcd` | — |
| I²C | ESP-IDF `i2c_master` | — |
| Audio | ESP-IDF `ledc` (PWM) | — |
| IO Expanders | PCF8574 / PCF8575 / MCP23017 | — |
| Build system | PlatformIO [UNCLEAR — no `platformio.ini` inside `WolfEngine/`] | — |

---

## 3. Repository Structure

```
src/WolfEngine/
│
├── WolfEngine.hpp / .cpp          # Engine singleton — init + game loop
│
├── Settings/                      # Compile-time configuration (no runtime config)
│   ├── WE_Settings.hpp            # Master include: pulls all settings headers
│   ├── WE_PINDEFS.hpp             # All GPIO, SPI, I²C pin numbers
│   ├── WE_InputSettings.hpp       # Per-controller button map, expander type, joystick ADC
│   ├── WE_RenderSettings.hpp      # Background color, game region rect, feature flags
│   └── WE_Layers.hpp              # RenderLayer enum + CollisionLayer bitmask enum
│
├── GameObjectSystem/              # Entity base class and registry
│   ├── WE_GameObject.hpp / .cpp   # Base class, factory, lifecycle, component dispatch
│   └── WE_GORegistry.hpp          # Fixed-size pointer array for all live GameObjects
│
├── ComponentSystem/               # ECS components (no ComponentManager — deleted/refactoring)
│   └── Components/
│       ├── WE_BaseComp.hpp        # Abstract base: ComponentType enum, tick()
│       ├── WE_Comp_Transform.hpp  # Position (Vec2) + width/height — always present on GO
│       ├── WE_Comp_SpriteRenderer.hpp / .cpp  # Sprite asset + palette + rotation; auto-registers with RenderCore
│       ├── WE_Comp_Collider.hpp / .cpp        # Box/Circle shapes; collision + trigger layer bitmasks
│       ├── WE_Comp_Animator.hpp / .cpp        # Frame-strip animation driver on top of SpriteRenderer
│       └── WE_Components.hpp      # Convenience include for all components
│
├── Graphics/
│   ├── RenderSystem/
│   │   ├── WE_RenderCore.hpp / .cpp  # Framebuffer owner; layer management; sprite blit + rotation
│   │   └── WE_Camera.hpp / .cpp      # World↔screen transform; follow target; frustum cull
│   ├── SpriteSystem/
│   │   ├── WE_Sprite.hpp             # Sprite asset: pixel data (palette indices), constexpr factory
│   │   ├── WE_SpriteData.hpp         # Render-ready struct passed from SpriteRenderer → RenderCore
│   │   └── WE_SpriteRotation.hpp     # Rotation enum (R0 / R90 / R180 / R270)
│   ├── ColorPalettes/                # Five built-in 32-entry RGB565 palettes; index 0 = transparent
│   │   ├── WE_Palettes.hpp           # Convenience include
│   │   ├── WE_Palette_Grayscale.hpp
│   │   ├── WE_Palette_Warm.hpp
│   │   ├── WE_Palette_Cool.hpp
│   │   ├── WE_Palette_Gameboy.hpp
│   │   └── WE_Palette_Sunset.hpp
│   └── UserInterface/
│       ├── WE_UIManager.hpp / .cpp   # UI root: owns element array, dirty tracking, render dispatch
│       ├── Fonts/
│       │   └── WE_Font.hpp           # 5×7 bitmap font for ASCII 32–126
│       └── UIElements/
│           ├── Base/
│           │   ├── WE_BaseUIElement.hpp / .cpp  # Abstract element: show/hide, dirty flag, drawPixelRaw()
│           │   ├── WE_UITransform.hpp            # Position, size, margins, UIAnchor enum (9 positions)
│           │   └── WE_UITransformHelpers.hpp     # Anchor resolution math (resolveAnchor, resolveLayout)
│           ├── WE_UILabel.hpp / .cpp   # Text label (32-char max, 5×7 font)
│           ├── WE_UIShape.hpp / .cpp   # Rectangle / HLine / VLine, filled or outline
│           ├── UIPanel.hpp / .cpp      # Container with optional background; translates child positions
│           └── WE_UIElements.hpp       # Convenience include
│
├── InputSystem/
│   ├── WE_InputManager.hpp / .cpp  # Owns all Controller instances; shared ADC handle; tick() per frame
│   └── WE_Controller.hpp / .cpp    # Single physical controller: GPIO/expander polling, debounce, ADC joystick
│
├── Physics/
│   ├── WE_ColliderManager.hpp / .cpp  # O(n²) pair iteration; shape intersection; Enter/Stay/Exit dispatch
│
├── Sound/
│   ├── WE_SoundManager.hpp / .cpp  # Two PWM channels (music + SFX); note sequencer; loop + callback
│   └── WE_Notes.hpp                # Note_t frequency enum, octaves 1–8 with semitones
│
├── Drivers/
│   ├── DisplayDrivers/
│   │   ├── WE_Display_Driver.hpp       # Abstract display interface (initialize, flush, backlight, sleep)
│   │   ├── WE_Display_ST7735.hpp / .cpp  # Concrete: SPI @ 40 MHz, DMA flush, FreeRTOS semaphore
│   └── IODrivers/
│       ├── WE_IExpander.hpp            # Abstract expander interface (begin, pinRead)
│       ├── WE_ExpanderDrivers.hpp      # ExpanderType enum + ExpanderSettings struct
│       ├── WE_PCF8574.hpp              # 8-bit quasi-bidirectional expander (0x20–0x27)
│       ├── WE_PCF8575.hpp              # 16-bit variant of PCF8574
│       └── WE_MCP23017.hpp             # Register-based 16-bit expander, Port A + B, pull-ups
│
└── Utilities/
    ├── WE_I2C.hpp / .cpp          # I²C bus singleton: I2C_NUM_0, 400 kHz, scan, reg read/write
    ├── WE_Time.hpp / .cpp         # Wall clock + game clock (pause-aware); since/elapsed/check helpers
    ├── WE_Timer.hpp / .cpp        # Timer struct wrapping WETime; start/stop/reset/check
    ├── WE_Vector2d.hpp            # Vec2 (float) + IntVec2; full operator set; lerp, dist, fromAngle
    ├── WE_EEPROM24LC512.hpp       # 64 KB I²C EEPROM; page-safe burst write; repeated-start read
    └── WE_Debug.h                 # DebugLog/DebugErr macros → ESP_LOGI/LOGE; no-op when disabled
```

**Non-obvious notes:**

- `ComponentSystem/ComponentManager.hpp` is **deleted** in the current working tree.
  The engine tick loop directly iterates component arrays on each `GameObject`. A
  centralised manager was apparently removed or never finished.
- `WE_GORegistry.hpp` declares a pointer registry but is **not wired into the engine**
  in the files read — status unclear.
- The `Drivers/` hierarchy is a **reorganisation in progress**: the old
  `Graphics/RenderSystem/DisplayDrivers/` path still exists in git history but was
  deleted and replaced with `Drivers/DisplayDrivers/`.

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
- **Virtual dispatch for components:** Components share a base but are ticked by
  type-cast direct calls. Avoids vtable overhead per-object per-frame.

---

## 5. Entry Points & Bootstrapping

There is exactly one entry point.

**File:** [src/main.cpp](../main.cpp)  
**ESP-IDF entry:** `extern "C" void app_main()`

```
app_main()
  │
  ├─ Engine().StartEngine()
  │     ├─ WEI2C::getInstance().init()          // I²C bus @ 400 kHz
  │     ├─ DisplayDriver::initialize()           // SPI bus, ST7735 reset, init sequence
  │     ├─ WEInputManager::init()               // GPIO config for all controllers + ADC
  │     ├─ WESoundManager::init()               // LEDC timer + channels for music/SFX pins
  │     ├─ WECamera::init()                     // Sets game region, default zoom
  │     └─ WEUIManager::initialize()            // Clears element list
  │
  ├─ UI().setElements(uiElements[])             // Register null-terminated UI element array
  │
  └─ Engine().StartGame()                       // → main loop (see §7 Flow A)
```

To invoke: flash the firmware via PlatformIO. `app_main` is called by the FreeRTOS
scheduler after the ESP-IDF startup sequence.

---

## 6. Core Modules & Responsibilities

### 6.1 WolfEngine

**Why it exists:** Owns system initialisation order and runs the fixed-timestep game
loop. Without it nothing starts and nothing ticks.

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

**Why it exists:** Owns the RGB565 framebuffer and arbitrates all pixel writes. Provides
the layered compositing model so game objects, background, and FX can be authored
independently without z-order bugs.

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

**Why it exists:** Decouples world-space game logic from screen-space pixel positions.
Enables scrolling levels larger than the 128×160 screen.

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

**Why it exists:** Provides the entity abstraction. Handles lifecycle (`Start` once,
`Update` every frame) and the ECS glue (component ownership + ticking).

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

**Why it exists:** Separates reusable behaviours (rendering, collision, animation) from
game-specific logic in `GameObject` subclasses.

**Components:**

| Component | Purpose |
|---|---|
| `TransformComponent` | Position + size. Always on every GO. |
| `SpriteRendererComponent` | Binds a `Sprite` asset + palette to a render slot. Auto-registers/deregisters with `RenderCore`. |
| `ColliderComponent` | Box or circle shape; collision + trigger layer bitmasks; offset from transform origin. |
| `AnimatorComponent` | Drives `SpriteRenderer` frame index over time; play/pause; per-animation frame duration. |


---

### 6.6 ColliderManager

**Why it exists:** Centralises all collision detection so individual `GameObjects` don't
need to know about each other.

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

**Why it exists:** Abstracts the hardware diversity of input (direct GPIO, three
different I²C expander chips, analog joystick via ADC) behind a uniform button/axis API.

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

**Why it exists:** Provides a retained-mode UI layer rendered on top of the game
framebuffer, with anchor-based layout so positions don't need to be hard-coded to
screen pixels.

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

**Why it exists:** Drives background music and sound effects over two independent PWM
channels without blocking the game loop.

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

**Why it exists:** Decouples the render pipeline from the specific LCD controller chip.

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

**Why it exists:** ESP32-S has limited GPIO. Buttons are read through I²C expanders to
free pins for SPI (display) and audio.

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

## 7. Data Flow & Key Interactions

### Flow A — Frame render cycle

```
1. StartGame() busy-waits until ENGINE_FRAME_TIME (33 333 µs at 30 fps) has elapsed.

2. WEInputManager::tick()
   └─ For each controller: poll GPIO or I²C expander pins
      → debounce → update button state bitmasks
      → read ADC channels → normalise joystick axes

3. WETime::tick()  — advances game clock, increments frameCount

4. For each active GameObject in registry:
   └─ GO.Update()
      ├─ calls user game logic (virtual override)
      └─ calls tick() on each enabled Component

5. WEColliderManager::check()
   └─ For every pair (i, j) of registered colliders:
      a. Apply layer bitmask filter — skip if no interaction
      b. Run shape intersection test (AABB / circle / mixed)
      c. Compare result to previous-frame state
      d. Fire OnCollisionEnter / Stay / Exit  or  OnTriggerEnter / Stay / Exit
         directly on both GameObjects

6. WEUIManager::render()  [only if any element is dirty]
   └─ For each UIElement: call element.draw() → writes pixels into framebuffer region

7. WERenderCore::render()
   └─ Clear framebuffer (if WE_CLEAR_FRAMEBUFFER defined)
   └─ For each layer (BackGround → FX):
      For each registered SpriteData on this layer:
        a. Camera::worldToScreen() → pixel position
        b. Camera::isVisible() → skip if off-screen
        c. drawSprite(): for each pixel, lookup palette index →
           if index == 0 skip (transparent)
           else write RGB565 to framebuffer with rotation remap
   └─ DisplayDriver::flush(framebuffer)  → DMA to ST7735 → semaphore wait
```

### Flow B — Button press to game response

```
1. Hardware: player presses button → GPIO pin goes LOW (or expander register bit clears)

2. WEInputManager::tick() (runs every frame):
   └─ WEController::readRaw() reads GPIO or expander register
   └─ Per-button debounce: if state changed and debounce interval elapsed,
      update currentState bitmask, record timestamp

3. WEController tracks:
   - prevState (last frame bitmask)
   - currentState (this frame bitmask)
   getButton()     = currentState bit set
   getButtonDown() = bit set in current AND NOT prev
   getButtonUp()   = bit set in prev AND NOT current

4. In GameObject::Update() (user code):
   if (Input().getController(0).getButtonDown(Button::A)) {
       // fire weapon, jump, etc.
   }

5. Resulting state change (e.g. spawn projectile) creates a new GameObject via
   GameObject::Create<Projectile>() — placed into registry slot, Start() called
   next frame, Update() participates in subsequent frames.
```

### Flow C — Sprite animation

```
1. AnimatorComponent::tick() runs inside its owning GameObject's Update().

2. Checks WETime::since(lastFrameTime) against frameDuration.

3. If elapsed: advances frameIndex, calls SpriteRendererComponent::setFrame(index).

4. SpriteRenderer updates the SpriteData pointer (pixel array offset).

5. RenderCore reads the updated SpriteData pointer on next render() call —
   no explicit notification needed.
```

---

## 8. Data Layer

WolfEngine has no database and no OS-level file system.

| Storage | Technology | Usage |
|---|---|---|
| Runtime state | Static/stack arrays | All GameObjects, colliders, sprites, UI elements |
| Persistent save data | 24LC512 I²C EEPROM | 64 KB, page-safe writes via `WE_EEPROM24LC512` |
| Framebuffer | Static `uint16_t[]` | RGB565, `screenWidth × screenHeight` words |

**Allocation strategy:** No `new`/`delete` is used anywhere in the engine. All
collections are fixed-size arrays sized by constants in `WE_Settings.hpp`
(`MAX_GAME_OBJECTS`, `MAX_COLLIDERS`, etc.). This is a deliberate choice to eliminate
heap fragmentation on a device with ~520 KB of SRAM.

**EEPROM access:** `readBytes()` uses repeated-start I²C for atomic address-then-data
in one transaction. `writeBytes()` splits writes at 128-byte page boundaries
automatically and waits 5 ms between pages for the write cycle.

---

## 9. External Dependencies & Integrations

| Service / Library | Purpose | How integrated | Failure impact |
|---|---|---|---|
| ESP-IDF `esp_lcd` | ST7735 SPI panel ops | Linked library; `WE_Display_ST7735.cpp` | Black screen |
| ESP-IDF `driver/spi_master` | SPI bus for display | Init in `StartEngine()` | Black screen |
| ESP-IDF `driver/i2c_master` | I²C for expanders + EEPROM | `WEI2C` singleton init | Input loss; save data unavailable |
| ESP-IDF `driver/ledc` | PWM square-wave audio | `WESoundManager::init()` | Silent game |
| ESP-IDF `driver/adc` | Joystick ADC reads | `WEInputManager::init()` | Joystick axes dead |
| ESP-IDF `esp_log` | Debug serial output | Macro in `WE_Debug.h` | No effect on game |
| FreeRTOS | Binary semaphore for DMA flush sync | `xSemaphoreCreateBinary` in ST7735 driver | Display torn / hung |

No network services, no cloud APIs, no MQTT, no Bluetooth used in the engine layer.

---

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

This section documents what a new engineer will likely stumble on.

| Issue | Severity | Detail |
|---|---|---|
| **API in active flux** | High | Several files deleted from working tree (`ComponentManager`, old display driver, old input manager, old sound core). Public interfaces will change. Do not build stable game code on top of deleted paths. |
| **`WE_GORegistry` not wired** | Medium | `WE_GORegistry.hpp` defines a registry struct but it is not referenced in `WolfEngine.cpp` or the game loop. Either the engine iterates a different internal list, or this is dead code awaiting integration. |
| **Hardcoded camera lerp** | Low | Camera follow interpolation factor is a literal constant inside `WE_Camera.cpp`, not exposed in `WE_Settings.hpp`. |
| **Coarse UI dirty tracking** | Low | `UIManager::render()` redraws all elements when any element is dirty. For a 128×160 screen this is tolerable, but will become visible if the UI element count grows. |
| **O(n²) collision detection** | Low | Fine at `MAX_COLLIDERS = 64`. Would need a spatial hash or broad-phase if the limit ever rises significantly. |
| **No audio volume / mixing** | Low | Two PWM channels at fixed duty cycle. No fade-in/out, no relative volume between music and SFX. |
| **No scene / state management** | Low | There is no `Scene`, `GameState`, or `StateMachine` abstraction. The user manages game states manually in game code. A growing game will need to invent this. |
| **Sprite pixel data is `const uint8_t*`** | Low | Sprites are ROM assets only. No runtime-generated sprites or partial updates without a design change to `WE_Sprite`. |
| **No error handling on I²C** | Low | `WEI2C` returns `esp_err_t` but callers (expanders, EEPROM) do not consistently propagate or log errors. A loose connector will produce silent failures. |
| **5 ms EEPROM write delay is blocking** | Low | `writeBytes()` calls `vTaskDelay(pdMS_TO_TICKS(5))` between pages. Writing more than ~128 bytes will stall the game loop for perceptible time. Write save data only outside active gameplay or from a background task. |
