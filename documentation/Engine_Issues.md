# Engine Known Issues

This file tracks active, unresolved issues in the WolfEngine core — design limitations,
performance concerns, and missing abstractions that may need to be addressed as the engine
grows. These are engine-level issues, not desktop-build-specific issues. Desktop build
issues live in KNOWN_ISSUES.md.

When an issue is resolved, move the full entry to RESOLVED_ISSUES.md with a Resolution
section added. Do not delete entries from this file — strike-through is not sufficient.

---

## Entry Format

## <Short Title>
**Status:** Active | Deferred | Needs Investigation
**Severity:** Low | Medium | High
**Location:** <file(s) and specific area>
**What it is:** <description of the limitation or design gap>
**Impact:** <what goes wrong or becomes painful as the engine grows>
**Maintenance note:** <optional — what to watch for when touching related code>

---

## Severity Guide

- **High** — will cause visible bugs, crashes, or compile failures under realistic
  usage conditions. Needs a fix before the engine is used seriously.
- **Medium** — produces incorrect or suboptimal behaviour in specific scenarios.
  Should be fixed before the affected subsystem is relied upon heavily.
- **Low** — tolerable at current scale. Will become a problem if usage grows beyond
  current limits or if the engine is used in a larger project.

---

## Rules for AI Assistants

- Do not mark issues as resolved here — move them to RESOLVED_ISSUES.md instead.
- When adding a new issue, always include Status, Severity, Location, What it is,
  and Impact. Never leave fields blank.
- When an issue is partially addressed, update the entry in place and add a note —
  do not move it to RESOLVED_ISSUES.md.
- Severity must be reassessed if the affected subsystem changes significantly.
- If a new feature introduces a known limitation, add it here immediately as part
  of that feature's changelog.

---


## One-Frame Position Lag in Sprite Rendering

**Status:** Deferred  
**Severity:** Low  
**Location:** `src/WolfEngine/WolfEngine.cpp` — `gameTick()` tick ordering  
**What it is:** `SpriteRenderer::tick()` submits draw commands during `componentTick()`, which runs before `Update()` and `camera.followTick()`. Commands are submitted with last frame's positions and camera state.  
**Impact:** Sprites render one frame behind their actual game logic position. Invisible at 30fps for normal movement. Becomes noticeable with fast-moving objects, projectiles, or a physics debug overlay requiring current-frame accuracy.  
**Maintenance note:** Fix requires splitting `render()` into `beginFrame()` before `componentTick` and `executeAndFlush()` after `camera.followTick()` in `gameTick()`. Defer until frame-accurate rendering is visibly needed.

---


## Public Interfaces in Active Flux

**Status:** Needs Investigation
**Severity:** High
**Location:** Engine-wide — all public headers
**What it is:** Public API surfaces (method signatures, struct layouts, module
interfaces) may change without notice as the engine develops. There is no stability
contract or versioning on any interface.
**Impact:** Game code written against the current API may break silently between
engine updates. Refactors to core systems (renderer, input, camera) risk cascading
changes across all game code that depends on them.
**Maintenance note:** Consider marking stable interfaces explicitly once the engine
reaches a usable milestone.

---

## Hardcoded Camera Lerp Factor

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Graphics/RenderSystem/WE_Camera.cpp`
**What it is:** The camera follow interpolation factor is a numeric literal constant
inside `WE_Camera.cpp`. It is not exposed in `WE_Settings.hpp` or any public API.
**Impact:** Game developers cannot tune camera feel per-scene or per-game-object
without editing engine source. A fast-moving game and a slow puzzle game need
different follow speeds, and there is currently no way to configure this without
touching the engine.
**Maintenance note:** The follow speed should be a configurable field on `Camera`
or a value in `WE_RenderSettings`.

---

## Coarse UI Dirty Tracking

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Graphics/UserInterface/WE_UIManager.cpp`
**What it is:** `UIManager::render()` redraws all UI elements whenever any single
element is marked dirty. There is no per-element or per-region dirty tracking.
**Impact:** Tolerable on a 128×160 screen with a small number of elements. As UI
element count grows, unnecessary redraws will consume increasing CPU time per frame,
potentially pushing the engine below its target frame rate.
**Maintenance note:** A per-element dirty flag or a dirty-region rect would contain
the cost. Not worth addressing until UI complexity actually causes frame time issues.

---

## O(n²) Collision Detection

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Physics/WE_ColliderManager.cpp`
**What it is:** Collision detection checks every collider against every other collider
— O(n²) complexity. `MAX_COLLIDERS` is currently 64.
**Impact:** At 64 colliders the cost is fixed and tolerable. If `MAX_COLLIDERS` ever
rises significantly (e.g. to 256+), collision detection will dominate frame time.
**Maintenance note:** A spatial hash or broad-phase pass (AABB sweep) would reduce
average complexity to O(n log n) or better. Only worth implementing if the collider
limit needs to grow.

---

## No Audio Volume or Mixing

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Sound/WE_SoundManager.cpp`
**What it is:** Two PWM buzzer channels run at a fixed duty cycle. There is no
relative volume control between music and SFX channels, no fade-in/out, and no
mixing of simultaneous tones beyond the two fixed channels.
**Impact:** All sounds play at the same perceived volume. Games that need quiet
background music under loud SFX, or smooth audio transitions, cannot achieve this
without a design change to `WE_SoundManager`.
**Maintenance note:** Volume control via PWM duty cycle modulation is possible within
the current architecture but requires API additions to `WE_SoundManager`.

---

## No Scene or State Management

**Status:** Active
**Severity:** Low
**Location:** Engine-wide — no `Scene`, `GameState`, or `StateMachine` exists
**What it is:** There is no built-in abstraction for game scenes, states, or
transitions. The user manages all game state manually in game code.
**Impact:** A growing game will need to invent its own scene management, leading to
ad-hoc solutions that are hard to maintain. Common patterns (main menu → gameplay →
pause → game over) have no engine support and must be implemented from scratch each
time.
**Maintenance note:** A lightweight `IScene` interface with `enter()`, `update()`,
and `exit()` methods would cover most use cases without significant engine complexity.

---

## Sprite Pixel Data is Read-Only Flash Asset

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Graphics/` — `WE_Sprite` and related types
**What it is:** Sprite pixel data is stored as `const uint8_t*` pointing to
`constexpr` arrays in flash. Sprites are ROM assets only — there is no mechanism for
runtime-generated sprites or partial pixel updates.
**Impact:** Any use case that requires dynamic sprite content (procedural generation,
runtime image loading, particle effects with unique pixels) cannot be supported without
a design change to `WE_Sprite` that introduces a RAM-backed pixel buffer.
**Maintenance note:** RAM on ESP32 is scarce — any RAM-backed sprite design must
account for heap fragmentation and total RAM budget carefully.

---

## No Error Propagation on I²C Operations

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Utilities/WE_I2C.cpp` and all I²C callers
(expanders, EEPROM)
**What it is:** `WE_I2C` functions return `esp_err_t` but callers — expanders and
EEPROM drivers — do not consistently propagate or log non-OK results. Errors are
silently discarded.
**Impact:** A loose connector, wrong I²C address, or hardware fault will produce
silent failures with no diagnostic output. The engine will continue running as if
the operation succeeded, leading to confusing downstream behaviour (wrong input
readings, corrupted saves).
**Maintenance note:** At minimum, callers should log `ESP_LOGE` on non-OK results.
Propagating errors up to the game layer would require API changes.

---

## EEPROM Write Delay Blocks Game Loop

**Status:** Active
**Severity:** Low
**Location:** `src/WolfEngine/Modules/SaveLoadSystem/WE_SaveManager.cpp` —
`writeBytes()` implementation
**What it is:** `writeBytes()` calls `vTaskDelay(pdMS_TO_TICKS(5))` between page
writes to satisfy EEPROM write-cycle timing. Writing more than approximately 128
bytes stalls the calling thread for a perceptible duration.
**Impact:** Saving large data structures during active gameplay will cause visible
frame drops or stutter. The longer the save data, the worse the stall.
**Maintenance note:** Save operations should only be triggered outside active
gameplay (pause screen, level transition, game over) or offloaded to a background
FreeRTOS task. Do not call save functions from within `gameTick()`.