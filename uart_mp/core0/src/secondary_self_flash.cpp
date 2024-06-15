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
#include "rp_mailbox.h"
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

// this part is copied from the PICO-SDK (https://github.com/raspberrypi/pico-sdk)

#define PSM_FRCE_OFF_OFFSET _u(0x00000004)
#define PSM_FRCE_OFF_PROC1_BITS   0x00010000

void multicore_reset_core1()
{
  // Use atomic aliases just in case core 1 is also manipulating some PSM state
  io_rw_32 * power_off     = (io_rw_32 *) (PSM_BASE + PSM_FRCE_OFF_OFFSET);
  io_rw_32 * power_off_set = hw_set_alias(power_off);
  io_rw_32 * power_off_clr = hw_clear_alias(power_off);

  // Hard-reset core 1.
  // Reading back confirms the core 1 reset is in the correct state, but also
  // forces APB IO bridges to fence on any internal store buffering
  *power_off_set = PSM_FRCE_OFF_PROC1_BITS;
  while (!(*power_off & PSM_FRCE_OFF_PROC1_BITS))
  {
    __NOP();
  };

  // Bring core 1 back out of reset. It will drain its own mailbox FIFO, then push
  // a 0 to our mailbox to tell us it has done this.
  *power_off_clr = PSM_FRCE_OFF_PROC1_BITS;
}

TRpMailBox rp_mailbox;

bool load_secondary_core_code()
{
  rp_mailbox.Init(0);

  // warning: the secondary core might be running
  multicore_reset_core1();

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

  rp_mailbox.StartSecodaryCore(papph->addr_entry, SECONDARY_STACK_ADDR, 0x21040000);

  //    __isr_vectors void (*[])(void)  0x21020400
  //SECONDARY_CODE_ADDR + sizeof(TAppHeader));

  TRACE("Secondary started.\r\n");

  return true;
}
