set(PROGRAM pico-56)

add_subdirectory(pio-utils)
add_subdirectory(devices)
add_subdirectory(boot-menu)

add_executable(${PROGRAM})

target_sources(${PROGRAM} PRIVATE main.c bus.c rom.c)

pico_add_extra_outputs(${PROGRAM})

pico_enable_stdio_usb(${PROGRAM} 1)
pico_enable_stdio_uart(${PROGRAM} 0)

target_link_libraries(${PROGRAM} PRIVATE
        pico-56-tms9918
        pico-56-ps2-kbd
        pico-56-nes-ctrl
        pico-56-audio
        pico-56-boot-menu
        pico-56-interrupts
        pico-56-sdcard
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma
        hardware_pwm
        vrEmu6502
        vrEmu6522        
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico)

