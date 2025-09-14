# PICO-56
My [HBC-56](https://github.com/visrealm/hbc-56), a 65C02/TMS9918A homebrew computer on a backplane, fully emulated on a Raspberry Pi Pico.

<p align="left"><a href="https://github.com/visrealm/hbc-56"><img src="img/thumb.png" alt="HBC-56" width="720px"></a></p>

Emulating the following HBC-56 hardware
* 65C02 CPU
* 65C22 VIA
* TMS9918A VDP
* Dual AY-3-8910 PSGs
* Dual NES controller inputs
* PS/2 keyboard input
* 96KB Banked RAM/ROM

Making use of my various emulation libraries (and more):

* [vrEmu6502](https://github.com/visrealm/vrEmu6502) - 6502/65C02 CPU emulation library (C99)
* [vrEmu6522](https://github.com/visrealm/vrEmu6522) - 6522/65C22 VIA emulation library (C99)
* [vrEmuTms9918](https://github.com/visrealm/vrEmuTms9918) - TMS9918A/TMS9929A VDP emulation library (C99)

Follow along on YouTube here: [youtube.com/@TroySchrapel](https://youtube.com/@TroySchrapel)

## Development environment

To set up your development environment for the Raspberry Pi Pico, follow the [Raspberry Pi C/C++ SDK Setup](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html) instructions.

#### Windows

For Windows users, there is a pre-packaged installer provided by the Raspberry Pi Foundation: https://github.com/raspberrypi/pico-setup-windows/releases/. Once installed, just open the pre-configured "Pico - Visual Studio Code" from your start menu.

The build system expects `python3` to be available. If you have installed Python 3 outside of the Microsoft Store, you may need to alias your Python executable.

You can do this from an elevated (Administator) command prompt in your python directory e.g. `C:\Program Files\Python310\` by creating a symlink with the command: `mklink python3.exe python.exe`.

#### Linux and Mac

To build the project, run the following commands from the project root:

```bash
git clone --recursive https://github.com/visrealm/pico-56.git
cd pico-56 && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DPICO_SDK_FETCH_FROM_GIT=ON ..
make -j4
```

This will download the correct version of the Pi Pico SDK for your system.

## Episodes
In the Episodes, I build the code from the ground up and provide a number of working demos. These are a work in progress with more to come as the videos come out.
#### [Episode 1 - VGA](episodes/ep01-vga)
In this episode, I build a VGA circuit on the Raspberry Pi Pico and write an intitial generic VGA output library and a number of VGA test programs from a test pattern through to moving sprites and an 800x600 framebuffer.

[<img src="img/boing-sm.gif" height="150px"></img>](episodes/ep01-vga/ep01-vga-04-boing)  [<img src="img/nyan-sm.gif" height="150px"></img>](episodes/ep01-vga/ep01-vga-05-nyancat)  [<img src="img/framebuff-sm.gif" height="150px"></img>](episodes/ep01-vga/ep01-vga-06-framebuffer)  [<img src="img/slideshow-sm.gif" height="150px"></img>](episodes/ep01-vga/ep01-vga-09-slideshow)

#### [Episode 2 - TMS9918A](episodes/ep02-tms)
In this episode, I incorporate my TMS9918 library and create some test programs to test the TMS9918 functionality on the Pi Pico.

## Complete kits

Complete PICO-56 kits are now available on Tindie:

<a href="https://www.tindie.com/stores/visrealm/?ref=offsite_badges&utm_source=sellers_visrealm&utm_medium=badges&utm_campaign=badge_large"><img src="https://cdn.tindiemedia.com/images/resize/aYF_NMOzpFobut8nNDlEHyrd5dg=/p/full-fit-in/1782x1336/i/214054/products/2024-02-08T05%3A21%3A57.401Z-kit-components.png?1707341274" height="480px"></img></a>

<a href="https://www.tindie.com/stores/visrealm/?ref=offsite_badges&utm_source=sellers_visrealm&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

Also featured in HackerBox 0103 for those in the US:

<a href="https://hackerboxes.com/collections/past-hackerboxes/products/hackerbox-0103-homebrew"><img src="https://hackerboxes.com/cdn/shop/files/HB0103WholeBox_1024x1024@2x.png" height="480px"></img></a>

## Gerbers

Gerbers for the PICO-56 v1.4 are now available. See [/schematics](schematics)

If you would like to support this project, you can [order your PCBs from my PCBWay project link](https://www.pcbway.com/project/shareproject/PICO_56_Retro_Computer_on_a_Pi_Pico_515c59b8.html)

## Schematics

<p align="left"><a href="schematics"><img src="schematics/Schematic_Pico56_v1_4.png" alt="PICO-56 v1.4" width="720px"></a></p>

## Bill of materials

| Qty.     | Description                       | Code                              |
|----------|-----------------------------------|-----------------------------------|
| 1        | PICO-56 PCB                       | PCB                               |
| 5        | 100nf (104) ceramic capacitor     | C3,C4,C5,C6,C7                    |
| 3        | 10uf (106) electrolytic capacitor | C8,C1,C2                          |
| 3        | 4kΩ 0.25w resistor (3.9kΩ ok)     | R1,R5,R9                          |
| 2        | 10kΩ 0.25w resistor               | R20,R21                           |
| 6        | 2kΩ 0.25w resistor                | R22,R2,R6,R10,R17,R18             |
| 9        | 1kΩ 0.25w resistor                | R3,R7,R11,R13,R14,R15,R16,R19,R23 |
| 3        | 500Ω 0.25w resistor (510Ω ok)     | R4,R8,R12                         |
| 1        | 2N4401 transistor                 | Q1                                |
| 1        | 1N5819 diode                      | D1                                |
| 1        | Blue LED                          | LED1                              |
| 1        | Green LED                         | LED2                              |
| 1        | Push button                       | KEY1                              |
| 1        | Push button (locking)             | PWR1                              |
| 1        | Barrel jack connector             | 5V                                |
| 1        | VGA connector                     | DSUB1                             |
| 2        | RCA connector                     | J2,J3                             |
| 2        | NES connector                     | NO1,NO2                           |
| 1        | PS/2 connector                    | PS2                               |
| 1        | Raspberry Pi Pico (plus headers)  | U1                                |
| 1        | MicroSD card connector            | U2                                |

There is a more detailed BOM in the [/schematics](schematics) folder. Alternatively, I have created a [Mouser project](https://www.mouser.com/ProjectManager/ProjectDetail.aspx?AccessID=8431486f76) for this which contains everything except for the NES connectors which can be obtained from AliExpress.

## 3D Printed case

A 3D printed case is available. All STLs are in the [/case](case) directory.

<p align="left"><a href="case"><img src="case/img/pico56-case4.jpg" alt="PICO-56 case" width="720px"></a></p>

## Videos
[![PICO-56 - Introduction](https://img.visualrealmsoftware.com/youtube/thumb/Nj_KkYn7YaA)](https://www.youtube.com/watch?v=Nj_KkYn7YaA)

[![PICO-56 - Full Kit Build](https://img.visualrealmsoftware.com/youtube/thumb/1hwMNQ1DXIU)](https://www.youtube.com/watch?v=1hwMNQ1DXIU)

### Thanks

Thanks to [PCBWay](https://pcbway.com/g/186WQ9) for supporting this project.

<p align="left"><a href="https://pcbway.com/g/186WQ9"><img src="img/pcb02.jpg" alt="PICO-56" width="720px"></a></p>

## Resources

* [Windows Development Environment Installer](https://github.com/raspberrypi/pico-setup-windows/releases)
* [Raspberry Pi Pico C/C++ SDK (PDF)](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf)
* [HBC-56 Project](https://github.com/visrealm/hbc-56)
* [vrEmu6502 - 6502 CPU Emulation Library](https://github.com/visrealm/vrEmu6502)
* [vrEmu6522 - 6522 VIA Emulation Library](https://github.com/visrealm/vrEmu6522)
* [vrEmuTms9918 - TMS9918A VDP Emulation Library](https://github.com/visrealm/vrEmuTms9918)

## License
This code is licensed under the [MIT](https://opensource.org/licenses/MIT "MIT") license
