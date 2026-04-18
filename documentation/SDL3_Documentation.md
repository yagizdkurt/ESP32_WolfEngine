# SDL3 Desktop Documentation

This document explains how the SDL3 desktop target is structured, how it boots, and how it connects to core engine systems.
It is intended for human readers who want to build, run, and extend the desktop platform layer.

---

## 1. Purpose

The SDL3 desktop target provides a host-platform runtime for WolfEngine so you can:

- Run the engine without ESP32 hardware
- Iterate game logic faster
- Use keyboard-driven input emulation
- Visualize the framebuffer through SDL renderer/texture pipeline

The desktop target is built as a separate executable and integrates with engine code via:

- Display driver swap (`DISPLAY_SDL`)
- Input provider injection (`IInputProvider`)
- ESP-IDF/FreeRTOS stub headers under desktop/stubs

---

## 2. High-Level Architecture

At runtime, desktop mode is split into two cooperating loops:

1. SDL main thread:
- Owns SDL window + renderer
- Polls OS events
- Feeds keyboard state into SDLInputDriver atomics

2. Engine thread:
- Runs WolfEngine `StartGame()` loop
- Calls `InputManager::tick()` every game tick
- Pulls atomics via `SDLInputDriver::flush()` and pushes state into Controller 0
- Renders to framebuffer, then flushes through SDL3 display driver

### Architecture sketch

```text
main_desktop.cpp
  -> SDLManager::init()
  -> Engine().StartEngine()
  -> Input().setInputProvider(&SDLInputDriver)
  -> start Engine().StartGame() on background thread

Main thread (SDL):
  SDLManager::pollEvents()
    -> SDLInputDriver::processEvent(e)

Engine thread:
  WolfEngine::StartGame()
    -> gameTick()
      -> InputManager::tick()
         -> inputProvider->flush(...)
      -> Renderer::render()
         -> DisplayDriver::flush(...)
            -> SDL texture upload + present
```

---

## 3. Desktop Folder Map

| Path | Role |
|---|---|
| desktop/main_desktop.cpp | Desktop entry point and thread orchestration |
| desktop/CMakeLists.txt | SDL3 desktop build target definition |
| desktop/WE_SDLManager.* | SDL lifecycle, event loop, shutdown coordination |
| desktop/WE_SDLInputDriver.* | SDL keyboard -> engine controller bridge |
| desktop/WE_SDLKeyMap.hpp | Default key mapping table |
| desktop/stubs/WE_Display_SDL3.* | Display driver implementation used by renderer |
| desktop/stubs/Display_SDL.h | Render resolution macros + GetDriver declaration |
| desktop/stubs/*.h | ESP-IDF / FreeRTOS compatibility stubs |
| desktop/Buildit.bat | Windows convenience build script |

---

## 4. Build and Compile-Time Configuration

Desktop target is configured in desktop/CMakeLists.txt.

### Key behavior

- Finds SDL3 using `find_package(SDL3 REQUIRED)`
- Globs engine sources from src/WolfEngine
- Excludes ST7735 driver implementation (desktop uses SDL display driver)
- Adds desktop-specific sources:
  - main_desktop.cpp
  - WE_SDLManager.cpp
  - WE_SDLInputDriver.cpp
  - stubs/WE_Display_SDL3.cpp

### Important compile definitions

- `WE_DESKTOP_BUILD=1`
- `DISPLAY_SDL`
- `WE_PLATFORM_SDL`

`DISPLAY_SDL` activates Display_SDL include path selection in renderer-related headers.

### Include-path strategy

desktop/stubs is intentionally placed before engine/system headers so ESP-IDF includes resolve to host stubs.

---

## 5. Startup and Shutdown Sequence

## 5.1 Startup order

1. Create SDLManager
2. `SDLManager::init(config)`
   - `SDL_Init(SDL_INIT_VIDEO)`
   - Create window and renderer
   - Set logical presentation/letterboxing
   - Set render color scale (brightness)
   - Inject SDL renderer into SDL display driver (`GetSDLDriver().setRenderer(...)`)
3. `Engine().StartEngine()`
4. Create static SDLInputDriver
5. `Input().setInputProvider(&inputDriver)`
6. `Input().setAlwaysEnableController0(true)`
7. Launch `Engine().StartGame()` on background thread
8. Main thread keeps polling SDL events

## 5.2 Shutdown order

When window close or ESC is received:

1. `SDLManager::shutdown(...)` requests engine quit
2. Joins engine thread
3. Destroys SDL display resources (`GetSDLDriver().destroy()`)
4. Destroys SDL window and quits SDL

This sequence preserves ownership and avoids use-after-free between display and renderer.

---

## 6. SDL Manager Responsibilities

SDLManager manages host runtime concerns only.

### init(config)

- Creates window and renderer
- Sets logical presentation with letterboxing
- Applies render brightness scale
- Connects SDL renderer to display driver

### pollEvents()

- Drains SDL event queue
- For each event calls `SDLInputDriver::processEvent(e)`
- Returns false on quit or ESC
- Uses `SDL_Delay(16)` to avoid event-loop busy spin

### shutdown(engineThread, engine)

- Signals engine termination
- Joins game thread
- Destroys display texture/renderer via driver
- Destroys SDL window
- Calls SDL_Quit

---

## 7. Input Pipeline (SDL -> Controller)

Desktop input uses provider injection instead of GPIO/ADC polling.

## 7.1 Event ingestion

`SDLInputDriver::processEvent` runs on the SDL thread and writes keyboard state into atomics:

- `s_buttons` (`uint32_t`) for A-J buttons
- `s_joyKeys` (`uint8_t`) for axis direction bits

## 7.2 Per-frame flush

`SDLInputDriver::flush` runs on engine thread and:

- Applies button states via `Controller::simulateButton`
- Computes axis from held direction bits and applies via `simulateJoystick`
- Targets controller index 0

### Default mapping

Buttons:
- A: Z
- B: X
- C: C
- D: V
- E: Up
- F: Down
- G: Left
- H: Right
- I: Left Shift
- J: Space

Joystick axes:
- X+: D, X-: A
- Y+: S, Y-: W

Mapping lives in `WE_SDLKeyMap.hpp` and can be changed centrally.

---

## 8. Display Pipeline (Framebuffer -> SDL)

The engine renderer still writes to RGB565 framebuffer as usual.

Desktop display driver translates that framebuffer to SDL output.

### Driver entry points

- `GetDriver()` returns `DisplayDriver*` used by engine constructor path
- `GetSDLDriver()` returns concrete SDL3DisplayDriver for desktop setup/shutdown hooks

### initialize()

- Creates streaming SDL texture in RGB565 (128 x 160)

### flush(...)

- Locks texture
- Copies framebuffer row-by-row (handles pitch alignment)
- Unlocks texture
- Renders texture to SDL renderer
- Presents frame

Current implementation uploads full framebuffer every flush call.

---

## 9. Stub Layer Overview

desktop/stubs provides host-compatible definitions for selected ESP-IDF and FreeRTOS headers.

Purpose:
- Allow engine compilation on desktop
- Preserve function signatures expected by engine code
- Keep desktop build independent from ESP-IDF toolchain

Stub categories:

- Timing/logging/errors:
  - esp_timer.h
  - esp_log.h
  - esp_err.h
- RTOS:
  - freertos/FreeRTOS.h
  - freertos/semphr.h
- Drivers:
  - driver/gpio.h
  - driver/i2c.h
  - driver/ledc.h
  - esp_adc/adc_oneshot.h

These headers are compatibility shims for desktop execution and testing workflows.

---

## 10. Integration Contracts You Must Keep

When modifying desktop platform code, preserve these contracts:

1. Set SDL renderer on display driver before `Engine().StartEngine()`
2. Keep input provider object alive for full engine runtime
3. Request quit and join engine thread before destroying SDL resources
4. Keep desktop stubs include path precedence in CMake
5. Keep `DISPLAY_SDL` compile definition for renderer path selection

Breaking any of these usually causes startup failures, missing display output, or race/lifetime bugs.

---

## 11. Extending Desktop Platform Safely

Common extension points:

- Key remapping:
  - Edit default map in `WE_SDLKeyMap.hpp`
- Add gamepad support:
  - Extend SDLInputDriver with SDL gamepad events
  - Keep provider flush contract unchanged
- Add diagnostics overlay:
  - Build in SDL layer without changing engine renderer contracts
- Add pacing/profiling helpers:
  - Keep engine tick ownership in engine thread
  - Avoid blocking event thread

Guideline: prefer extending SDL platform wrapper first, then touch core engine only when abstraction boundaries truly require it.

---

## 12. Desktop Build Quickstart

### Using Buildit.bat (Windows)

1. Set `SDL3_DIR` in `desktop/Buildit.bat`
2. Run `Buildit.bat`

### Using CMake directly

```bash
cmake -B build -S desktop -DSDL3_DIR="C:/SDL3/cmake"
cmake --build build
```

Run produced executable from desktop/build output.

---

## 13. Practical Notes

- The engine runs on a background thread; SDL event loop remains on main thread.
- Input injection path avoids hardware GPIO/ADC polling in desktop mode.
- Renderer remains engine-owned; desktop only supplies display/input backend implementation.

This keeps gameplay systems largely platform-agnostic while preserving a fast desktop iteration workflow.
