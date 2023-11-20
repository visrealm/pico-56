cmake_minimum_required(VERSION 3.12)

set(LIBRARY pico-56-boot-menu)

project (${LIBRARY} C)

set(CMAKE_C_STANDARD 11)

add_library(${LIBRARY} STATIC boot-menu.c font.c input.c)

target_include_directories (${LIBRARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${LIBRARY} PRIVATE pico-56-tms9918 pico-56-sdcard pico-56-ps2-kbd pico-56-nes-ctrl)