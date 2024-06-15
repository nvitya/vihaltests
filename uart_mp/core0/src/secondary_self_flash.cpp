/*
 * secondary_self_flash.cpp
 *
 *  Created on: Jun 15, 2024
 *      Author: vitya
 */

#include "secondary_self_flash.h"
#include "multicore_app.h"
#include "app_header.h"
#include "vgboot_utils.h"
#include "spiflash_updater.h"
#include "board_pins.h"
#include "hwmulticore.h"
#include "traces.h"

bool check_secondary_self_flash() // called regularly from idle loop
{
  TAppHeader * papph = (TAppHeader *)(SECONDARY_CODE_ADDR);
  if ((papph->signature != APP_HEADER_SIGNATURE) || (papph->customdata != SECONDARY_SELF_FLASH_FLAG))
  {
    return false;
  }

  // check the header checksum

  uint32_t csum;

  csum = vgboot_checksum(papph, sizeof(*papph) - 4);  // skip the checksum from the calculation
  if (papph->csum_head != csum)
  {
    //TRACE("Secondary Self Flash: invalid header checksum!\r\n");
    papph->customdata = 0;
    return false;
  }

  TRACE("Secondary Self Flash: valid request detected, code size = %u\r\n", papph->length);
  papph->customdata = 0; // clear the self-flash request
  papph->csum_head = vgboot_checksum(papph, sizeof(*papph) - 4);  // recalculate the header checksum for proper storage

  uint8_t   localbuf[256] __attribute__((aligned(8)));
  TSpiFlashUpdater  flashupdater(&spiflash, localbuf, sizeof(localbuf));
  uint32_t  totallen = papph->length + sizeof(TAppHeader);

  TRACE("Self-Flashing Secondary Core Code:\r\n");
  TRACE("  mem = %08X -> flash = %08X, len = %u ...\r\n", unsigned(SECONDARY_CODE_ADDR), SECONDARY_CODE_SPI_ADDR, totallen);

  unsigned  t0, t1;
  t0 = CLOCKCNT;
  if (!flashupdater.UpdateFlash(SECONDARY_CODE_SPI_ADDR, (uint8_t *)(SECONDARY_CODE_ADDR), totallen))
  {
    TRACE("  ERROR!\r\n");
    return true;
  }
  t1 = CLOCKCNT;
  unsigned clocksperus = SystemCoreClock / 1000000;
  unsigned ssize_k = flashupdater.sectorsize >> 10;
  TRACE("  %u * %uk updated, %u * %uk matched, took %u us\r\n",
      flashupdater.writecnt, ssize_k, flashupdater.samecnt, ssize_k, (t1 - t0) / clocksperus);

  return true;
}

bool load_secondary_core_code()
{

  // warning: the secondary core might be running
  multicore.ResetCore(1);

  TAppHeader * papph = (TAppHeader *)(SECONDARY_CODE_ADDR);

  spiflash.StartReadMem(SECONDARY_CODE_SPI_ADDR, papph, sizeof(*papph));
  spiflash.WaitForComplete();

  if (papph->signature != APP_HEADER_SIGNATURE)
  {
    return false;
  }

  uint32_t csum;
  csum = vgboot_checksum(papph, sizeof(*papph) - 4);  // skip the checksum from the calculation
  if (papph->csum_head != csum)
  {
    return false;
  }

  // load the whole to the destination (including the application header)
  //TRACE("APP LEN=%u\r\n", apphead.length);
  TRACE("Loading secondary core code (%u bytes) ...\r\n", papph->length);
  spiflash.StartReadMem(SECONDARY_CODE_SPI_ADDR + sizeof(*papph), papph + 1, papph->length);
  spiflash.WaitForComplete();

  csum = vgboot_checksum(papph + 1, papph->length);  // skip the checksum from the calculation
  if (papph->csum_body != csum)
  {
    TRACE("  Body Checksum error!\r\n");
    return false;
  }

  TRACE("  OK.\r\n");

  TRACE("Trying to start secondary at %08X...\r\n", papph->addr_entry);

  if (multicore.StartCore(1, papph->addr_entry, SECONDARY_STACK_ADDR, SECONDARY_CODE_ADDR))
  {
    TRACE("  OK.\r\n");
  }
  else
  {
    TRACE("  ERROR starting secondary core !\r\n");
    return false;
  }

  return true;
}
