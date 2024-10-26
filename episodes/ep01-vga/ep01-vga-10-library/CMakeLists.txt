set(PROGRAM ${EPISODE}-10-library)

add_executable(${PROGRAM})

# generate image array source files from png images
visrealm_generate_image_source(${PROGRAM} slides res/*.png )

target_sources(${PROGRAM} PRIVATE main.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        pico_stdlib
        pico-56-vga)
