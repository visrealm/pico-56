set(PROGRAM ${EPISODE}-00-blink)

add_executable(${PROGRAM})

target_sources(${PROGRAM} PRIVATE main.c)

pico_add_extra_outputs(${PROGRAM})

target_link_libraries(${PROGRAM} PRIVATE pico_stdlib)
