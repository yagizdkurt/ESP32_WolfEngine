# SDL3 Desktop AI Documentation

Machine-oriented reference for the SDL3 desktop target.
Use this as the primary contract map when editing desktop platform code.

---

## 1. Intent

Desktop target runs WolfEngine on host OS with SDL3 by:

- Replacing hardware display backend with SDL display driver
- Injecting keyboard-based input provider
- Providing ESP-IDF/FreeRTOS compatibility headers via stubs

Core engine logic remains shared with ESP32 path.

---

## 2. Build Contracts

Defined in desktop/CMakeLists.txt.

### Required compile definitions

- `WE_DESKTOP_BUILD=1`
- `DISPLAY_SDL`
- `WE_PLATFORM_SDL`

### Required source composition

- Include all src/WolfEngine cpp files
- Exclude ST7735 concrete driver cpp
- Include desktop-specific files:
  - main_desktop.cpp
  - WE_SDLManager.cpp
  - WE_SDLInputDriver.cpp
  - stubs/WE_Display_SDL3.cpp

### Include path precedence

desktop/stubs must be before system/engine include paths so ESP-IDF headers resolve to stubs.

---

## 3. File Responsibility Map

| File | Responsibility |
|---|---|
| desktop/main_desktop.cpp | Boot sequence and thread orchestration |
| desktop/WE_SDLManager.hpp/.cpp | SDL subsystem lifecycle and event loop |
| desktop/WE_SDLInputDriver.hpp/.cpp | Event-thread to engine-thread input bridge |
| desktop/WE_SDLKeyMap.hpp | Default key bindings |
| desktop/stubs/Display_SDL.h | Render dimension macros + display driver declaration hook |
| desktop/stubs/WE_Display_SDL3.hpp/.cpp | DisplayDriver implementation + GetDriver bridge |
| desktop/stubs/esp_timer.h | Monotonic time source stub |
| desktop/stubs/esp_err.h | Error codes + aborting ESP_ERROR_CHECK |
| desktop/stubs/esp_log.h | Log macro mapping |
| desktop/stubs/freertos/*.h | Minimal RTOS primitive stubs |
| desktop/stubs/driver/*.h | GPIO/I2C/LEDC stub APIs |
| desktop/stubs/esp_adc/adc_oneshot.h | ADC oneshot stub API |

---

## 4. Runtime Sequence

## 4.1 Initialization

1. SDLManager init
2. Window + renderer creation
3. Renderer -> SDL display driver injection (`GetSDLDriver().setRenderer`)
4. Engine StartEngine
5. Input provider injection (`Input().setInputProvider(&SDLInputDriver)`)
6. Controller 0 bypass enable (`setAlwaysEnableController0(true)`)
7. Engine StartGame on background thread

## 4.2 Steady-state loop

Main thread:
- poll SDL events
- pass each event to SDLInputDriver::processEvent

Engine thread:
- gameTick
- InputManager tick
- provider flush
- renderer flush via DisplayDriver

## 4.3 Shutdown

1. Request quit
2. Join engine thread
3. Destroy display driver resources
4. Destroy window/SDL

---

## 5. Thread Model

### Thread ownership

- SDL main thread:
  - SDLManager::pollEvents
  - SDLInputDriver::processEvent

- Engine thread:
  - WolfEngine::StartGame loop
  - InputManager::tick
  - SDLInputDriver::flush
  - Renderer operations

### Synchronization strategy

- Shared input state uses atomics (`std::atomic<uint32_t>`, `std::atomic<uint8_t>`)
- No mutex around input handoff
- Memory order currently relaxed for load/store operations

---

## 6. Input Provider Contract

`IInputProvider` API:
- `flush(Controller* controllers, int count)`

Desktop implementation obligations:

1. `processEvent` updates shared state only
2. `flush` applies full state every engine frame
3. Preserve button edge behavior by writing every button each flush
4. Keep provider object lifetime >= engine runtime

Current mapping implementation:
- Buttons A-J via key map table
- Joystick as discrete directional composition to `{-1,0,1}` per axis

---

## 7. Display Driver Contract

Engine expects `DisplayDriver* GetDriver()` resolution through display-selection headers.

Desktop path provides:
- `Display_SDL.h` macros:
  - `RENDER_SCREEN_WIDTH`
  - `RENDER_SCREEN_HEIGHT`
- `GetDriver()` returning static SDL3DisplayDriver

SDL3DisplayDriver requirements:

1. `setRenderer` called before `initialize`
2. `initialize` creates texture
3. `flush` uploads framebuffer and presents
4. `destroy` called after engine thread stops

Resource ownership notes:
- Driver owns texture handle
- Driver receives renderer pointer from SDLManager
- Destruction order must preserve renderer validity until texture destroy completes

---

## 8. Stub Layer Behavior Summary

This is compatibility behavior, not hardware behavior.

- esp_timer.h:
  - `esp_timer_get_time()` from steady_clock since process start
- esp_err.h:
  - `ESP_ERROR_CHECK` aborts on non-ESP_OK
- esp_log.h:
  - log macros routed to printf
- FreeRTOS stubs:
  - tick constants/types + no-op vTaskDelay
  - semaphore functions return placeholder values
- driver stubs:
  - GPIO/I2C/LEDC APIs return success placeholders
- ADC oneshot stub:
  - API present with placeholder read value

Treat stubs as compile/run compatibility layer for desktop iteration.

---

## 9. Invariants to Preserve

1. `DISPLAY_SDL` must remain enabled in desktop build path
2. ST7735 concrete driver cpp must stay excluded from desktop executable
3. Input provider must be injected before StartGame
4. SDL renderer must be set on display driver before StartEngine uses renderer path
5. Engine thread must join before SDL resource teardown
6. stubs include path must remain first in desktop target

---

## 10. Safe Modification Checklist

Before merge of desktop changes, verify:

- Build still resolves SDL3 package and links
- GetDriver symbol is still provided once
- Window close/ESC still leads to graceful shutdown
- Input state still reaches Controller 0 every frame
- Render output still updates in SDL window
- No cross-thread lifetime regression (provider, renderer, texture)

---

## 11. Suggested AI Workflow For Desktop Changes

1. Read these files first:
- desktop/main_desktop.cpp
- desktop/CMakeLists.txt
- desktop/WE_SDLManager.cpp
- desktop/WE_SDLInputDriver.cpp
- desktop/stubs/WE_Display_SDL3.hpp
- desktop/stubs/WE_Display_SDL3.cpp

2. Classify requested change as one of:
- build/config
- input translation
- display output
- lifecycle/shutdown
- stubs compatibility

3. Modify only the narrow layer first.
4. Re-check invariants from section 9.
5. Run desktop build and smoke test event loop + frame output path.

---

## 12. Notes For Future Refactors

Potential refactor domains (non-breaking intent):

- Promote key map to runtime-configurable profile
- Add SDL gamepad backend while preserving IInputProvider contract
- Add frame pacing diagnostics in SDL manager layer
- Keep engine platform-agnostic boundaries intact (desktop behavior isolated in desktop folder)

This document should stay implementation-aligned with desktop folder and architecture contracts.
