# Resolved Issues
This file is a permanent record of known issues that have been identified and resolved
during development. It is append-only — entries are never deleted or edited after they
are written. Active (unresolved) issues live in KNOWN_ISSUES.md.

## Entry Format
Each entry follows this exact structure:

## <Short Title>
RESOLVED in <Phase> - <DD.MM.YY>
**Location:** <file(s) and specific area affected>
**What it did:** <what the problem was and why it was acceptable or unnoticed>
**Resolution (<Phase>):** <what was changed and why, including any design decisions>
- Bullet points for specific code or file changes
- Keep it precise enough that a future developer can understand the decision without
  reading the git diff
**Extra Notes** any extra notes to be informed.
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
---