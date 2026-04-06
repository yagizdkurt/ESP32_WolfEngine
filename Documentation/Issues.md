# Known Issues, Gaps & Technical Debt

This section documents what a new developer will likely stumble on.

| Issue | Severity | Detail |
|---|---|---|
| **API in active flux** | High | Public interfaces may change. |
| **Hardcoded camera lerp** | Low | Camera follow interpolation factor is a literal constant inside `WE_Camera.cpp`, not exposed in `WE_Settings.hpp`. |
| **Coarse UI dirty tracking** | Low | `UIManager::render()` redraws all elements when any element is dirty. For a 128×160 screen this is tolerable, but will become visible if the UI element count grows. |
| **O(n²) collision detection** | Low | Fine at `MAX_COLLIDERS = 64`. Would need a spatial hash or broad-phase if the limit ever rises significantly. |
| **No audio volume / mixing** | Low | Two PWM channels at fixed duty cycle. No fade-in/out, no relative volume between music and SFX. |
| **No scene / state management** | Low | There is no `Scene`, `GameState`, or `StateMachine` abstraction. The user manages game states manually in game code. A growing game will need to invent this. |
| **Sprite pixel data is `const uint8_t*`** | Low | Sprites are ROM assets only. No runtime-generated sprites or partial updates without a design change to `WE_Sprite`. |
| **No error handling on I²C** | Low | `WEI2C` returns `esp_err_t` but callers (expanders, EEPROM) do not consistently propagate or log errors. A loose connector will produce silent failures. |
| **5 ms EEPROM write delay is blocking** | Low | `writeBytes()` calls `vTaskDelay(pdMS_TO_TICKS(5))` between pages. Writing more than ~128 bytes will stall the game loop for perceptible time. Write save data only outside active gameplay or from a background task. |