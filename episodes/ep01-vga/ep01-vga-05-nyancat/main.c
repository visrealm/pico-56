/*
 * Project: pico-56 - episode 1
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "vga.h"
#include "images.h"

#include "pico/stdlib.h"

#include <memory.h>

typedef struct
{
  int xPos;
  int yPos;
  int frame;
  int animDir;
} NyanStar;

#define NYAN_WIDTH 128
#define NYAN_HEIGHT 80

#define STAR_WIDTH 21
#define STAR_HEIGHT 21
#define STAR_SPEED -21

#define RAINBOW_WIDTH 34
#define RAINBOW_HEIGHT 60

#define NYAN_FRAME_RPT 4
#define RAINBOW_FRAME_RPT 12
#define STAR_FRAME_RPT 3

#define RAINBOW_POS_Y 80
#define RAINBOW_POS_X -9
#define RAINBOW_OFFSET_Y 3
#define RAINBOW_RPT 4

#define NYAN_POS_X  90
#define NYAN_POS_Y  71

void nyanScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  static int frame = 0;
  const uint16_t backColor = 0x0741;
  static NyanStar stars[] = { {140, -6, 0, 1},{250, 26, 1, 1},{310, 66, 2, 1},{0, 135, 3, 1},{265, 185, 3, -1},{100, 219, 4, 1} };
  static int nyanFrame = 0;

  const uint16_t* nyanFrames[] = { nyan00,nyan01,nyan02,nyan03,nyan04,nyan05 };
  const uint16_t* starFrames[] = { nyanstar00,nyanstar01,nyanstar02,nyanstar03,nyanstar04,nyanstar05 };

  const int numNyanFrames = sizeof(nyanFrames) / sizeof(uint16_t*);
  const int numStarFrames = sizeof(starFrames) / sizeof(uint16_t*);
  const int numStars = sizeof(stars) / sizeof(NyanStar);

  // output the background
  for (int x = 0; x < VGA_VIRTUAL_WIDTH; ++x)
  {
    pixels[x] = backColor;
  }

  // output the rainbow
  if (y >= RAINBOW_POS_Y && y <= (RAINBOW_POS_Y + RAINBOW_HEIGHT + RAINBOW_OFFSET_Y))
  {
    int rainbowFrame = (frame / RAINBOW_FRAME_RPT) & 1;

    int destX = 0;
    for (int i = 0; i < RAINBOW_RPT; ++i)
    {
      int xFrame = i & 1;
      int srcY = y - RAINBOW_POS_Y;
      if (xFrame == rainbowFrame) srcY -= RAINBOW_OFFSET_Y;

      int srcWidth = RAINBOW_WIDTH;
      if (i == 0) srcWidth += RAINBOW_POS_X;
      int srcOffset = ((srcY + 1) * RAINBOW_WIDTH) - srcWidth;

      if (srcY >= 0 && srcY < RAINBOW_HEIGHT)
      {
        memcpy(pixels + destX, nyanrainbow + srcOffset, srcWidth * sizeof(uint16_t));
      }
      destX += srcWidth;
    }
  }

  // output the stars
  for (int i = 0; i < numStars; ++i)
  {
    if (y >= stars[i].yPos && y < (stars[i].yPos + STAR_HEIGHT))
    {
      int startPixel = (y - stars[i].yPos) * STAR_WIDTH;
      for (int x = 0; x < STAR_WIDTH; ++x)
      {
        int pixelX = stars[i].xPos + x;
        if (pixelX < 0 || pixelX >= VGA_VIRTUAL_WIDTH) continue;
        uint16_t c = starFrames[stars[i].frame][startPixel + x];
        if (c & 0xf000)
        {
          pixels[pixelX] = c;
        }
      }
    }
  }

  // output the cat
  if (y >= NYAN_POS_Y && y < (NYAN_POS_Y + NYAN_HEIGHT))
  {
    int srcOffset = (y - NYAN_POS_Y) * NYAN_WIDTH;
    for (int x = 0; x < NYAN_WIDTH; ++x)
    {
      uint16_t c = nyanFrames[nyanFrame][srcOffset + x];
      if (c & 0xf000)
      {
        pixels[x + NYAN_POS_X] = c;
      }
    }

  }

  // end of frame updates
  if (y == VGA_VIRTUAL_HEIGHT - 1)
  {
    ++frame;

    if ((frame % STAR_FRAME_RPT) == 0)
    {
      for (int i = 0; i < numStars; ++i)
      {
        stars[i].xPos += STAR_SPEED;
        if (stars[i].xPos < -STAR_WIDTH)
        {
          stars[i].xPos += VGA_VIRTUAL_WIDTH;
        }
        stars[i].frame += stars[i].animDir;
        if (stars[i].frame < 0) stars[i].frame = numStarFrames - 1;
        else if (stars[i].frame >= numStarFrames) stars[i].frame = 0;
      }
    }

    if ((frame % NYAN_FRAME_RPT) == 0)
    {
      ++nyanFrame;
      nyanFrame %= numNyanFrames;
    }
  }
}
int main(void)
{
  set_sys_clock_khz(252000, false);

  VgaInitParams params;
  params.scanlineFn = nyanScanline;

  vgaInit(params);

  while (1)
  {
    tight_loop_contents();
  }

  return 0;
}
