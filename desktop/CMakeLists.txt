cmake_minimum_required(VERSION 3.16)
project(WolfEngine_Desktop CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SDL3 ──────────────────────────────────────────────────────────────────────
# On Windows, set SDL3_DIR to the cmake/ subdirectory of your SDL3 dev package:
#   cmake -B build -S . -DSDL3_DIR="C:/SDL3/cmake"
find_package(SDL3 REQUIRED)

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
    WE_SDLManager.cpp
    stubs/WE_Display_SDL3.cpp
    WE_SDLInputDriver.cpp
    ${ENGINE_SRCS}
)

# ── Include paths ─────────────────────────────────────────────────────────────
# stubs/ must come BEFORE any system paths so our fake ESP-IDF headers shadow
# the real ones (or satisfy includes on a host that has no ESP-IDF at all).
target_include_directories(WolfEngine_Desktop PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"                        # fake ESP-IDF headers
    "${CMAKE_CURRENT_SOURCE_DIR}"                              # desktop-specific headers (WE_SDLInputDriver.hpp etc.)
    "${CMAKE_CURRENT_SOURCE_DIR}/.."                           # engine root (WolfEngine/ includes)
    "${CMAKE_CURRENT_SOURCE_DIR}/../WolfEngine/Settings"       # relative includes inside WE_Settings.hpp
    ${SDL3_INCLUDE_DIRS}
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(WolfEngine_Desktop PRIVATE
    WE_DESKTOP_BUILD=1
    DISPLAY_SDL
    WE_PLATFORM_SDL
)

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(WolfEngine_Desktop PRIVATE ${SDL3_LIBRARIES})

# ── Convenience message ───────────────────────────────────────────────────────
message(STATUS "WolfEngine_Desktop: engine sources = ${ENGINE_SRCS}")
