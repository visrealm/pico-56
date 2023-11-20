set(PROGRAM ${EPISODE}-01-testpatt)

add_executable(${PROGRAM})

pico_generate_pio_header(${PROGRAM} ${CMAKE_CURRENT_LIST_DIR}/vga.pio)

target_sources(${PROGRAM} PRIVATE main.c vga.c pio_utils.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma)
