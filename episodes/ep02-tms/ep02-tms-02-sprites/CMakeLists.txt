set(PROGRAM ${EPISODE}-02-sprites)

add_executable(${PROGRAM})

target_sources(${PROGRAM} PRIVATE main.c font.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        vrEmuTms9918
        vrEmuTms9918Util
        pico-56-vga
        pico_stdlib)
