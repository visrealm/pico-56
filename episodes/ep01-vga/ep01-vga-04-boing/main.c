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
#include "hardware/clocks.h"

#include <memory.h>
#include <math.h>

void boingScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
#define SPEED_X 1
#define ACC_Y 0.25
#define BALL_FRAME_RATE 1
#define BALL_SIZE_X 96
#define BALL_SIZE_Y 90

  const uint16_t* ballSprites[] = { ball00,ball01,ball02,ball03,ball04,ball05,ball06,ball07,ball08,ball09,ball10,ball11,ball12,ball13,ball14,ball15,ball16,ball17,ball18,ball19 };

  static float frame = 1000;
  static const uint16_t* ball = ball01;
  static float xPos = 120, yPos = 5;
  static float xSpeed = SPEED_X;
  static float ySpeed = 0;
  static float frameAnim = -1;


  memcpy(pixels, background + y * VGA_VIRTUAL_WIDTH, VGA_VIRTUAL_WIDTH * sizeof(uint16_t));

  if (y > yPos && y < (yPos + BALL_SIZE_Y))
  {
    int bally = y - round(yPos);
    int xPosInt = round(xPos);
    for (int i = 0; i < BALL_SIZE_X; ++i)
    {
      uint16_t c = ball[bally * BALL_SIZE_X + i];
      uint8_t a = (c & 0xf000) >> 12;
      if (a == 0x00)
        continue;

      // need to alpha blend source pixel to dest
      if (a != 0x0f)
      {
        uint8_t sa = a;
        uint8_t da = 15 - a;

        uint16_t dc = pixels[xPosInt + i];

        uint16_t sb = (c & 0x0f00) >> 8;
        uint16_t sg = (c & 0x00f0) >> 4;
        uint16_t sr = (c & 0x000f);
        uint16_t db = (dc & 0x0f00) >> 8;
        uint16_t dg = (dc & 0x00f0) >> 4;
        uint16_t dr = (dc & 0x000f);

        sr = (sr * sa + dr * da) / 15;
        sg = (sg * sa + dg * da) / 15;
        sb = (sb * sa + db * da) / 15;

        c = ((sb & 0x0f) << 8) | ((sg & 0x0f) << 4) | ((sr & 0x0f) << 0);
      }

      pixels[xPosInt + i] = c;
    }
  }

  if (y == VGA_VIRTUAL_HEIGHT - 1)
  {
    frame += frameAnim;

    ySpeed += ACC_Y;
    yPos += ySpeed;
    if (yPos > 138)
    {
      ySpeed *= -1;
      yPos = 138;
    }

    xPos += xSpeed;
    if (xPos > 218 || xPos < 10)
    {
      xSpeed *= -1.0f;
      frameAnim *= -1.0f;
      xPos += xSpeed;
    }

    int ballIdx = ((int)frame / BALL_FRAME_RATE) % 20;
    ball = ballSprites[ballIdx];
  }
}

int main(void)
{
  set_sys_clock_khz(252000, false);

  VgaInitParams params;
  params.scanlineFn = boingScanline;

  vgaInit(params);

  while (1)
  {
    tight_loop_contents();
  }

  return 0;
}
