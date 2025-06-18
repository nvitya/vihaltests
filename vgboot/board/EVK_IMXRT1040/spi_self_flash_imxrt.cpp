/*
 * spi_self_flash_imxrt.cpp
 *
 */

#include "stdlib.h"
#include "platform.h"
#include "spiflash_updater.h"
#include "spi_self_flashing.h"
#include "traces.h"

extern unsigned __image_end;

#define SELFFLASH_BUFSIZE  4096
#define NVSADDR_FW_BOOTLOADER  0x00000

// do self flashing using the flash writer
bool spi_self_flashing(TSpiFlash * spiflash)
{
  uint8_t * tempbuf = (uint8_t *)malloc(SELFFLASH_BUFSIZE);  // allocate the temp buffer on the heap !!!

  uint32_t  flash_addr = NVSADDR_FW_BOOTLOADER;
  uint8_t * start_addr = (uint8_t *)BOOTLOADER_LOAD_ADDR;

  unsigned copylength = unsigned(&__image_end) - unsigned(start_addr);
  copylength = ((copylength + 255) & 0xFFF00);

  __DSB();

  // Using the flash writer to first compare the flash contents:
  TSpiFlashUpdater  flashupdater(spiflash, tempbuf, SELFFLASH_BUFSIZE);

  TRACE("Self-Flashing:\r\n");
  TRACE("  mem = %08X -> flash = %08X, len = %u ...\r\n", unsigned(start_addr), flash_addr, copylength);

  unsigned  t0, t1;
  t0 = CLOCKCNT;

  if (!flashupdater.UpdateFlash(flash_addr, start_addr, copylength))
  {
    TRACE("  ERROR!\r\n");
    return false;
  }

  t1 = CLOCKCNT;
  unsigned clocksperus = SystemCoreClock / 1000000;

  unsigned ssize_k = flashupdater.sectorsize >> 10;

  TRACE("  %u * %uk updated, %u * %uk matched, took %u us\r\n",
      flashupdater.writecnt, ssize_k, flashupdater.samecnt, ssize_k, (t1 - t0) / clocksperus);

  free(tempbuf);
  return true;
}
