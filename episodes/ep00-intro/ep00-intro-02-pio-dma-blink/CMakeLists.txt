set(PROGRAM ${EPISODE}-02-pio-dma-blink)

add_executable(${PROGRAM})

target_sources(${PROGRAM} PRIVATE main.c)

# generate header file from pio
pico_generate_pio_header(${PROGRAM} ${CMAKE_CURRENT_LIST_DIR}/dma-blink.pio)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE pico_stdlib hardware_pio hardware_dma)
