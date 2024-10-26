set(PROGRAM ${EPISODE}-00-color)

add_executable(${PROGRAM})

target_sources(${PROGRAM} PRIVATE main.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE
        vrEmuTms9918
        vrEmuTms9918Util
        pico-56-vga
        pico_stdlib)
