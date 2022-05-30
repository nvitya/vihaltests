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

#include "app_header.h"

#define SELFFLASH_BUFSIZE  256

extern unsigned __bl_stage2_src;
extern unsigned __bl_stage2_len;

extern unsigned __app_image_end;

extern "C" void cold_entry();

/* this CRC algorithm was taken from here:
 *   https://stackoverflow.com/questions/21001659/crc32-algorithm-implementation-in-c-without-a-look-up-table-and-with-a-public-li
 *
 * A little reformatted, and adjusted for the RP2040 needs:
 *   polynom:        0x04c11db7
 *   initial value:  0xffffffff
 *
*/

uint32_t calc_stage2_crc(uint8_t * data, unsigned len)
{
   int i, j;
   uint32_t byte, crc, mask;

   crc = 0xFFFFFFFF;
   i = 0;
   while (i < len)
   {
      byte = data[i];               // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--)      // Do eight times.
      {
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0x04C11DB7 & mask);
      }
      ++i;
   }
   return ~crc;
}

// do self flashing using the flash writer
bool spi_self_flashing(TSpiFlash * spiflash)
{
  uint8_t   localbuf[SELFFLASH_BUFSIZE] __attribute__((aligned(8)));

  // Using the flash writer to first compare the flash contents:
  TSpiFlashUpdater  flashupdater(spiflash, localbuf, sizeof(localbuf));
  TAppHeader        apph;

  apph.signature = APP_HEADER_SIGNATURE;
  apph.copy_addr = 0x21000000;
  apph.copy_length = ((3 + unsigned(&__app_image_end) - unsigned(0x21000000)) & 0x0FFFFFFC);
  apph.entry_point = unsigned(cold_entry);

  TRACE("Self-Flashing Main Code:\r\n");
  TRACE("  mem = %08X -> flash = %08X, len = %u ...\r\n", unsigned(0x21000000), 0x002000, apph.copy_length);

  unsigned  t0, t1;
  t0 = CLOCKCNT;
  if (!flashupdater.UpdateFlash(0x002000, (uint8_t *)(0x21000000), apph.copy_length))
  {
    TRACE("  ERROR!\r\n");
    return false;
  }
  t1 = CLOCKCNT;
  unsigned clocksperus = SystemCoreClock / 1000000;
  unsigned ssize_k = flashupdater.sectorsize >> 10;
  TRACE("  %u * %uk updated, %u * %uk matched, took %u us\r\n",
      flashupdater.writecnt, ssize_k, flashupdater.samecnt, ssize_k, (t1 - t0) / clocksperus);


  TRACE("Application header...\r\n");
  if (!flashupdater.UpdateFlash(0x001000, (uint8_t *)&apph, sizeof(apph)))
  {
    TRACE("  ERROR!\r\n");
    return false;
  }
  if (flashupdater.writecnt)  { TRACE("  updated\r\n"); }  else { TRACE("  matched\r\n"); }

  uint8_t *   st2_source = (uint8_t *)&__bl_stage2_src;
  uint32_t *  st2_crc = (uint32_t *)(st2_source + 252);

  //TRACE("Sencond stage addr=%08X, len=%u\r\n", , &__bl_stage2_len);

  // now we are changing the image inserting the CRC at the last position !
  *st2_crc = calc_stage2_crc(st2_source, 252);

  // to debug it can be copied to the original place
  //memcpy((void *)0x20041000, st2_source, 256);

  TRACE("Stage2 Loader...\r\n");
  if (!flashupdater.UpdateFlash(0x000000, st2_source, 256))
  {
    TRACE("  ERROR!\r\n");
    return false;
  }
  if (flashupdater.writecnt)  { TRACE("  updated\r\n"); }  else { TRACE("  matched\r\n"); }

	return true;
}

