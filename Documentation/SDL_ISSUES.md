# Known Issues

This file tracks active, unresolved issues in the desktop build. Each entry describes
a behavioural difference, missing stub, or deferred implementation that could cause
confusion or incorrect results during development.

When an issue is resolved, move the full entry to RESOLVED_ISSUES.md with a
Resolution section added. Do not delete entries from this file — strike-through is
not sufficient.

---

## Entry Format

## <Short Title>
**Status:** Active | Deferred | Needs Investigation
**Phase found:** <Phase N>
**Phase to fix:** <Phase N> | TBD
**Severity:** Low | Medium | High
**Location:** <file(s) and specific area>
**What it does now:** <current stub or placeholder behaviour>
**Impact:** <what could go wrong or be misleading during development>
**Maintenance note:** <optional — what to watch for when touching related code>

---

## Severity Guide

- **High** — can produce silently wrong results that look correct, or will cause a
  compile error when the related subsystem is wired up
- **Medium** — produces obviously wrong or missing behaviour, but will not mislead;
  the difference from hardware is visible and expected
- **Low** — cosmetic, negligible runtime impact, or only relevant in edge cases

---

## Rules for AI Assistants

- Do not mark issues as resolved here — move them to RESOLVED_ISSUES.md instead.
- When adding a new issue, always include Status, Phase found, Phase to fix,
  Severity, Location, What it does now, and Impact. Never leave fields blank.
- When an issue is partially addressed (e.g. a stub improved but not fully replaced),
  update the entry in place and add a note — do not move it to RESOLVED_ISSUES.md.
- If a new phase introduces new stubs or placeholders, add their issues here
  immediately as part of that phase's changelog.
- Severity must be reassessed if the affected subsystem is wired up in a new phase.

---

## FreeRTOS Stub Behaviour Differences

**Status:** Deferred
**Phase found:** Phase 1
**Phase to fix:** Phase 3
**Severity:** Medium
**Location:** `desktop/stubs/freertos/`
**What it does now:** `vTaskDelay` is a no-op. `xSemaphoreTake` always returns
`pdTRUE`. `pdMS_TO_TICKS` is a straight millisecond passthrough with no tick-rate
conversion.
**Impact:**
- Any engine code that uses `vTaskDelay` for timing or hardware settle delays will
  skip those waits entirely on desktop. Initialization order that depends on these
  delays may differ from hardware.
- No real locking occurs anywhere semaphores are used. Safe while the engine runs on
  a single thread, but will cause race conditions if real desktop threading is
  introduced.
- Tick-based math produces different results from hardware where tick rate affects
  the conversion.
**Maintenance note:** Reassess all three items when real timing and threading are
introduced in Phase 3.

---



## esp_timer Desktop Resolution is OS-Dependent

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** Phase 3
**Severity:** Medium
**Location:** `desktop/stubs/esp_timer.h`
**What it does now:** `esp_timer_get_time()` uses `std::chrono::steady_clock`.
Precision and jitter are platform- and OS-scheduler-dependent. Windows in particular
has historically had coarser timer resolution than hardware timers.
**Impact:** Timing-sensitive subsystems (delta-time calculation, animation step,
fixed-update loops) may behave correctly on ESP32 but exhibit visible frame-rate
variation or drift on desktop, making it harder to distinguish engine bugs from
platform differences.

---

## vTaskDelay Declared in FreeRTOS.h Instead of task.h

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** Medium
**Location:** `desktop/stubs/freertos/FreeRTOS.h`
**What it does now:** `vTaskDelay` is defined directly in `FreeRTOS.h`. In real
FreeRTOS it lives in `task.h` — `FreeRTOS.h` only provides core primitives.
**Impact:** Any engine source file that calls `vTaskDelay` but only includes
`<freertos/FreeRTOS.h>` (not `<freertos/task.h>`) will compile cleanly on desktop
but fail on hardware where the symbol is absent from `FreeRTOS.h`. This masks a
missing-include class of bug.

---

## Semaphore Create Functions Return nullptr

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** High
**Location:** `desktop/stubs/freertos/semphr.h`
**What it does now:** `xSemaphoreCreateBinary()` and `xSemaphoreCreateMutex()` both
return `nullptr`. On real FreeRTOS a `NULL` return means allocation failed.
**Impact:** Engine code that checks `if (handle == NULL)` after creating a semaphore
will always take the error branch on desktop. Code that skips the check and calls
`xSemaphoreTake`/`xSemaphoreGive` with a null handle is formally undefined behaviour,
even though the stub functions currently accept `void*` and ignore it.
**Maintenance note:** `xSemaphoreGiveFromISR` always writes `pdFALSE` to
`pxHigherPriorityTaskWoken`. Any ISR-driven wake-up logic will never fire on desktop.

---

## GPIO Reads Always Return 0

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** Phase 2 (Input)
**Severity:** High
**Location:** `desktop/stubs/driver/gpio.h`
**What it does now:** `gpio_get_level()` always returns `0` regardless of pin.
**Impact:** Any input logic that polls GPIO pins (buttons, direction pads, hardware
signals) will always read "not pressed / low". Logic that waits for a rising edge or
high level will never proceed. Edge-detection code cannot be tested on desktop.
**Maintenance note:** `gpio_install_isr_service`, `gpio_isr_handler_add`, and
`gpio_isr_handler_remove` are not stubbed. If any subsystem registers GPIO interrupts,
the desktop build will fail to compile with an undefined-symbol error.

---

## I2C Read Functions Never Populate Output Buffer

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** High
**Location:** `desktop/stubs/driver/i2c.h`
**What it does now:** `i2c_master_read()` and `i2c_master_read_byte()` accept a
`uint8_t*` output buffer but never write to it. Buffer contents remain uninitialised
after the call. `i2c_cmd_link_create()` returns `nullptr`.
**Impact:** Any subsystem that performs an I2C read will operate on garbage data
without any error signal. The full I2C transaction sequence compiles and runs without
error but does absolutely nothing, making it impossible to test command-building logic
on desktop.

---

## LEDC Duty Reads Always Return 0

**Status:** Dormant — no fade or brightness logic currently exists in the renderer
**Phase found:** Phase 1
**Phase to fix:** When fade/brightness logic is added
**Severity:** Low (medium if fade logic is added)
**Location:** `desktop/stubs/driver/ledc.h`
**What it does now:** `ledc_get_duty()` always returns `0` regardless of channel or
what was previously set via `ledc_set_duty`.
**Impact:** No current impact. If fade or brightness stepping logic is added to the
renderer (e.g. backlight fade-in on startup, dim on inactivity, brightness menu option),
any code that reads back the current duty cycle via `ledc_get_duty()` to compute the
next step will always see `0` on desktop, causing the fade to always restart from the
bottom regardless of actual state. Behaviour will silently diverge from hardware.
**When this becomes relevant:** The moment any of these are implemented in the renderer:
- Backlight fade-in / fade-out
- Brightness stepping or ramping
- Inactivity dimming
- A brightness setting that reads current level before adjusting
**Fix needed at that point:** Replace the always-0 stub with a small in-memory state
table that `ledc_set_duty()` writes to and `ledc_get_duty()` reads back from, so the
desktop stub stays internally consistent with what was last set.
**Maintenance note:** `ledc_fade_func_install`, `ledc_set_fade_with_time`,
`ledc_fade_start`, and `ledc_fade_stop` are not stubbed. If hardware-assisted fading
is added to the renderer, the desktop build will fail to compile with unresolved symbol
errors. These stubs must be added at the same time as any fade logic.

---

## ADC Always Returns Mid-Range; Analog Inputs Frozen

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** Phase 2 (Input)
**Severity:** High
**Location:** `desktop/stubs/esp_adc/adc_oneshot.h`
**What it does now:** `adc_oneshot_read()` always writes `2048` to the output — the
exact midpoint of the 12-bit range.
**Impact:** Any analog input (joystick axes, potentiometers, analog buttons) is
permanently frozen at the center/neutral value. Threshold-based input code can never
be triggered on desktop, making analog input paths completely untestable.
**Maintenance note:** `adc_cali_*` calibration functions are not stubbed. If the
engine uses calibrated ADC readings, the desktop build will fail to compile.

---

## Screen Resolution Hardcoded, Not Derived from Settings

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** Low
**Location:** `desktop/stubs/Display_SDL.h`, `desktop/stubs/WE_Display_SDL3.hpp`,
`desktop/main_desktop.cpp`
**What it does now:** `RENDER_SCREEN_WIDTH` and `RENDER_SCREEN_HEIGHT` are defined as
compile-time literals (`128` / `160`) in `Display_SDL.h`. The SDL window is created
with hardcoded `512, 640` instead of `RENDER_SCREEN_WIDTH * 4, RENDER_SCREEN_HEIGHT * 4`.
**Impact:** If the target hardware display changes resolution, the desktop build must
be updated manually in multiple places with no single source of truth shared between
hardware and desktop builds.
**Maintenance note:** Window size should be `RENDER_SCREEN_WIDTH * 4,
RENDER_SCREEN_HEIGHT * 4` so it scales automatically if logical resolution changes.

---

## SDL Audio Not Initialised

**Status:** Deferred
**Phase found:** Phase 1
**Phase to fix:** Phase 5 (Sound)
**Severity:** Low
**Location:** `desktop/main_desktop.cpp` — `SDL_Init(SDL_INIT_VIDEO)`
**What it does now:** SDL is initialised with `SDL_INIT_VIDEO` only.
**Impact:** When the sound subsystem (`WE_SoundManager`) is wired to a real SDL audio
backend, `SDL_Init` must also receive `SDL_INIT_AUDIO`. This is a known deferred step
that may be overlooked when Phase 5 work begins.

---

## Controller 0 Only

**Status:** Active
**Phase found:** Phase 3
**Phase to fix:** TBD
**Severity:** Medium
**Location:** `src/desktop/WE_SDLInputDriver.cpp` — `SDLInputDriver::flush()`
**What it does now:** Only controller 0 receives SDL input. Controllers 1–3 always
report no input on desktop.
**Impact:** Multi-player games cannot be tested on desktop. Missing input on controllers
1–3 is silent — no error or warning is produced.
**Maintenance note:** Any future multi-player feature will need a keyboard split scheme
or gamepad support added to `SDLInputDriver` before it can be tested on desktop.

---

## getController(0) Always Returns Non-Null on SDL

**Status:** Active
**Phase found:** Phase 3
**Phase to fix:** TBD
**Severity:** Low
**Location:** `src/WolfEngine/InputSystem/WE_InputManager.cpp` — `getController()`
**What it does now:** The `enabled` flag in `ControllerSettings` is ignored for index 0
on desktop. An SDL-specific early-return bypasses the enabled check entirely.
**Impact:** Behaviour diverges silently from ESP32 — code that disables controller 0 in
settings will still receive input on desktop. Intentional design decision, but could
mask misconfigured settings during development.
**Maintenance note:** If `getController()` logic is ever refactored, preserve the SDL
early-return for index 0 or document the removal explicitly.

---

## Keyboard-Only Joystick — No Analog Range

**Status:** Active
**Phase found:** Phase 3
**Phase to fix:** TBD
**Severity:** Medium
**Location:** `src/desktop/WE_SDLInputDriver.cpp` — `SDLInputDriver::flush()`
**What it does now:** Joystick axes are driven by WASD keys and can only output discrete
values: `-1.0`, `0.0`, or `+1.0`. No analog ramp. Diagonal input produces `(1.0, -1.0)`.
**Impact:** Any game logic that depends on analog axis values between `-1.0` and `1.0`
(acceleration curves, variable speed, analog aiming) cannot be tested on desktop. The
behaviour difference is visible and expected, not silent.
**Maintenance note:** USB gamepad support via SDL's joystick/gamepad API would resolve
this. Track as a future improvement if analog testing becomes necessary.

---

## Debounce Bypassed on Desktop

**Status:** Active
**Phase found:** Phase 3
**Phase to fix:** TBD
**Severity:** Low
**Location:** `src/WolfEngine/InputSystem/WE_Controller.cpp` — `simulateButton()`
**What it does now:** `simulateButton` writes directly to `m_currState` / `m_prevState`.
The debounce window in `InputSettings::debounceMs` has no effect on desktop.
**Impact:** Input that relies on debounce timing behaving identically to hardware will
not be reproducible on desktop. In practice this is unlikely to matter for game logic,
but hardware-specific timing tests are invalid on desktop.
**Maintenance note:** Key repeat events from the OS are already filtered via the atomic
bitmask (set on first down, cleared on up), so spurious `getButtonDown` firings are not
a concern.

---

## Controller::tick() Not Called on SDL

**Status:** Active
**Phase found:** Phase 3
**Phase to fix:** TBD
**Severity:** Medium
**Location:** `src/WolfEngine/InputSystem/WE_InputManager.cpp` — `InputManager::tick()`
**What it does now:** The GPIO/ADC polling path is entirely skipped on desktop.
`InputManager::tick()` calls `SDLInput_flush` instead of the normal `Controller::tick()`
loop.
**Impact:** Any per-frame side-effects added to `Controller::tick()` in the future will
silently not run on desktop unless an explicit SDL-path equivalent is added to
`WE_SDLInputDriver.cpp`.
**Maintenance note:** Every time `Controller::tick()` gains new responsibilities, assess
whether the SDL path needs a matching update. This is a recurring maintenance risk as
the engine grows.

---

## Key Repeat Events Ignored

**Status:** Active
**Phase found:** Phase 3
**Phase to fix:** TBD
**Severity:** Low
**Location:** `src/desktop/WE_SDLInputDriver.cpp` — `SDLInputDriver::processEvent()`
**What it does now:** SDL fires repeated `SDL_EVENT_KEY_DOWN` events while a key is held
(OS key-repeat). The driver ignores these — state is stored as a bitmask set on first
down and cleared on up, so repeat events are no-ops.
**Impact:** No phantom button presses. Intentional behaviour, but means OS key-repeat
cannot be used as an input mechanism if ever needed (e.g. menu navigation auto-repeat).
**Maintenance note:** If menu or UI systems want to leverage key-repeat for held
navigation, a separate repeat counter would need to be added to `SDLInputDriver` rather
than relying on OS events.

---
