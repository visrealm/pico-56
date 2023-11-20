set(PROGRAM ${EPISODE}-03-gfx2)

add_executable(${PROGRAM})


# generate image array source files from png images
visrealm_generate_bindata_source(${PROGRAM} breakout res/BREAKOUT.* )

target_sources(${PROGRAM} PRIVATE main.c font.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        vrEmuTms9918
        vrEmuTms9918Util
        pico-56-vga
        pico_stdlib)
