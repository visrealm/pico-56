set(PROGRAM ${EPISODE}-08b-image-800)

add_executable(${PROGRAM})

# generate header file from pio
pico_generate_pio_header(${PROGRAM} ${CMAKE_CURRENT_LIST_DIR}/vga.pio)

# generate image array source files from png images
visrealm_generate_image_source(${PROGRAM} image '' /res/birds.png)

target_sources(${PROGRAM} PRIVATE main.c vga.c pio_utils.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma)
