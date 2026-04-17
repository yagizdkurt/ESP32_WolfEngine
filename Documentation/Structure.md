# Project Structure

## Source Folder:

```
WolfEngine/
│
├── WolfEngine.hpp / .cpp          # Engine singleton — init + game loop
│
├── Settings/                      # Compile-time configuration (no runtime config)
│   ├── WE_Settings.hpp            # Master include: pulls all settings headers
│   ├── WE_Modules.hpp             # Module feature flags: #define SaveLoadModule, etc.
│   ├── WE_PINDEFS.hpp             # All GPIO, SPI, I²C pin numbers
│   ├── WE_InputSettings.hpp       # Per-controller button map, expander type, joystick ADC
│   ├── WE_RenderSettings.hpp      # Background color, game region rect, feature flags
│   ├── WE_Layers.hpp              # RenderLayer enum + CollisionLayer bitmask enum
│   └── WE_SaveSettings.hpp        # Save system: EEPROM chip list, slot definitions, integrity flag
│
├── GameObjectSystem/              # Entity base class and registry
│   ├── WE_GameObject.hpp / .cpp   # Base class, factory, lifecycle, component dispatch
│   └── WE_GORegistry.hpp          # Fixed-size pointer array for all live GameObjects
│
├── ComponentSystem/               # ECS components (no ComponentManager — deleted/refactoring)
│   └── Components/
│       ├── WE_BaseComp.hpp        # Abstract base: ComponentType enum, tick()
│       ├── WE_Comp_Transform.hpp  # Position (Vec2) + width/height — always present on GO
│       ├── WE_Comp_SpriteRenderer.hpp / .cpp  # Sprite asset + palette + rotation; auto-registers with RenderCore
│       ├── WE_Comp_Collider.hpp / .cpp        # Box/Circle shapes; collision + trigger layer bitmasks
│       ├── WE_Comp_Animator.hpp / .cpp        # Frame-strip animation driver on top of SpriteRenderer
│       └── WE_Components.hpp      # Convenience include for all components
│
├── Graphics/
│   ├── RenderSystem/
│   │   ├── WE_RenderCore.hpp / .cpp  # Framebuffer owner; layer management; sprite blit + rotation
│   │   └── WE_Camera.hpp / .cpp      # World↔screen transform; follow target; frustum cull
│   ├── SpriteSystem/
│   │   ├── WE_Sprite.hpp             # Sprite asset: pixel data (palette indices), constexpr factory
│   │   ├── WE_SpriteData.hpp         # Render-ready struct passed from SpriteRenderer → RenderCore
│   │   └── WE_SpriteRotation.hpp     # Rotation enum (R0 / R90 / R180 / R270)
│   ├── ColorPalettes/                # Five built-in 32-entry RGB565 palettes; index 0 = transparent
│   │   ├── WE_Palettes.hpp           # Convenience include
│   │   ├── WE_Palette_Grayscale.hpp
│   │   ├── WE_Palette_Warm.hpp
│   │   ├── WE_Palette_Cool.hpp
│   │   ├── WE_Palette_Gameboy.hpp
│   │   └── WE_Palette_Sunset.hpp
│   └── UserInterface/
│       ├── WE_UIManager.hpp / .cpp   # UI root: owns element array, dirty tracking, render dispatch
│       ├── Fonts/
│       │   └── WE_Font.hpp           # 5×7 bitmap font for ASCII 32–126
│       └── UIElements/
│           ├── Base/
│           │   ├── WE_BaseUIElement.hpp / .cpp  # Abstract element: show/hide, dirty flag, drawPixelRaw()
│           │   ├── WE_UITransform.hpp            # Position, size, margins, UIAnchor enum (9 positions)
│           │   └── WE_UITransformHelpers.hpp     # Anchor resolution math (resolveAnchor, resolveLayout)
│           ├── WE_UILabel.hpp / .cpp   # Text label (32-char max, 5×7 font)
│           ├── WE_UIShape.hpp / .cpp   # Rectangle / HLine / VLine, filled or outline
│           ├── UIPanel.hpp / .cpp      # Container with optional background; translates child positions
│           └── WE_UIElements.hpp       # Convenience include
│
├── InputSystem/
│   ├── WE_InputManager.hpp / .cpp  # Owns all Controller instances; shared ADC handle; tick() per frame
│   ├── WE_IInputProvider.hpp       # Abstract input provider interface; implement to override hardware polling
│   └── WE_Controller.hpp / .cpp    # Single physical controller: GPIO/expander polling, debounce, ADC joystick
│
├── Physics/
│   ├── WE_ColliderManager.hpp / .cpp  # O(n²) pair iteration; shape intersection; Enter/Stay/Exit dispatch
│
├── Sound/
│   ├── WE_SoundManager.hpp / .cpp  # Two PWM channels (music + SFX); note sequencer; loop + callback
│   └── WE_Notes.hpp                # Note_t frequency enum, octaves 1–8 with semitones
│
├── Drivers/
│   ├── DisplayDrivers/
│   │   ├── WE_Display_Driver.hpp         # Abstract display interface (initialize, flush, backlight, sleep)
│   │   └── WE_Display_ST7735.hpp / .cpp  # Concrete: SPI @ 40 MHz, DMA flush, FreeRTOS semaphore
│   ├── EepromDrivers/
│   │   ├── WE_IEEPROMDriver.hpp          # Abstract EEPROM interface (writeBytes, readBytes, eraseAll, totalBytes)
│   │   └── WE_EEPROM24LC512.hpp          # Concrete: 64 KB, 128-byte pages, ACK-polled write cycle
│   └── IODrivers/
│       ├── WE_IExpander.hpp              # Abstract expander interface (begin, pinRead)
│       ├── WE_ExpanderDrivers.hpp        # ExpanderType enum + ExpanderSettings struct
│       ├── WE_PCF8574.hpp                # 8-bit quasi-bidirectional expander (0x20–0x27)
│       ├── WE_PCF8575.hpp                # 16-bit variant of PCF8574
│       └── WE_MCP23017.hpp               # Register-based 16-bit expander, Port A + B, pull-ups
│
├── Modules/                       # Optional engine subsystems managed by ModuleSystem
│   ├── WE_IModule.hpp             # Module base and template
│   ├── WE_ModuleSystem.hpp        #
│   ├── WE_ModuleSystem.cpp        # !! IMPORTANT: THIS IS WHERE MODULES ARE REGISTERED BY MAINTAINERS. !!
│   └── SaveLoadSystem/            # Persistent save data module
│       ├── WE_SaveManager.hpp     # Save Manager Class
│       ├── WE_SaveManager.cpp     #
│       └── WE_SaveSettings.hpp    # EEPROM chip list, slot definitions, integrity flag (used by SaveManager only)
│
└── Utilities/
    ├── WE_I2C.hpp / .cpp          # I²C bus singleton: I2C_NUM_0, 400 kHz, scan, reg read/write
    ├── WE_Time.hpp / .cpp         # Wall clock + game clock (pause-aware); since/elapsed/check helpers
    ├── WE_Timer.hpp / .cpp        # Timer struct wrapping WETime; start/stop/reset/check
    ├── WE_Vector2d.hpp            # Vec2 (float) + IntVec2; full operator set; lerp, dist, fromAngle
    └── WE_Debug.h                 # DebugLog/DebugErr macros → ESP_LOGI/LOGE; no-op when disabled

GameCode
└── Your actual code for game development

Main.cpp ── Entry point

```

**Non-obvious notes:**
-
-
-

## Documentation Folder:
```
Documentation/
│
├── architecture.md      # AI-oriented context: architecture, design decisions, constraints, patterns
├── Documentation.md     # Human-readable documentation: system overview, modules, usage, examples
├── Structure.md         # Project structure reference (this file)
├── KNOWN_ISSUES.md      # Active known issues and current limitations
├── SOLVED_ISSUES.md     # Resolved issues archive
└── SDL_ISSUES.md        # Desktop/SDL-specific issues and workarounds
```

## Desktop Folder:
```
desktop/
│
├── main_desktop.cpp                # Desktop entry point — mirrors app_main(), runs engine on background thread
├── WE_SDLManager.hpp / .cpp        # SDL3 window + renderer lifecycle; init/shutdown/pollEvents
├── WE_SDLInputDriver.hpp / .cpp    # SDL3 event → WE_Controller translation
├── WE_SDLKeyMap.hpp                # SDL3 keycode → WE button mapping table
├── CMakeLists.txt                  # CMake build for desktop target
├── Buildit.bat                     # Windows helper script to configure + build via CMake
├── build/                          # CMake build artifacts (not tracked)
└── stubs/                          # ESP-IDF API stubs — let WolfEngine compile on desktop
    ├── WE_Display_SDL3.hpp / .cpp
    ├── Display_SDL.h
    ├── esp_err.h / esp_log.h / esp_timer.h
    ├── driver/gpio.h / i2c.h / ledc.h
    ├── esp_adc/adc_oneshot.h
    └── freertos/FreeRTOS.h / semphr.h
```

## Non Source Folders and items:
- `IconsPhotosEtc/` — assets for README only, not used in development
- `test/` — PlatformIO test directory (currently empty)
- `cxx_only_flags.py` — extra script: applies C++-only flags that ESP-IDF