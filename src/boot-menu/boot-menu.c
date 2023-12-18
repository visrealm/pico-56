/*
 * Project: pico-56 - boot menu
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */


#include "tms9918.h"
#include "sdcard.h"

#include "vrEmuTms9918Util.h"

#include "input.h"

#include "pico/stdlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const uint8_t tmsFont[];
extern const size_t tmsFontBytes;

extern uint8_t* romPtr();
extern size_t romSize();

#define FILE_PATTERN "*.o"
#define PAGE_SIZE 16

static int fileCount = 0;

/*
 * Load a page of file metadata from the sdcard
 */
bool loadPage(uint16_t pageNumber, FILINFO fileList[PAGE_SIZE])
{
  DIR d;
  FILINFO fno;
  memset(&d, 0, sizeof d);
  memset(&fno, 0, sizeof fno);
  memset(fileList, 0, sizeof(FILINFO) * PAGE_SIZE);


  int index = 0;
  FRESULT fr = f_findfirst(&d, &fno, ".", FILE_PATTERN);

  int fromIndex = pageNumber * PAGE_SIZE;
  int toIndex = fromIndex + PAGE_SIZE;
  bool status = false;

  while ((FR_OK == fr) && fno.fname[0])
  {
    if (index >= fromIndex && (index < toIndex))
    {
      fileList[index - fromIndex] = fno;
      status = true;
    }

    index++;
    fr = f_findnext(&d, &fno);
  }

  fileCount = index;

  return status;
}

/*
 * Render the current page of file metadata to the tms9918
 */
void renderPage(FILINFO fileList[PAGE_SIZE], int currentIndex, int currentPage)
{
  VrEmuTms9918* tms9918 = getTms9918();

  char fileName[32];
  sprintf(fileName, "Pg %d/%d", currentPage + 1, fileCount / PAGE_SIZE + 1);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + (32 * 3) + 24);
  vrEmuTms9918WriteString(tms9918, fileName);

  for (int i = 0; i < PAGE_SIZE; ++i)
  {
    char c = i + '0';
    if (i > 9) c += 7;

    memset(fileName, ' ', sizeof(fileName));
    fileName[30] = '\0';
    if (fileList[i].fname[0])
    {
      sprintf(fileName, "%c%-18.18s %7d B ", (currentIndex == i) ? '>' : ' ', fileList[i].fname, fileList[i].fsize);
    }

    vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + (32 * 5) + (32 * i) + 1);
    vrEmuTms9918WriteStringOffset(tms9918, fileName, (currentIndex == i) ? 128 : 0);
  }
}

/*
 * Render the boot menu (except for the main list)
 */
void renderBootMenu()
{
  VrEmuTms9918* tms9918 = getTms9918();

  vrEmuTms9918InitialiseGfxI(tms9918);
  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_COLOR_ADDRESS);
  vrEmuTms9918SetFgBgColor(tms9918, TMS_WHITE, TMS_DK_BLUE);
  vrEmuTms9918WriteByteRpt(tms9918, vrEmuTms9918FgBgColor(TMS_DK_BLUE, TMS_WHITE), 16);
  vrEmuTms9918WriteByteRpt(tms9918, vrEmuTms9918FgBgColor(TMS_WHITE, TMS_DK_BLUE), 16);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_PATT_ADDRESS);
  vrEmuTms9918WriteBytes(tms9918, tmsFont, tmsFontBytes);
  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_PATT_ADDRESS + 128 * 8);
  vrEmuTms9918WriteBytes(tms9918, tmsFont, tmsFontBytes);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 + 1);
  for (int i = 20; i < 27; ++i)
    vrEmuTms9918WriteData(tms9918, i);
  vrEmuTms9918WriteString(tms9918, "    Boot Menu");

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS);
  vrEmuTms9918WriteByteRpt(tms9918, ' ' - 5, 32);
  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 23);
  vrEmuTms9918WriteByteRpt(tms9918, 128 + ' ' - 5, 32);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 2);
  vrEmuTms9918WriteByteRpt(tms9918, '~' + 1, 32);
  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 21);
  vrEmuTms9918WriteByteRpt(tms9918, '~' + 1, 32);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 4);
  vrEmuTms9918WriteData(tms9918, 32 - 4);
  vrEmuTms9918WriteByteRpt(tms9918, '~' + 1, 30);
  vrEmuTms9918WriteData(tms9918, 32 - 2);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 5);
  vrEmuTms9918WriteData(tms9918, '|');
  for (int i = 6; i < 16 + 6; ++i)
  {
    vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * i - 1);
    vrEmuTms9918WriteData(tms9918, '|');
    vrEmuTms9918WriteData(tms9918, '|');
  }
  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 21);
  vrEmuTms9918WriteData(tms9918, 32 - 3);
  vrEmuTms9918WriteByteRpt(tms9918, '~' + 1, 30);
  vrEmuTms9918WriteData(tms9918, 32 - 1);

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 22 + 3);
  vrEmuTms9918WriteString(tms9918, "github.com/visrealm/pico-56");
}

/*
 * Run the boot menu. Optionally update the ROM image
 */
void runBootMenu()
{
  renderBootMenu();

  int currentPage = 0;
  int currentIndex = 0;
  int renderedPage = -1;

  VrEmuTms9918* tms9918 = getTms9918();

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 3 + 1);
  vrEmuTms9918WriteString(tms9918, "Checking for MicroSD...");

  sd_card_t* pSD = sd_get_by_num(0);

  FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);

  TCHAR label[255] = "Not present";
  DWORD vsn = 0;
  char fileName[FF_MAX_LFN + 10];

  f_getlabel(pSD->pcName, label, &vsn);
  if (!label[0]) strcpy(label, "<no label>");

  vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 3 + 1);
  sprintf(fileName, "MicroSD: %-20.20s", label);
  vrEmuTms9918WriteString(tms9918, fileName);

  FILINFO* fileList = malloc(sizeof(FILINFO) * PAGE_SIZE);
  bool status = false;

  if (fr == FR_OK)
  {
    status = loadPage(currentPage, fileList);
  }

  if (!status)
  {
    free(fileList);
    sleep_ms(1000);
    vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 5 + 1);
    vrEmuTms9918WriteString(tms9918, "Starting BASIC...");
    sleep_ms(2000);
    return;
  }

  printf("PICO-56 boot menu\n");

  uint8_t lastScancode = 0;
  renderPage(fileList, currentIndex, currentPage);

  int uiUpdateIndex = 0;
  absolute_time_t nextUiUpdate = 0;

  while (status)
  {
    BootMenuInput inp = currentInput();

    if (inp == BMI_DOWN)
    {
      if (currentIndex < PAGE_SIZE - 1)
      {
        if (fileList[currentIndex + 1].fname[0])
        {
          ++currentIndex;
          renderPage(fileList, currentIndex, currentPage);
          sleep_ms(150);
        }
      }
      else
      {
        inp = BMI_PGDOWN;
      }
    }
    else if (inp == BMI_UP)
    {
      if (currentIndex > 0)
      {
        --currentIndex;
        renderPage(fileList, currentIndex, currentPage);
        sleep_ms(150);
      }
      else
      {
        inp = BMI_PGUP;
      }
    }

    if (inp == BMI_PGDOWN)
    {
      if (loadPage(currentPage + 1, fileList))
      {
        ++currentPage;
        currentIndex = 0;
      }
      else
      {
        loadPage(currentPage, fileList);
      }
      renderPage(fileList, currentIndex, currentPage);
      sleep_ms(150);
    }
    else if (inp == BMI_PGUP)
    {
      if (loadPage(currentPage - 1, fileList))
      {
        --currentPage;
        currentIndex = 0;
      }
      else
      {
        loadPage(currentPage, fileList);
      }
      renderPage(fileList, currentIndex, currentPage);
      sleep_ms(150);
    }
    else if (inp == BMI_SELECT)
    {
      break;
    }

    // update bottom message
    absolute_time_t currentTime = get_absolute_time();
    if (currentTime > nextUiUpdate)
    {
      nextUiUpdate = delayed_by_ms(currentTime, 5000);
      vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 22);
      if (++uiUpdateIndex & 0x01)
      {
        vrEmuTms9918WriteString(tms9918, "   github.com/visrealm/pico-56");
      }
      else
      {
        vrEmuTms9918WriteString(tms9918, "      \x13 2023 Troy Schrapel    ");
      }
    }
  }

  memset(fileList, 0, sizeof(FILINFO) * PAGE_SIZE);
  renderPage(fileList, -1, currentPage);
  loadPage(currentPage, fileList);

  if (status)
  {
    vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 5 + 1);
    vrEmuTms9918WriteString(tms9918, "Loading ");
    vrEmuTms9918WriteString(tms9918, fileList[currentIndex].fname);

    printf("Loading %s...\n", fileList[currentIndex].fname);

    FIL fil;
    fr = f_open(&fil, fileList[currentIndex].fname, FA_OPEN_EXISTING | FA_READ);
    if (fr == FR_OK || fr == FR_EXIST)
    {
      unsigned int nr;
      fr = f_read(&fil, (void*)romPtr(), romSize(), &nr);			/* Read data from the file */
    }

    vrEmuTms9918SetAddressWrite(tms9918, TMS_DEFAULT_VRAM_NAME_ADDRESS + 32 * 5 + 1);
    vrEmuTms9918WriteString(tms9918, (FR_OK == fr) ? "Loaded " : "Error loading ");
    vrEmuTms9918WriteString(tms9918, fileList[currentIndex].fname);
    vrEmuTms9918WriteString(tms9918, "    ");
    sleep_ms(1000);

  }
  free(fileList);
}
