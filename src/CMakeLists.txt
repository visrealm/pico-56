cmake_minimum_required(VERSION 3.12)

set(PICO56_VERSION   "0.6a")

string(REPLACE "." "-" PICO56_VERSION_STR "${PICO56_VERSION}")

set(PROGRAM ${PICO_BOARD}-56-v${PICO56_VERSION_STR})

add_subdirectory(pio-utils)
add_subdirectory(devices)
add_subdirectory(boot-menu)

add_executable(${PROGRAM})

pico_set_program_name(${PROGRAM} "pico-56")
pico_set_program_version(${PROGRAM} ${PICO56_VERSION})
pico_set_program_description(${PROGRAM} "PICO-56")
pico_set_program_url(${PROGRAM} "https://github.com/visrealm/pico-56")


target_sources(${PROGRAM} PRIVATE main.c bus.c rom.c)

pico_add_extra_outputs(${PROGRAM})

pico_enable_stdio_usb(${PROGRAM} 1)
pico_enable_stdio_uart(${PROGRAM} 0)

add_definitions(-DPICO56_VERSION="${PICO56_VERSION}")

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

