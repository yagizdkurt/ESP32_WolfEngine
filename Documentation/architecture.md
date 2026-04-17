# Architecture

This file provides contextual information to assist AI tools in understanding the project’s design decisions and constraints, helping ensure accurate and consistent contributions.
For comprehensive documentation intended for human readers, refer to documentation.md.

- Title indexes are not important.

---

## 1. Overview

WolfEngine is a 2D game engine targeting the **ESP32** (Xtensa LX7).
Single fixed-timestep busy-wait loop on one FreeRTOS task.

**Allocation policy:** No heap anywhere in the engine. Module instances are `static`
objects defined in `WE_ModuleSystem.cpp`; `ModuleSystem` holds a plain `static IModule*[]`
array — no `std::vector`, no heap. All other engine systems use fixed-size arrays.

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
| Static module system with priority dispatch | `ModuleSystem`, `WE_ModuleSystem.cpp`, `WE_Modules.hpp` | Optional subsystems are listed in `WE_ModuleSystem.cpp` under `#if defined()` guards; `ModuleSystem::InitAll()` insertion-sorts by `Priority` (descending) then calls `OnInit()` on each |
| Entity-Component System | `GameObjectSystem` + `ComponentSystem` | Lightweight: no archetype tables, no dynamic dispatch via vtable arrays — components are owned by the GO and ticked via direct method calls |
| Factory method | `GameObject::Create<T>()`, `Collider::Box()`, `Collider::Circle()`, `Sprite::Create()` | Hides construction details; `Create<T>` placement-news into a registry slot |
| Abstract interface | `WE_Display_Driver`, `WE_IExpander`, `WE_IEEPROMDriver`, `WE_IInputProvider` | Lets driver selection be a compile-time `#if` in settings or a runtime enum dispatch; `IInputProvider` is injected at runtime via `setInputProvider()` |
| Dirty flag | `WEUIManager`, `BaseUIElement` | UI skips redraw when nothing has changed |
| Triangular bitmask | `WEColliderManager` | Tracks per-pair collision state in O(n²/2) bits without a hash map |
| Placement new | `WEController` for expander objects; `WE_SaveManager` for EEPROM driver objects | Avoids heap; concrete driver constructed into a `uint8_t` buffer sized to the largest concrete type |
| Constexpr validation | `Sprite::Create()`, `WE_SaveManager` slot guards | Illegal dimensions / slot overflow / count mismatch caught at compile time, not runtime |
| Null-terminated config array | `WE_SAVE_EEPROMS[]` | EEPROM chip list terminates with `i2cAddr = 0x00`; chip count deduced via `constexpr` lambda |

### What is intentionally avoided

- **Heap anywhere in the engine:** No `new`/`delete` in any path. Module instances are
  file-scope `static` objects in `WE_ModuleSystem.cpp`; the module list is a plain
  `static IModule*[]` array. No `std::vector`, no heap.
- **STL containers:** Entirely avoided — code-size and heap reasons. No `std::vector`,
  no `std::sort`; the module sort uses a manual insertion sort.

---

## 6. Core Modules & Responsibilities

### 6.1 WolfEngine

**Public interface:**
```cpp
WolfEngine& Engine();          // global accessor
void StartEngine();            // one-time hardware init → core subsystems → ModuleRegistry::InitAll()
void StartGame();              // blocking game loop
```

**Key dependencies:** All core subsystems (renderer, camera, input, UI, sound, colliders)
plus `ModuleSystem` for optional modules.

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
InputManager& Input();                   // global accessor
Controller* getController(int i);        // 0–3; returns nullptr if disabled

// Platform injection (call after StartEngine(), before StartGame()):
void setInputProvider(IInputProvider* provider);  // nullptr = hardware GPIO/ADC path
void setAlwaysEnableController0(bool value);      // bypass enabled check for controller 0

// On Controller:
bool getButton(Button b);       // held
bool getButtonDown(Button b);   // pressed this frame
bool getButtonUp(Button b);     // released this frame
float getAxis(JoyAxis a);       // -1.0 to 1.0
```

**Key design choices:**
- Each controller is configured entirely by `ControllerSettings` in `WE_InputSettings.hpp`
  — no code changes needed to remap buttons.
- Expander objects are placement-newed into a fixed buffer inside `Controller`;
  type is selected at runtime from `ExpanderType` enum.
- Debounce per-button via timestamp; poll interval is configurable.
- ADC joystick: raw reading normalised against calibration min/mid/max values defined
  in settings.
- `IInputProvider` interface allows any platform to override the hardware polling path
  without touching engine source. `setInputProvider()` stores a raw pointer; the caller
  owns the lifetime. `nullptr` restores the hardware path.
- `setInputProvider()` and `setAlwaysEnableController0()` are intentionally separate —
  a provider does not imply that controller 0 must bypass the enabled check.


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
| `WE_Debug.h` | `DebugLog`/`DebugErr` macros. Zero-cost when `MODULE_DEBUG_ENABLED` is not defined. |

---

### 6.13 Module System

**Key files:**

| File | Role |
|---|---|
| `Modules/WE_IModule.hpp` | `IModule` base: `Name`/`Priority` public fields, private `OnInit/OnUpdate/OnShutdown`; `TModule<T, Priority>` CRTP helper |
| `Modules/WE_ModuleSystem.hpp` | `ModuleSystem` class: declares `InitAll/UpdateAll/ShutdownAll` (only `WolfEngine` may call them) |
| `Modules/WE_ModuleSystem.cpp` | Owns all module instances and the `IModule*[]` list; `InitAll` sorts then calls hooks |
| `Settings/WE_Modules.hpp` | Feature flags — `#define WE_MODULE_SAVELOAD`, etc. Controls which `#if` blocks compile |

**IModule / TModule interface:**
```cpp
class IModule {
public:
    const char* const Name;
    const int         Priority;
private:
    friend class ModuleSystem;
    virtual void OnInit()     {}
    virtual void OnUpdate()   {}
    virtual void OnShutdown() {}
};

template<typename T, int Priority>
class TModule : public IModule {
public:
    static T& Get();          // returns the singleton instance
protected:
    TModule(const char* name) : IModule(name, Priority) {}
};
```

`OnInit/OnUpdate/OnShutdown` are private — only `ModuleSystem` (friended) can invoke them.
`TModule<T, Priority>` bakes the priority into the `IModule::Priority` field at construction
and provides the typed `Get()` accessor. The constructor is private to the subclass, preventing
external instantiation.

**WE_ModuleSystem.cpp — the single list:**
```cpp
// Instance + pointer added together under the same #if guard
#if defined(WE_MODULE_SAVELOAD)
#include "SaveLoadSystem/WE_SaveManager.hpp"
static WE_SaveManager s_saveLoad;
#endif

static IModule* s_modules[] = {
#if defined(WE_MODULE_SAVELOAD)
    &s_saveLoad,
#endif
};
```
`InitAll()` runs an insertion sort on `s_modules` by `Priority` (descending — highest first)
before calling `OnInit()`. `UpdateAll/ShutdownAll` iterate the already-sorted array.
No separate priority list; no STL.

**ModuleSystem API** (called only by `WolfEngine`):
```cpp
ModuleSystem::InitAll();       // sorts by priority, then OnInit() each — once, in StartEngine()
ModuleSystem::UpdateAll();     // OnUpdate() each — every frame in gameTick()
ModuleSystem::ShutdownAll();   // OnShutdown() in reverse priority order
```

**Accessing a module from game code:**
```cpp
WE_SaveManager::Get().write(SAVE_SLOT_0, myData);
```

**Adding a new module:**
1. Inherit `TModule<MyModule, Priority>`, implement desired hooks.
2. `#define WE_MODULE_MYMODULE` in `Settings/WE_Modules.hpp`.
3. In `WE_ModuleSystem.cpp`: add `static MyModule s_myModule` and `&s_myModule` to `s_modules[]` under `#if defined(WE_MODULE_MYMODULE)`.

---

### 6.14 SaveManager

**Registered via:** `WE_ModuleSystem.cpp`, guarded by `#if defined(WE_MODULE_SAVELOAD)`.

**Access from game code:**
```cpp
WE_SaveManager& save = WE_SaveManager::Get();
```

**Public interface:**
```cpp
template<typename T>
esp_err_t write(SaveSlot slot, const T& data); // blocking write; T must be trivially copyable

template<typename T>
esp_err_t read(SaveSlot slot, T& outData);     // fast read

esp_err_t erase(SaveSlot slot);                // fill slot region with 0xFF
esp_err_t eraseAll();                          // erase all chips — never call during gameplay

// Error codes returned by read():
// WE_ERR_SAVE_EMPTY    (0x2001) — slot never written
// WE_ERR_SAVE_CORRUPT  (0x2002) — CRC8 mismatch
// WE_ERR_SAVE_VERSION  (0x2003) — WE_SAVE_VERSION changed
// WE_ERR_SAVE_OVERFLOW (0x2004) — sizeof(T) > slot's defined size
```

**Key design choices:**
- `OnInit()` does the EEPROM driver init (placement-new per chip). `OnUpdate()` is a no-op — SaveManager is purely on-demand.
- Each EEPROM chip is instantiated via placement new into `m_driverBufs[i]` (type from `WE_SAVE_EEPROMS[i].type`). Same pattern as `WEController` + expanders.
- Driver buffer size (`WE_EEPROM_DRIVER_BUF_SIZE`) is computed at compile time as the `sizeof` max of all registered drivers via a constexpr lambda. Individual sizes are scoped inside the lambda and disappear.
- `WE_IEEPROMDriver*` abstract pointer is used for all chip communication — `WE_SaveManager` never calls any concrete driver method directly.
- Slot addresses are computed per-chip by summing earlier slot sizes on the same chip. Each chip's address space starts at 0.
- `WE_SAVE_INTEGRITY = true` (default): a 4-byte header `[magic(2) | version(1) | CRC8(1)]` is prepended to every slot. All header code is inside `if constexpr (WE_SAVE_INTEGRITY)` — zero overhead when false.
- CRC8 is CRC-8/SMBUS (polynomial 0x07), computed over data bytes only.
- `write<T>` builds `[header][struct bytes]` on the stack — no heap allocation.
- `static_assert(std::is_trivially_copyable<T>::value)` rejects structs with vtables or owning pointers at compile time.
- Do NOT call `write()` or `eraseAll()` during gameplay — writes block for 5–20 ms per 128-byte page. Call only at safe moments (between levels, pause menu, boot). Reads are fast (~1 ms) and can be called at any time. Always access via `WE_SaveManager::Get()`.

**Compile-time guards (in `WE_SaveManager.hpp`):**
- `SAVE_SLOTS` count must equal `SAVE_SLOT_COUNT` → catches add/remove mismatch
- All slot sizes must be > 0 → catches missing `SAVE_SLOTS[]` entries
- Total bytes per chip must not exceed `WE_GetEEPROMCapacity(type)` → catches over-allocation

**Adding a new EEPROM driver (maintainer workflow):**
1. Create `WE_EEPROMXXXXXX.hpp` inheriting `WE_IEEPROMDriver`
2. Add enum entry to `EEPROMDriverType` + capacity to `WE_EEPROM_CAPACITIES[]` in `WE_SaveSettings.hpp`
3. In `WE_SaveManager.hpp`: `#include` driver + add its `sizeof` inside the `WE_EEPROM_DRIVER_BUF_SIZE` lambda
4. Add `case` to the `switch` in `WE_SaveManager::OnInit()`

---

## 10. Configuration & Environment

All configuration is **compile-time**. There are no runtime environment variables,
no config files read from flash, and no over-the-air configuration.

### Settings files (all in `Settings/`)

| File | Controls |
|---|---|---|
| `WE_Settings.hpp` | Master include — pulls all settings headers |
| `WE_Modules.hpp` | Module feature flags (`#define SaveLoadModule`, etc.) — comment out to disable |
| `WE_PINDEFS.hpp` | All GPIO numbers: SPI, I²C, audio, display DC/Reset/CS |
| `WE_InputSettings.hpp` | Per-controller button→pin map, expander type & address, joystick ADC channel & calibration |
| `WE_RenderSettings.hpp` | Background color (RGB565), game region rect, framebuffer clear flag |
| `WE_Layers.hpp` | `RenderLayer` enum values, `CollisionLayer` bitmask values |
| `WE_SaveSettings.hpp` | EEPROM chip list, slot definitions + sizes, integrity flag, magic/version constants |

---

## 11. Known Issues, Gaps & Technical Debt

Issues have been moved into Issues.md file.