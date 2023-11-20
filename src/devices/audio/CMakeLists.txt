cmake_minimum_required(VERSION 3.12)

set(LIBRARY pico-56-audio)

project (${LIBRARY} C)

set(CMAKE_C_STANDARD 11)

add_library(${LIBRARY} STATIC audio.c)

target_include_directories (${LIBRARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${LIBRARY} PRIVATE
        pico_stdlib
        hardware_pwm
        emu2149)