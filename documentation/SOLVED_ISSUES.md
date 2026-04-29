# Resolved Issues
This file is a permanent record of known issues that have been identified and resolved
during development. It is append-only — entries are never deleted or edited after they
are written. Active (unresolved) issues live in KNOWN_ISSUES.md.

## Entry Format
Each entry follows this exact structure:

## <Short Title>
RESOLVED in - <DD.MM.YY>
**Location:** <file(s) and specific area affected>
**What it did:** <what the problem was and why it was acceptable or unnoticed>
**Resolution:** <what was changed and why, including any design decisions>
- Bullet points for specific code or file changes
- Keep it precise enough that a future developer can understand the decision without
  reading the git diff
**Extra Notes** any extra notes to be informed.

note: Phases are just for SDL integration fixes. Dont use phases for normal engine issues.
---

## Rules for AI Assistants
- When resolving an entry from KNOWN_ISSUES.md, move it here — do not delete it.
- Strike-through in KNOWN_ISSUES.md is not sufficient — the full resolved entry
  belongs here with a proper Resolution section added.
- When adding new entry start writing in top of the entries so that top most entry is the most recent one.
- Never edit existing entries in this file. Add new entries only at the bottom.
- Always include the phase and date in the RESOLVED line.
- The Resolution section must explain the mechanism, not just say "fixed".
- If multiple issues are resolved in the same phase, add them as separate entries.

-----------------------------------------------------------------------------------------

## I2C ISR Fires During Driver Install
RESOLVED in - 29.04.26
**Location:** `src/WolfEngine/WolfEngine.cpp` — `initializeDrivers()` / `initializeSubsystems()`
**What it did:** During cold boot the I2C driver was enabled (ISR active) and an immediate write to the PCF8574 expander occurred while the physical bus lines could still be unstable. That produced interrupt storms and `TG0WDT_SYS_RESET` resets on some boots.
**Resolution:** Addressed the immediate race between driver enable and first device access by deferring first device I2C traffic and adding a short bus-stabilization delay:
- Added `vTaskDelay(pdMS_TO_TICKS(50))` at the end of `initializeDrivers()` immediately after `I2CManager::begin()` to give the bus time to reach idle-high before any device writes.
- Moved `m_InputManager.init()` to the end of `initializeSubsystems()` so PCF8574 writes occur after renderer bring-up, providing additional temporal separation of bus activity.
- Verification: serial boot logs should show `I2C driver ready` before any expander-ready messages; no `TG0WDT_SYS_RESET` observed across multiple cold boots.
**Extra Notes** The change is deliberately minimal (deferred-first-access) rather than a full boot API redesign; the architectural split is tracked separately but the immediate crash vector is closed by these edits.

---

## StartEngine Responsibilities Split (minimal hardening)
RESOLVED in - 29.04.26
**Location:** `src/WolfEngine/WolfEngine.cpp` — `StartEngine()` / `initializeSubsystems()`
**What it did:** `StartEngine()` mixed low-level hardware bring-up (I2C, display) with runtime subsystem initialization (input, camera, UI, modules). That coupling made startup ordering fragile and obscured which steps must happen before the engine can safely run frames.
**Resolution:** Implemented a low-risk, behavior-preserving hardening to separate concerns without large refactors:
- Deferred hardware-dependent device initialization (notably controller expander init) until after renderer and core runtime initialization by moving `m_InputManager.init()` later in the sequence.
- Left `I2CManager::begin()` in hardware bring-up but ensured a stabilization delay before device use (see I2C entry above).
- Documented the recommended two-phase public API (`BootHardware()` / `StartRuntime()`) for future refactors; the immediate safety fixes avoid a disruptive large refactor while making startup ordering explicit.
**Extra Notes** This reduces boot-time coupling and makes future split/refactor lower-risk; a full `BootHardware()`/`StartRuntime()` refactor remains an optional follow-up.

---

## I2C Initialization Hardening — Expander Logging + Follow-ups
RESOLVED in  - 29.04.26
**Location:** `src/WolfEngine/InputSystem/WE_Controller.cpp`; `src/WolfEngine/Utilities/WE_I2C.cpp`; `src/WolfEngine/Drivers/EepromDrivers/WE_EEPROM24LC512.hpp`
**What it did:** The I2C bring-up path left some diagnostic and defensive gaps: callers ignored `m_expander->begin()` return values (silent failures), and the code had API edge-cases such as unguarded zero-length reads. There was also no automatic startup probe to surface missing devices early.
**Resolution:** Applied targeted correctness and diagnostics improvements from the instability fix plan:
- `Controller::init()` now captures and logs the `esp_err_t` result from `m_expander->begin()` and emits `WE_LOGI("Controller", "Expander ready at 0x%02X", addr)` on success or an error log on failure.
- Combined with the I2C stabilization delay and deferred device init, the expander logging makes hardware failures observable at boot instead of silent misoperation.
- Kept the zero-length read guard and more invasive bus-reset proposals as tracked follow-ups (out of scope for this quick stability pass).
**Extra Notes** Optional follow-ups: add a guarded `read(len==0)` early-return, add an optional startup `I2CManager::scan()` probe, or implement a conservative bus-recovery pulse if electrical tests require it.

---

## vTaskDelay Declared in FreeRTOS.h Instead of task.h
RESOLVED in Phase 4 - 24.04.26
**Location:** `desktop/stubs/freertos/FreeRTOS.h`; `desktop/stubs/freertos/task.h`; `src/WolfEngine/Drivers/EepromDrivers/WE_EEPROM24LC512.hpp`
**What it did:** Desktop stubs defined `vTaskDelay` directly in `freertos/FreeRTOS.h`, while real FreeRTOS exposes it via `freertos/task.h`. This allowed desktop builds to compile task API call sites without explicit `task.h` inclusion, masking missing-include defects that can fail on hardware.
**Resolution (Phase 4):** Header ownership was aligned to real FreeRTOS and task API usage was made explicit at the engine call site:
- Removed `vTaskDelay` declaration from `desktop/stubs/freertos/FreeRTOS.h`.
- Added `desktop/stubs/freertos/task.h` and moved the desktop `vTaskDelay` stub there.
- Added explicit `#include "freertos/task.h"` in `WE_EEPROM24LC512.hpp`, where `vTaskDelay` is called in `pollAck()`.
**Extra Notes** This prevents desktop transitive-include behavior from hiding missing `task.h` dependencies.

---

## Diagnostics Counters Were Lifetime Totals, Not Per-Frame
RESOLVED in Phase 4 - 19.04.26
**Location:** `src/WolfEngine/Graphics/RenderSystem/WE_RenderCore.cpp` — `beginFrame()` diagnostics reset path; `src/WolfEngine/Graphics/RenderSystem/WE_DrawCommand.hpp` — `FrameDiagnostics` field comments
**What it did:** `commandsSubmitted`, `commandsDropped`, and `commandsExecuted` previously behaved as lifetime totals in documentation, which made per-frame overlays require external diffing and caused confusion about expected values.
**Resolution (Phase 4):** Diagnostics were aligned to per-frame behavior and documented accordingly:
- `beginFrame()` resets `commandsSubmitted`, `commandsDropped`, and `commandsExecuted` every frame.
- `peakCommandCount` remains a lifetime high watermark.
- DrawCommand restructure docs now describe `commandsDropped` as a per-frame counter with throttled first-drop logging.
**Extra Notes** This keeps frame diagnostics directly usable without caller-side snapshots/diffs.
---

## DrawCommand Struct Layout/Padding Risk
RESOLVED in Phase 4 - 19.04.26
**Location:** `src/WolfEngine/Graphics/RenderSystem/WE_DrawCommand.hpp`; `src/WolfEngine/Graphics/RenderSystem/WE_RenderCore.cpp`; `src/WolfEngine/Settings/WE_Layers.hpp`; `src/WolfEngine/ComponentSystem/Components/WE_Comp_SpriteRenderer.*`
**What it did:** Old `DrawCommand` layout mixed separate `layer` + `sortKey`, rotation inside sprite payload, and wider types (`int` size field, signed layer backing) that increased padding risk and compare complexity in sort.
**Resolution (Phase 4):** DrawCommand was restructured for compactness and single-key sort semantics:
- `RenderLayer` backing type moved to `uint8_t` and sprite renderer layer storage was tightened accordingly.
- `sortKey` now packs `RenderLayer` (high byte) + screenY (low byte) so sorting uses one compare.
- Rotation moved into command `flags` (bits 7-6) with `cmdGetRotation` / `cmdSetRotation` helpers.
- Sprite size narrowed to `uint8_t`, and reserved bytes were made explicit in sprite payload.
- Sort loop now compares only packed `sortKey`, and overflow log spam was reduced to first drop per frame.
**Extra Notes** The new layout targets a compact command footprint and cleaner extension for future command types.
---

## getController(0) Always Returns Non-Null on SDL
RESOLVED in Phase 3 - 17.04.26
**Location:** `src/WolfEngine/InputSystem/WE_InputManager.cpp` — `getController()`
**What it did:** An `#ifdef WE_PLATFORM_SDL` early-return bypassed the `enabled` check
in `ControllerSettings` for index 0. This was necessary because desktop builds do not
configure hardware controller settings, so controller 0 would appear disabled and return
`nullptr`. Acceptable as a short-term coupling but violated the zero-platform-knowledge
rule for engine source.
**Resolution (Phase 3):** The `#ifdef` was removed as part of the `IInputProvider`
refactor. The bypass is now expressed as a platform-agnostic flag:
- `InputManager` gains a private `bool m_alwaysEnableController0 = false` and a public
  `setAlwaysEnableController0(bool)` setter.
- `getController()` checks the flag instead of `#ifdef WE_PLATFORM_SDL`.
- `main_desktop.cpp` calls `Input().setAlwaysEnableController0(true)` after
  `StartEngine()` — explicit, visible, and not hidden in a preprocessor branch.
- `setInputProvider()` and `setAlwaysEnableController0()` are intentionally separate so
  a future provider (e.g. replay system) does not implicitly inherit the bypass.
---

## ESP_ERROR_CHECK Silently Swallows Errors
RESOLVED in Phase 2 - 14.04.26
**Location:** `desktop/stubs/esp_err.h` — `ESP_ERROR_CHECK` macro definition
**What it did:** `ESP_ERROR_CHECK(x)` expanded to `(x)`, evaluating the expression and
discarding the return value. Any non-`ESP_OK` result from driver or hardware init calls
was silently ignored. Code that depended on a hard abort to surface misconfigurations
would continue running in a broken state, masking bugs that would be immediately visible
on hardware.
**Resolution (Phase 2):** Macro replaced with a `do { } while(0)` block that captures
the return value, prints a diagnostic to `stderr` with the error code, source file, and
line number, then calls `abort()`:
- `ESP_ERROR_CHECK(x)` now stores the result in `esp_err_t _err`, checks against
  `ESP_OK`, and on failure emits `[ESP_ERROR_CHECK] FAILED (err=0x%x) at %s:%d` to
  `stderr` before calling `abort()`
- `<cstdio>` and `<cstdlib>` added to satisfy `fprintf` and `abort()`
- All error code constants and `typedef int esp_err_t` left unchanged
---

## WE_Settings.hpp Display Define Patched at CMake Configure Time
RESOLVED in Phase 2 - 14.04.26
**Location:** `Game1/src/WolfEngine/Settings/WE_Settings.hpp` — display target define block; `Game1/src/desktop/CMakeLists.txt` — configure-time patch block and include paths
**What it did:** `WE_Settings.hpp` contained a hardcoded `#define DISPLAY_ST7735`. A
compiler flag (`-DDISPLAY_SDL`) cannot suppress an in-source `#define`, so the desktop
CMake worked around this by reading the header at configure time, using `string(REPLACE)`
to swap the define, and writing the patched copy into `CMAKE_CURRENT_BINARY_DIR` where it
was picked up first via the include path. This was fragile: any new display define added to
the settings header would also need to be added to the regex, or the desktop build would
silently compile with two display targets active.
**Resolution (Phase 2):** The hardcoded define in `WE_Settings.hpp` is replaced with a
preprocessor guard that falls back to `DISPLAY_ST7735` only when no other display driver
flag is already defined. The desktop build now passes `DISPLAY_SDL` as a normal compile
definition, which the guard detects:
- `WE_Settings.hpp` lines 125–132: replaced `#define DISPLAY_ST7735` with
  `#ifndef DISPLAY_SDL / #define DISPLAY_ST7735 / #endif`
- `desktop/CMakeLists.txt`: removed the entire `file(READ / string(REPLACE / file(WRITE)`
  patch block (formerly lines 11–31)
- `desktop/CMakeLists.txt`: removed `${CMAKE_CURRENT_BINARY_DIR}` from
  `target_include_directories` — no patched copy exists to find
- `desktop/CMakeLists.txt`: added `DISPLAY_SDL` to `target_compile_definitions` so the
  guard in the header correctly skips the default on desktop
---

## Shutdown Signal is Hard Kill
RESOLVED in Phase 2 - 13.04.26
**Location:** desktop/main_desktop.cpp — ESC/window-close handler
**What it did:** called std::exit(0) which hard-killed the process. The detached engine
thread got no chance to run destructors or release resources. Acceptable in Phase 1 because
the engine held no real resources.
**Resolution (Phase 2):** `WolfEngine` gains a proper quit mechanism — no #ifdefs, no
globals, no desktop-specific code in engine source:
- `m_isRunning` promoted from `bool` to `std::atomic<bool>` inside `WolfEngine`.
- `WolfEngine::RequestQuit()` stores `false` into `m_isRunning` (callable from any thread).
- `WolfEngine::IsRunning()` loads `m_isRunning` (callable from any thread).
- `StartGame()` loop changes from `while (true)` to `while (IsRunning())`.
- `main_desktop.cpp` calls `Engine().RequestQuit()` on ESC/quit, then
  `engineThread.join()`, then destroys SDL objects in order before returning normally.
- `std::exit(0)` is removed entirely.
