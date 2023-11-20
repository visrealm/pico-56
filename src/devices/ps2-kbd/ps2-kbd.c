/*
 * Project: pico-56 - PS/2 keyboard
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "pico/time.h"

#include "ps2-kbd.h"
#include "ps2-kbd.pio.h"
#include "interrupts.h"


uint8_t ascii2Ps2[128] = {
    0,    // NUL
    0,    // SOH
    0,    // STX
    0,//0x03,    // ETX
    0,    // EOT
    0,    // ENQ
    0,    // ACK
    0,    // BEL
    0x66,    // BS
    0x0d,    // TAB
    0x5a,    // LF
    0,    // VT
    0,    // FF
    0x5a,    // CR
    0,    // SO
    0,    // SI
    0,    // DLE
    0,    // DC1
    0,    // DC2
    0,    // DC3
    0,    // DC4
    0,    // NAK
    0,    // SYN
    0,    // ETB
    0,    // CAN
    0,    // EM
    0,    // SUB
    0,    // ESC
    0,    // FS
    0,    // GS
    0,    // RS
    0,    // US
    0x29,    // SPACE
    0x16,  // !
    0x52,    // "
    0x26,    // #
    0x25,    // $
    0x2e,    // %
    0x3d,    // &
    0x52,    // '
    0x46,    // (
    0x45,    // )
    0x3e,    // *
    0x55,    // +
     0x41,    // ,
     0x7b,    // -
     0x49,    // .
     0x4a,    // /
    0x45,  // 0
    0x16,  // 1
    0x1e,  // 2
    0x26,  // 3
    0x25,  // 4
    0x2e,  // 5
    0x36,  // 6
    0x3d,  // 7
    0x3e,  // 8
    0x46,  // 9
    0x4c,    // :
    0x4c,    // ;
     0x41,    // <
     0x55,    // =
     0x49,    // >
     0x4a,    // ?
     0x1e,    // @
    0x1c,  // A
    0x32,  // B
    0x21,  // C
    0x23,  // D
    0x24,  // E
    0x2b,  // F
    0x34,  // G
    0x33,  // H
    0x43,  // I
    0x3b,  // J
    0x42,  // K
    0x4b,  // L
    0x3a,  // M
    0x31,  // N
    0x44,  // O
    0x4d,  // P
    0x15,  // Q
    0x2d,  // R
    0x1b,  // S
    0x2c,  // T
    0x3c,  // U
    0x2a,  // V
    0x1d,  // W
    0x22,  // X
    0x35,  // Y
    0x1a,  // Z
    0x54,    // [
    0x5d,    // backslash
    0x5b,    // ]
    0x36,    // ^
    0x7b,    // _
    0x0e,    // `
    0x1c,  // A
    0x32,  // B
    0x21,  // C
    0x23,  // D
    0x24,  // E
    0x2b,  // F
    0x34,  // G
    0x33,  // H
    0x43,  // I
    0x3b,  // J
    0x42,  // K
    0x4b,  // L
    0x3a,  // M
    0x31,  // N
    0x44,  // O
    0x4d,  // P
    0x15,  // Q
    0x2d,  // R
    0x1b,  // S
    0x2c,  // T
    0x3c,  // U
    0x2a,  // V
    0x1d,  // W
    0x22,  // X
    0x35,  // Y
    0x1a,  // Z
    0x54,    // [
    0x5d,    // backslash
    0x5b,    // ]
    0x0e,    // ~
};


uint8_t ascii2Ps2[128];
bool isShifted = false;
bool isControlled = false;


#define   KB_QUEUE_SIZE 256
#define   KB_QUEUE_MASK (KB_QUEUE_SIZE - 1)
char      kbQueue[KB_QUEUE_SIZE];
int       kbStart = 0;
int       kbEnd = 0;

extern bool kbdQueueEmpty()
{
  return kbEnd == kbStart;
}

extern void kbdQueuePush(uint8_t scancode)
{
  kbQueue[kbEnd++] = scancode; kbEnd &= KB_QUEUE_MASK;
  raiseInterrupt(KBD_INT);
}

extern uint8_t kbdQueuePop()
{
  uint8_t val = kbQueue[kbStart++];
  kbStart &= KB_QUEUE_MASK;
  if (kbdQueueEmpty())
    releaseInterrupt(KBD_INT);
  return val;
}


char processAsciiToPs2(char c)
{
  if (c >= 128)
    return 0;

  bool shift = false;
  if (isupper(c))
  {
    shift = true;
  }
  else
  {
    switch (c)
    {
      case '!': shift = true; break;
      case '\"': shift = true; break;
      case '#': shift = true; break;
      case '$': shift = true; break;
      case '%': shift = true; break;
      case '&': shift = true; break;
      case '(': shift = true; break;
      case ')': shift = true; break;
      case '*': shift = true; break;
      case '+': shift = true; break;
      case ':': shift = true; break;
      case '<': shift = true; break;
      case '>': shift = true; break;
      case '?': shift = true; break;
      case '^': shift = true; break;
      case '_': shift = true; break;
      case '{': shift = true; break;
      case '|': shift = true; break;
      case '}': shift = true; break;
      case '~': shift = true; break;
    }
  }

  char sc = ascii2Ps2[c];
  if (sc)
  {
    if (shift)
    {
      kbdQueuePush(0x12);
    }

    kbdQueuePush(sc);
    kbdQueuePush(0xf0);
    kbdQueuePush(sc);

    isShifted = shift;

    raiseInterrupt(KBD_INT);
  }
  else
  {
    if (c == 0x03) // Ctrl+C
    {
      kbdQueuePush(0x14);
      kbdQueuePush(ascii2Ps2['c']);
      isControlled = true;
      raiseInterrupt(KBD_INT);

      printf("Ctrl+C\n", c);
    }
    else
    {
      printf("%02x not mapped\n", c);
    }
  }
  return sc;
}
