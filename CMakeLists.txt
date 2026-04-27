cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Game1) # Rename this to match your project name

find_package(Python3 REQUIRED)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import PIL"
    RESULT_VARIABLE PILLOW_RESULT
)

if(NOT PILLOW_RESULT EQUAL 0)
    message(STATUS "Pillow not found — installing automatically...")
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pip install Pillow
        RESULT_VARIABLE PIP_RESULT
    )
    if(NOT PIP_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Failed to install Pillow automatically. "
            "Run manually: pip install -r tools/requirements.txt")
    endif()
    # Verify install actually worked
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import PIL"
        RESULT_VARIABLE PILLOW_RECHECK
    )
    if(NOT PILLOW_RECHECK EQUAL 0)
        message(FATAL_ERROR
            "Pillow installed but still not importable. "
            "Try manually: pip install -r tools/requirements.txt")
    endif()
    message(STATUS "Pillow installed successfully.")
endif()

add_custom_target(asset_converter ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/src/GeneratedAssets
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/asset_converter.py
        ${CMAKE_SOURCE_DIR}/Images
        ${CMAKE_SOURCE_DIR}/src/GeneratedAssets
        ${CMAKE_SOURCE_DIR}/tools/palettes
    COMMENT "Converting image assets..."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# ESP-IDF exposes the firmware target as 'app'.
# The guard prevents errors during initial CMake configuration.
if(TARGET app)
    add_dependencies(app asset_converter)
endif()
