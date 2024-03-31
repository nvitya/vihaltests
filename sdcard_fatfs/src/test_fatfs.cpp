
#include "string.h"

#include "ff.h"

#include "board_pins.h"
#include "clockcnt.h"
#include "traces.h"

FATFS    g_fatfs;

#define DATABUF_SIZE   (2 * 16 * 1024)

uint8_t  databuf[DATABUF_SIZE]   __attribute__((aligned(4)));
uint8_t  databuf2[DATABUF_SIZE]  __attribute__((aligned(4)));


void display_bm_res(uint32_t aclocks, uint32_t abytesize)
{
  float ul_ticks = (float)aclocks / (SystemCoreClock / 1000);
  float ul_rw_speed = (float)abytesize / ul_ticks;
  TRACE("  Speed: %8.0f K/s\n\r", ul_rw_speed);
}

void test_file_write()
{
  int i;
  FRESULT  fres;
  FIL   fil;
  UINT  rlen;

  //Try to open file
  fres = f_open(&fil, "1stfile.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  if (fres != FR_OK)
  {
    TRACE("Error opening file to write!\r\n");
  }
  else
  {
    //If we put more than 0 characters (everything OK)

    for (i = 0; i < sizeof(databuf); ++i)
    {
      databuf[i] = 0xB0 + i;
    }

    fres = f_write(&fil, databuf, sizeof(databuf), &rlen);
    if (fres != FR_OK)
    {
      TRACE("File write ERROR: %i\r\n", fres);
    }
    else
    {
      TRACE("File write ok.\r\n");
    }

    f_close(&fil);
  }
}

void test_fatfs()
{
  TRACE("FatFS test init.\r\n");

  FRESULT  fres;
  FIL fil;

  fres = f_mount(&g_fatfs, "", 1);  // last param: 1 = wait for mount
  if (fres != FR_OK)
  {
    TRACE("FatFS mount error: %i\r\n", fres);
    return;
  }

  TRACE("FatFS mount ok.\r\n");

  test_file_write();

  TRACE("Unmounting FatFs...\r\n");
  f_mount(0, "", 1);

  TRACE("FatFS test finished.\r\n");
}



