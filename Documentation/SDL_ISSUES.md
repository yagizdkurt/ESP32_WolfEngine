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

## WE_Settings.hpp Display Define Patched at CMake Configure Time

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** Medium
**Location:** `desktop/CMakeLists.txt` — configure-time file patch
**What it does now:** `WE_Settings.hpp` intentionally contains a user-facing
`#define DISPLAY_ST7735` that selects the display driver at compile time. This cannot
be overridden by a command-line `-D` flag. The desktop CMake patches a copy of the
file to suppress `DISPLAY_ST7735` and activate `DISPLAY_CUSTOM` instead.
**Impact:** If a new display type is added to `WE_Settings.hpp` (e.g.
`#define DISPLAY_ILI9341`), the CMake patch regex must also suppress that new define.
Otherwise the desktop build will break when the user switches display targets.
**Maintenance note:** The patch logic lives in `desktop/CMakeLists.txt` in the
configure-time `file(READ/WRITE)` block. Any new display define must be added to the
regex there.

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

## ESP_ERROR_CHECK Silently Swallows Errors

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** High
**Location:** `desktop/stubs/esp_err.h`
**What it does now:** `ESP_ERROR_CHECK(x)` expands to `(x)` — the return value is
evaluated and discarded. On real hardware it calls `abort()` on any non-`ESP_OK`
result.
**Impact:** Initialization failures in driver setup (I2C, GPIO, ADC, LEDC) are
completely invisible on desktop. Code that depends on a hard abort to catch
misconfiguration will silently continue in a broken state, masking bugs that would
be immediately obvious on hardware.

---

## ESP_LOGD / ESP_LOGV Permanently Suppressed

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** Low
**Location:** `desktop/stubs/esp_log.h`
**What it does now:** `ESP_LOGD` and `ESP_LOGV` are no-ops (`do {} while(0)`). There
is no way to enable verbose or debug log levels on desktop. All log output goes to
`printf` on stdout.
**Impact:** Debug-level log output that exists in the engine cannot be activated
during desktop development. Any diagnostic instrumentation using `ESP_LOGD`/`ESP_LOGV`
is invisible, making it harder to trace issues in subsystems designed to log at those
levels.
**Maintenance note:** `printf` is not atomic under concurrent writes. Log lines from
the engine thread and any future worker threads may interleave or corrupt each other.

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

**Status:** Active
**Phase found:** Phase 1
**Phase to fix:** TBD
**Severity:** Medium
**Location:** `desktop/stubs/driver/ledc.h`
**What it does now:** `ledc_get_duty()` always returns `0` regardless of channel or
what was previously set via `ledc_set_duty`.
**Impact:** Any code that reads back the current duty cycle to compute a new value
(e.g. fade logic, brightness stepping) will always start from `0` on desktop,
producing different behaviour from hardware.
**Maintenance note:** `ledc_fade_func_install`, `ledc_set_fade_with_time`,
`ledc_fade_start`, and `ledc_fade_stop` are not stubbed. If the engine uses
hardware-assisted fading, the desktop build will fail to compile.

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