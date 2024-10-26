cmake_minimum_required(VERSION 3.12)

set(LIBRARY pico-56-nes-ctrl)

project (${LIBRARY} C)

set(CMAKE_C_STANDARD 11)

add_library(${LIBRARY} STATIC nes-ctrl.c)

# generate header file from pio
pico_generate_pio_header(${LIBRARY} ${CMAKE_CURRENT_LIST_DIR}/nes.pio)

target_include_directories (${LIBRARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${LIBRARY} PRIVATE
        pico_stdlib
        hardware_pio)