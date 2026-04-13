cmake_minimum_required(VERSION 3.16)
project(WolfEngine_Desktop CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SDL3 ──────────────────────────────────────────────────────────────────────
# On Windows, set SDL3_DIR to the cmake/ subdirectory of your SDL3 dev package:
#   cmake -B build -S . -DSDL3_DIR="C:/SDL3/cmake"
find_package(SDL3 REQUIRED)

# ── Patched WE_Settings.hpp ───────────────────────────────────────────────────
# WE_Settings.hpp hardcodes `#define DISPLAY_ST7735` as an in-source directive.
# A CMake -DDISPLAY_CUSTOM flag cannot suppress a #define that fires inside a
# header, so both macros end up defined simultaneously and the ST7735 branch in
# WE_RenderCore.hpp wins.
#
# Fix: read the real file at configure time, swap the display define, and write
# the patched copy into the build tree.  CMAKE_CURRENT_BINARY_DIR is prepended
# to the include path so the patched file is found before the original.
# WolfEngine/Settings is also added so the patched file's own relative
# includes ("WE_PINDEFS.hpp" etc.) still resolve from their original directory.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../WolfEngine/Settings/WE_Settings.hpp"
     _settings_src)
string(REPLACE
    "#define DISPLAY_ST7735"
    "// #define DISPLAY_ST7735  // suppressed by desktop build\n#define DISPLAY_SDL"
    _settings_patched "${_settings_src}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/WolfEngine/Settings")
file(WRITE
    "${CMAKE_CURRENT_BINARY_DIR}/WolfEngine/Settings/WE_Settings.hpp"
    "${_settings_patched}")

# ── Engine sources ────────────────────────────────────────────────────────────
# CONFIGURE_DEPENDS: if a .cpp is added/removed under src/WolfEngine/, CMake
# automatically re-globs on the next build rather than silently missing it.
file(GLOB_RECURSE ENGINE_SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../WolfEngine/*.cpp"
)

# Exclude the ST7735 driver — it owns GetDriver() and has unresolvable
# ESP-LCD / SPI / FreeRTOS dependencies. Our stubs/WE_Display_SDL3.cpp replaces it.
list(FILTER ENGINE_SRCS EXCLUDE REGEX "WE_Display_ST7735\\.cpp$")

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(WolfEngine_Desktop
    main_desktop.cpp
    stubs/WE_Display_SDL3.cpp
    ${ENGINE_SRCS}
)

# ── Include paths ─────────────────────────────────────────────────────────────
# stubs/ must come BEFORE any system paths so our fake ESP-IDF headers shadow
# the real ones (or satisfy includes on a host that has no ESP-IDF at all).
target_include_directories(WolfEngine_Desktop PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"                               # patched WE_Settings.hpp (must be first)
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"                        # fake ESP-IDF headers
    "${CMAKE_CURRENT_SOURCE_DIR}/.."                           # engine root (WolfEngine/ includes)
    "${CMAKE_CURRENT_SOURCE_DIR}/../WolfEngine/Settings"       # relative includes inside WE_Settings.hpp
    ${SDL3_INCLUDE_DIRS}
)

# ── Compile definitions ───────────────────────────────────────────────────────
# DISPLAY_SDL activates the #elif branch in WE_RenderCore.hpp, which then
# includes our stubs/Display_SDL.h instead of esp_lcd_st7735.h.
target_compile_definitions(WolfEngine_Desktop PRIVATE
    WE_DESKTOP_BUILD=1
)

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(WolfEngine_Desktop PRIVATE ${SDL3_LIBRARIES})

# ── Convenience message ───────────────────────────────────────────────────────
message(STATUS "WolfEngine_Desktop: engine sources = ${ENGINE_SRCS}")
