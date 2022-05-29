/*
 * self_flashing.cpp
 *
 *  Created on: Oct 11, 2021
 *      Author: vitya
 */

#include "string.h"

#include "platform.h"

#include "hwspi.h"
#include "spiflash.h"
#include "spiflash_updater.h"
#include "clockcnt.h"
#include "traces.h"

#define SELFFLASH_BUFSIZE  256

extern unsigned __bl_stage2_src;
extern unsigned __bl_stage2_len;

extern unsigned __app_image_end;

// do self flashing using the flash writer
bool spi_self_flashing(TSpiFlash * spiflash)
{
  TRACE("Self Flashing...\r\n");
  TRACE("Sencond stage addr=%08X, len=%u\r\n", &__bl_stage2_src, &__bl_stage2_len);
  memcpy((void *)0x20041000, (const void *)&__bl_stage2_src, unsigned(&__bl_stage2_len));

#if 0
  uint8_t   localbuf[SELFFLASH_BUFSIZE] __attribute__((aligned(8)));
	unsigned  len = unsigned(&__app_image_end) - unsigned(&application_header);

  bootrom_info_t * bootrom_info = (bootrom_info_t *)BOOTROM_INFO_ADDRESS;

  if (bootrom_info->signature != BOOTROM_INFO_SIGNATURE)
  {
    TRACE("BootRom signature error: %08X = %08X\r\n", BOOTROM_INFO_ADDRESS, bootrom_info->signature);
    return false;
  }

	bootblock_header_t * phead = (bootblock_header_t *)&application_header;
	phead->compid = bootrom_info->compid;

	// Using the flash writer to first compare the flash contents:
	TSpiFlashUpdater  flashupdater(spiflash, localbuf, sizeof(localbuf));

  TRACE("Self-Flashing:\r\n");
  TRACE("  mem = %08X -> flash = %08X, len = %u ...\r\n", unsigned(&application_header), bootrom_info->bootblock_staddr, len);

  len = ((len + 7) & 0xFFFFFFF8); // length must be also dividible with 8 !

	unsigned  t0, t1;
	t0 = CLOCKCNT;

	if (!flashupdater.UpdateFlash(bootrom_info->bootblock_staddr, (uint8_t *)&application_header, len))
	{
    TRACE("  ERROR!\r\n");
    return false;
	}

  t1 = CLOCKCNT;
  unsigned clocksperus = SystemCoreClock / 1000000;

  unsigned ssize_k = flashupdater.sectorsize >> 10;

	TRACE("  %u * %uk updated, %u * %uk matched, took %u us\r\n",
	    flashupdater.writecnt, ssize_k, flashupdater.samecnt, ssize_k, (t1 - t0) / clocksperus);

#endif

	return true;
}

