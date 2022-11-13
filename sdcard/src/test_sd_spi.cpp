/*
 * test_spi.cpp
 *
 *  Created on: Oct 7, 2021
 *      Author: vitya
 */

#include "string.h"

#include "platform.h"
#include "hwspi.h"
#include "hwdma.h"
#include "hwpins.h"
#include "clockcnt.h"
#include "traces.h"

#include "board_pins.h"
#include "sdcard_spi.h"

TSdCardSpi  sdcard;

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

uint8_t databuf[4096];

void test_sd_spi()
{
  int i;

	TRACE("SDCARD SPI test begin\r\n");

	if (!sdcard.Init(&sd_spi))
	{
	  TRACE("SDCARD init error !\r\n");
	  return;
	}

	TRACE("Runing SDCARD state machine...\r\n");
	while (!sdcard.card_initialized)
	{
	  sdcard.Run();
	}

	// trace take time, for better scoping no traces here until the end

#if 1

	sdcard.StartReadBlocks(0,  &databuf[0],  4);
	sdcard.WaitForComplete();

	show_mem(&databuf[0], 512 * 4);

#else

	for (i = 0; i < 512*4; ++i)
	{
	  databuf[i] = (i >> 6);
	}

  // it is relative safe to write to the second sector
  sdcard.StartWriteBlocks(1,  &databuf[0],  4);
  sdcard.WaitForComplete();

#endif

  TRACE("SDCARD initialized, size = %u MB\r\n", sdcard.card_megabytes);


	TRACE("SDCARD SPI test end\r\n");
}



