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

This is the placeholder repository for the up-coming video series covering the development of the PICO-56.

Follow along on YouTube here: [youtube.com/@TroySchrapel](https://youtube.com/@TroySchrapel)

### Schematics

<p align="left"><a href="schematics"><img src="schematics/Schematic_Pico56_v1_2.png" alt="PICO-56 v1.2" width="720px"></a></p>

### Thanks

Thanks to [PCBWay](https://pcbway.com/g/186WQ9) for supporting this project.

<p align="left"><a href="https://pcbway.com/g/186WQ9"><img src="img/pcb02.jpg" alt="PICO-56" width="720px"></a></p>

## License
This code is licensed under the [MIT](https://opensource.org/licenses/MIT "MIT") license
