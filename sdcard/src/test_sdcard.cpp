
#include "string.h"

#include "board_pins.h"
#include "clockcnt.h"
#include "traces.h"

void show_mem(void * addr, unsigned len)
{
	unsigned char * cp = (unsigned char *)addr;
	TRACE("Dumping memory at %08X, len = %u\r\n", addr, len);
	for (unsigned n = 0; n < len; ++n)
	{
		TRACE(" %02X", *cp++);
		if (n % 16 == 15) TRACE("\r\n");
	}
	TRACE("\r\n");
}

#define RW_BLOCK_CNT  1

uint8_t databuf[4096]  __attribute__((aligned(4)));

void test_sdcard()
{
  int i;

  #ifdef SDCARD_SDMMC

    TRACE("SDCARD SDMMC test begin\r\n");

    if (!sdcard.Init(&sd_mmc))
    {
      TRACE("SDCARD init error !\r\n");
      return;
    }
  #else
    TRACE("SDCARD SPI test begin\r\n");

    if (!sdcard.Init(&sd_spi))
    {
      TRACE("SDCARD init error !\r\n");
      return;
    }
  #endif

	TRACE("Runing SDCARD state machine...\r\n");
	while (!sdcard.card_initialized)
	{
	  sdcard.Run();
	}

  TRACE("SDCARD initialized, size = %u MB\r\n", sdcard.card_megabytes);

  #if 1
    TRACE("Writing SDCARD...\r\n");

    for (i = 0; i < sizeof(databuf); ++i)
    {
      databuf[i] = 0xA0 + (i >> 6);
    }

    // it is relative safe to write to the second sector
    sdcard.StartWriteBlocks(1,  &databuf[0],  RW_BLOCK_CNT);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      TRACE("  Write error: %i\r\n", sdcard.errorcode);
    }
    else
    {
      TRACE("  OK\r\n");
    }
  #endif


  #if 1
    TRACE("Reading SDCARD...\r\n");

    sdcard.StartReadBlocks(1024*1024,  &databuf[0],  RW_BLOCK_CNT);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      TRACE("  Read error: %i\r\n", sdcard.errorcode);
    }
    else
    {
      TRACE("  OK\r\n");
    }

    show_mem(&databuf[0], 512 * RW_BLOCK_CNT);

    TRACE("Reading SDCARD...\r\n");

    sdcard.StartReadBlocks(1024*1024,  &databuf[0],  RW_BLOCK_CNT);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      TRACE("  Read error: %i\r\n", sdcard.errorcode);
    }
    else
    {
      TRACE("  OK\r\n");
    }

    show_mem(&databuf[0], 512 * RW_BLOCK_CNT);

  #endif


	TRACE("SDCARD test end\r\n");
}



