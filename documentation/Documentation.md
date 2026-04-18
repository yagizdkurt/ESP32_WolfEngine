# Documentation

---

## 1. Project Overview

WolfEngine is a 2D game engine for the **ESP32** microcontroller,
designed to drive a handheld game device built around a TFT screen.

**Current state:** The API surface is not yet stable.

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

---

## 3. Repository Structure
Repository structure is in Structure.md for convenience to maintainers. You can check it from there.

---

## 4. Architecture Style & Key Patterns

### Design patterns in use

| Pattern | Where | Detail |
|---|---|---|
| Singleton | `WolfEngine`, `WEInputManager`, `WECamera`, `WEUIManager`, `WESoundManager`, `WEI2C` | Accessed via global free functions (`Engine()`, `Input()`, etc.) |
| Static module system with priority dispatch | `ModuleSystem`, `WE_ModuleSystem.cpp`, `WE_Modules.hpp` | Optional engine subsystems are listed in `WE_ModuleSystem.cpp` under `#if defined()` guards; `ModuleSystem` sorts them by priority and calls their lifecycle hooks at the right points |
| Entity-Component System | `GameObjectSystem` + `ComponentSystem` | Lightweight: no archetype tables, no dynamic dispatch via vtable arrays — components are owned by the GO and ticked via direct method calls |
| Factory method | `GameObject::Create<T>()`, `Collider::Box()`, `Collider::Circle()`, `Sprite::Create()` | Hides construction details; `Create<T>` placement-news into a registry slot |
| Abstract interface | `WE_Display_Driver`, `WE_IExpander`, `WE_IEEPROMDriver` | Lets driver selection be decoupled from the system that uses it |
| Dirty flag | `WEUIManager`, `BaseUIElement` | UI skips redraw when nothing has changed |
| Triangular bitmask | `WEColliderManager` | Tracks per-pair collision state in O(n²/2) bits without a hash map |
| Placement new | `WEController` for expander objects; `WE_SaveManager` for EEPROM drivers | Avoids heap; driver constructed in a `uint8_t` buffer sized to the largest concrete type |
| Constexpr validation | `Sprite::Create()`, `WE_SaveManager` slot guards | Illegal values caught at compile time, not runtime |

- **Heap allocation:** No dynamic allocation anywhere in the engine. Module instances are file-scope `static` objects; the module list is a plain C array of pointers.
- **STL containers:** Entirely avoided — no `std::vector`, no `std::sort`. The engine uses fixed-size arrays and manual algorithms throughout.

---

## 5. Entry Points & Bootstrapping

```
app_main()
  │
  ├─ Engine().StartEngine()
  │     ├─ Driver initialization
  │     ├─ Engine CORE subsystem initialization
  │     ├─ Engine Modules initialization
  │     └─ UI().setElements(uiElements[])
  │
  └─ Engine().StartGame()
      └─ gameTick()
```

---

## 6. Core Modules & Responsibilities

### 6.1 WolfEngine

**Why it exists:** Owns system initialisation order and runs the fixed-timestep game
loop. Without it nothing starts and nothing ticks.

**Public interface:**
```cpp
WolfEngine& Engine();          // global accessor
void StartEngine();            // one-time hardware init → core subsystems → ModuleRegistry::InitAll()
void StartGame();              // blocking game loop
```

**Frame tick order (`gameTick()`):**
1. `InputManager::tick()` — poll buttons / joysticks
2. `ModuleSystem::UpdateAll()` — tick all registered modules
3. `GameObject::componentTick()` for each active object — component logic before game logic
4. `GameObject::Update()` for each active object — user game logic
5. `ColliderManager::tick()` — collision detection + events
6. `Camera::followTick()` — camera follow after movement
7. `Renderer::render()` — composite framebuffer + DMA flush

**Key dependencies:** All core subsystems (renderer, camera, input, UI, sound, colliders)
plus `ModuleRegistry` for optional modules.

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
| `WE_Debug.h` | `DebugLog`/`DebugErr` macros. Zero-cost when `MODULE_DEBUG_ENABLED` is not defined. |

---

### 6.13 Module System

**Why it exists:** Provides a structured way to add optional engine subsystems that need `OnInit()` / `OnUpdate()` / `OnShutdown()` lifecycle hooks. All modules are listed in one place (`WE_ModuleSystem.cpp`) under compile-time feature flags; the engine calls them in priority order.

**Key files:**

| File | Role |
|---|---|
| `Modules/WE_IModule.hpp` | `IModule` base all modules must inherit; `TModule<T, Priority>` CRTP helper |
| `Modules/WE_ModuleSystem.hpp` | `ModuleSystem` class with `InitAll/UpdateAll/ShutdownAll` |
| `Modules/WE_ModuleSystem.cpp` | All module instances and the pointer list live here |
| `Settings/WE_Modules.hpp` | Compile-time feature flags (`#define WE_MODULE_SAVELOAD`, etc.) |

**`TModule<T, Priority>` — base for all modules:**
```cpp
// Inherit this instead of IModule directly.
// Priority: higher value = initialised first, shutdown last.
template<typename T, int Priority>
class TModule : public IModule {
public:
    static T& Get();   // returns the one instance
protected:
    TModule(const char* name);
    virtual void OnInit()     {}
    virtual void OnUpdate()   {}
    virtual void OnShutdown() {}
};
```

**`ModuleSystem` — called only by the engine:**
```cpp
ModuleSystem::InitAll();       // sorts by priority (descending), then OnInit() — once in StartEngine()
ModuleSystem::UpdateAll();     // OnUpdate() each module — once per frame in gameTick()
ModuleSystem::ShutdownAll();   // OnShutdown() in reverse priority order
```

**Adding a new module — step by step:**
1. Create your class inheriting `TModule<MyModule, Priority>` and implement whichever hooks you need.
2. Add a feature flag in `Settings/WE_Modules.hpp`:
   ```cpp
   #define WE_MODULE_MYMODULE
   ```
3. In `WE_ModuleSystem.cpp`, add your instance and pointer under the same guard:
   ```cpp
   #if defined(WE_MODULE_MYMODULE)
   #include "MyModule/WE_MyModule.hpp"
   static WE_MyModule s_myModule;
   #endif

   static IModule* s_modules[] = {
   #if defined(WE_MODULE_MYMODULE)
       &s_myModule,
   #endif
       // ...
   };
   ```
4. Comment out the `#define` in `WE_Modules.hpp` to disable the module — it will not be compiled or linked.

**Accessing a module from game code:**
```cpp
WE_SaveManager::Get().write(SAVE_SLOT_0, myData);
```

---

### 6.14 SaveManager

**Why it exists:** Provides a safe, structured API for reading and writing persistent game data to I²C EEPROM without exposing the caller to page boundaries, address arithmetic, multi-chip routing, or raw byte layouts.

**Registered as a module via:** `WE_ModuleSystem.cpp` (guarded by `#if defined(WE_MODULE_SAVELOAD)`).

**Quick setup:**
1. Enable in `Settings/WE_Modules.hpp`: `#define WE_MODULE_SAVELOAD`
2. Open `Settings/WE_SaveSettings.hpp`
3. Add your EEPROM chip to `WE_SAVE_EEPROMS[]` (address + driver type)
4. Add a named entry to the `SaveSlot` enum
5. Add a matching `SaveSlotDef` entry in `SAVE_SLOTS[]` with the size of your struct

**Accessing from game code:**
```cpp
WE_SaveManager& save = WE_SaveManager::Get();
```

**Public interface:**
```cpp
// Write — call between levels or on pause, NOT every frame
// Blocks 5–20 ms per 128-byte page while the chip writes internally
template<typename T>
esp_err_t write(SaveSlot slot, const T& data);

// Read — fast (~1 ms), safe to call at any time
template<typename T>
esp_err_t read(SaveSlot slot, T& outData);

esp_err_t erase(SaveSlot slot);    // reset one slot to 0xFF
esp_err_t eraseAll();              // erase all chips — do NOT call during gameplay
```

**Handling read results:**
```cpp
struct PlayerSave { int16_t health; int16_t score; uint8_t level; };

PlayerSave data;
switch (WE_SaveManager::Get().read(SAVE_SLOT_0, data)) {
    case ESP_OK:              /* use data normally */                     break;
    case WE_ERR_SAVE_EMPTY:   /* first boot — initialise to defaults */   break;
    case WE_ERR_SAVE_VERSION: /* save struct changed — reset or migrate */ break;
    case WE_ERR_SAVE_CORRUPT: /* bit-flip detected — reset */             break;
}
```

**Save struct rules:**
- Must be a plain struct — no virtual methods, no `std::string`, no owning pointers
- A `static_assert` at compile time rejects structs that violate this
- `sizeof(YourStruct)` must be ≤ the `.size` you set in `SAVE_SLOTS[]`

**Integrity system (on by default):**
Each slot is prefixed with a 4-byte header: magic number, version byte, and CRC8 checksum. The header lets the engine distinguish between an uninitialised chip (first boot), a version change, and actual data corruption. Set `WE_SAVE_INTEGRITY = false` in `WE_SaveSettings.hpp` to strip all header logic at compile time if you need the bytes.

**Multiple EEPROM chips:**
Add more entries to `WE_SAVE_EEPROMS[]` (valid addresses: `0x50`–`0x57`) and set `.eepromIndex` on each slot to route it to the right chip. The chip list is null-terminated (`i2cAddr = 0x00` last).

**Bumping the version:**
When you add/remove/reorder fields in a save struct, increment `WE_SAVE_VERSION` in `WE_SaveSettings.hpp`. Old saves will return `WE_ERR_SAVE_VERSION` on the next boot instead of silently loading garbage.

---

## 7. Data Flow & Key Interactions

### Flow A — Frame render cycle

```
1. StartGame() busy-waits until TARGET_FRAME_TIME_US (33 333 µs at 30 fps) has elapsed.

2. WEInputManager::tick()
   └─ For each controller: poll GPIO or I²C expander pins
      → debounce → update button state bitmasks
      → read ADC channels → normalise joystick axes

3. ModuleSystem::UpdateAll()
   └─ Calls OnUpdate() on every module in priority order
      (e.g. WE_SaveManager::OnUpdate() — currently a no-op, reserved for future use)

4. For each active GameObject in registry:
   └─ componentTick() — ticks all enabled components (Animator, Collider, etc.)

5. For each active GameObject in registry:
   └─ Update() — calls user game logic (virtual override)

6. WEColliderManager::tick()
   └─ For every pair (i, j) of registered colliders:
      a. Apply layer bitmask filter — skip if no interaction
      b. Run shape intersection test (AABB / circle / mixed)
      c. Compare result to previous-frame state
      d. Fire OnCollisionEnter / Stay / Exit  or  OnTriggerEnter / Stay / Exit
         directly on both GameObjects

7. Camera::followTick() — lerp camera toward follow target's new position

8. WERenderCore::render()
   └─ Clear framebuffer (if cleanFramebufferEachFrame = true in RENDER_SETTINGS)
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

| Storage | Technology | Usage |
|---|---|---|
| Persistent save data | I²C EEPROM via `WE_SaveManager` | Up to 8 chips × 64 KB; addressed through named slots |
| Framebuffer | Static `uint16_t[]` | RGB565, `screenWidth × screenHeight` words |

**Allocation strategy:** No `new`/`delete` is used anywhere in the engine. All
collections are fixed-size arrays sized by constants in `WE_Settings.hpp`
(`MAX_GAME_OBJECTS`, `MAX_COLLIDERS`, etc.). This is a deliberate choice to eliminate
heap fragmentation.

**EEPROM access:** Always go through `Save().read()` / `Save().write()` — do not use the raw `EEPROM24LC512` driver directly in game code. Writes block for 5–20 ms per 128-byte page; schedule them between levels or on pause, not inside `Update()`.

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
| `WE_Settings.hpp` | Master include — pulls all settings headers | Recompile |
| `WE_Modules.hpp` | Module feature flags (`#define SaveLoadModule`, etc.) — comment out to disable a module entirely | Recompile |
| `WE_PINDEFS.hpp` | All GPIO numbers: SPI, I²C, audio, display DC/Reset/CS | Recompile |
| `WE_InputSettings.hpp` | Per-controller button→pin map, expander type & address, joystick ADC channel & calibration | Recompile |
| `WE_RenderSettings.hpp` | Background color (RGB565), game region rect, framebuffer clear flag | Recompile |
| `WE_Layers.hpp` | `RenderLayer` enum values, `CollisionLayer` bitmask values | Recompile + update all layer assignments |
| `WE_SaveSettings.hpp` | EEPROM chip list, save slot names + sizes, integrity on/off, magic/version constants | Recompile |

### Feature flags

**Display target** — defined in `WE_Settings.hpp` (only one at a time):

| Flag | Effect |
|---|---|
| `DISPLAY_ST7735` | Selects ST7735 as display driver |
| `DISPLAY_CUSTOM` | Use a custom display driver |

**Module flags** — defined in `WE_Modules.hpp` (comment out to strip module from build):

| Flag | Module enabled |
|---|---|
| `WE_MODULE_SAVELOAD` | `WE_SaveManager` — I²C EEPROM save/load system |

**Per-file debug flag:**

| Flag | Effect when defined |
|---|---|
| `MODULE_DEBUG_ENABLED` | Enables `DebugLog`/`DebugErr` output via `esp_log` in that translation unit |

**Runtime settings** (structs in `WE_Settings.hpp`, not `#define` flags):

| Setting | Where | Controls |
|---|---|---|
| `RENDER_SETTINGS.spriteSystemEnabled` | `WE_RenderSettings.hpp` | Includes sprite registration in render loop |
| `RENDER_SETTINGS.cleanFramebufferEachFrame` | `WE_RenderSettings.hpp` | Zeroes framebuffer each frame |

---

## 11. Known Issues, Gaps & Technical Debt

Issues have been moved into Issues.md file.
