# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico CACHE STRING "Board type")

# pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# pull in helpers for my custom tools
include(visrealm_tools.cmake)

set(BUILD_SHARED_LIBS OFF)

set(PROJECT pico-56)

project(${PROJECT} C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_subdirectory(submodules/vrEmu6502)
add_subdirectory(submodules/vrEmu6522)
add_subdirectory(submodules/vrEmuTms9918)
add_subdirectory(submodules/sdcard/src)
add_subdirectory(submodules/emu2149)

add_subdirectory(src)

add_subdirectory(episodes)

