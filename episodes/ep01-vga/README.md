# Episode 1 - VGA

In this episode, I build a VGA circuit on the Pico and write an intitial generic VGA output library and a number of VGA test programs from a test pattern through to moving sprites and an 800x600 framebuffer.

## Schematic

For this episode, we start the build by adding a VGA connector and a reset button to the Pico. 

![VGA on Breadboard](ep01-vga-breadboard.png)

Full schematic here: [ep01-vga-schematic.pdf](ep01-vga-schematic.pdf)

## Example programs

| **[00 - VGA Sync](ep01-vga-00-sync)<br><br>Introduction to using the Pi Pico's Programmable Input/Output (PIO) to generate a configurable VGA sync signal.<br>**  | **[01 - VGA RGB](ep01-vga-01-rgb)<br><br>![rgb example video](/img/rgb-sm.gif)<br>Adding a second PIO state machine to handle the VGA's RGB signals. Fill the VGA display with a solid color.** | **[03 - Sine Wave](ep01-vga-03-sine)<br><br>![sine example video](/img/sine-sm.gif)<br>Generate an animated sine wave on the display.** |
|:--:|:--:|:--:|
| **[04 - Boing](ep01-vga-04-boing)<br><br>![boing example video](/img/boing-sm.gif)<br>The classic Commodore Amiga Boing demo. Showing animated sprites.** | **[05 - Nyan Cat](ep01-vga-05-nyancat)<br><br>![nyan example video](/img/nyan-sm.gif)<br>It's a meme. The Nyan Cat.** | **[06 - Frame Buffer](ep01-vga-06-framebuffer)<br><br>![framebuffer example video](/img/framebuff-sm.gif)<br>Here, we use a 320x240x12bpp framebuffer to output graphics.** |
| **[07 - Frame Buffer (640 x 480)](ep01-vga-07-framebuffer-640)<br><br>![framebuffer 640x480 example video](/img/framebuff-640-sm.gif)<br>We can't fit a 640x480x12bpp framebuffer, but we can do 4bpp using a palette.** | **[08 - Frame Buffer (800 x 600)](ep01-vga-08-framebuffer-800)<br><br>![framebuffer 800x600 example video](/img/framebuff-800-sm.gif)<br>Can we push it to 800 x 600?** | **[09 - Slideshow](ep01-vga-09-slideshow)<br><br>![slideshow example video](/img/slideshow-sm.gif)<br>A picture show** |

## VGA library

This VGA code is much more capable that is required for the future emulation of the TMS9918A and could be used stand-alone. This is the interface:

```c++
typedef void (*vgaScanlineFn)(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH]);

typedef struct
{
  vgaScanlineFn scanlineFn;
} VgaInitParams;


void vgaInit(VgaInitParams params);
```

To use the library, you just need to implement a `vgaScanlineFn()` function which could be as simple as copying out data from a framebuffer:

```c++
/* my framebuffer */
uint16_t __aligned(4) frameBuffer[VGA_VIRTUAL_WIDTH * VGA_VIRTUAL_HEIGHT];

/* my scanline function - copy from my framebuffer */
void frameBufferScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  memcpy(pixels, frameBuffer + y * VGA_VIRTUAL_WIDTH, VGA_VIRTUAL_WIDTH * sizeof(uint16_t));
}
```

Example `main()` function to initialize the VGA using the above framebuffer implementation:

```c++
int main(void)
{
  /* set up my VGA */
  VgaInitParams params;
  params.scanlineFn = frameBufferScanline;
  vgaInit(params);
  
  /* TODO: write to my framebuffer here */

  while (1)
  {
    /* TODO: or even in here */
	
    tight_loop_contents();
  }

  return 0;
}
```
