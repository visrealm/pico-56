set(PROGRAM ${EPISODE}-00-sync)

add_executable(${PROGRAM})

# generate header file from pio
pico_generate_pio_header(${PROGRAM} ${CMAKE_CURRENT_LIST_DIR}/vga.pio)

target_sources(${PROGRAM} PRIVATE main.c vga.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma)
