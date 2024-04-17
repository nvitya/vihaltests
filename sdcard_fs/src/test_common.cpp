/*
 * test_common.cpp
 *
 *  Created on: Apr 17, 2024
 *      Author: vitya
 */

#include "test_common.h"
#include "hwsdram.h"
#include "board_pins.h"
#include "hwuscounter.h"
#include "traces.h"

uint32_t g_sdram_allocated = 0;
uint8_t * sdram_alloc(unsigned asize)
{
  if (!hwsdram.initialized)
  {
    return nullptr;
  }

  unsigned available = hwsdram.byte_size - g_sdram_allocated;
  if (asize > available)
  {
    TRACE("FATAL ERROR: SDRAM allocation failed\r\n");
    TRACE("  requested: %u\r\n", asize);
    TRACE("  available: %u\r\n", available);
    while (1)
    {
      __NOP();
    }
    return nullptr;
  }

  uint8_t * result = (uint8_t *)(hwsdram.address + g_sdram_allocated);
  g_sdram_allocated += asize;
  return result;
}

uint32_t sdram_free_bytes()
{
  return hwsdram.byte_size - g_sdram_allocated;
}

void display_bm_res(uint32_t ausecs, uint32_t abytesize)
{
  float ul_rw_speed = ((float)abytesize * 1000.0) / ausecs;
  TRACE("  Speed: %8.0f kB/s\n\r", ul_rw_speed);
}

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

bool sdcard_init_wait(unsigned ausecs)
{
  #ifdef SDCARD_SDMMC

    TRACE("SDCARD SDMMC test begin\r\n");

    if (!sdcard.Init(&sd_mmc))
    {
      TRACE("SDCARD init error !\r\n");
      return false;
    }
  #else
    TRACE("SDCARD SPI test begin\r\n");

    if (!sdcard.Init(&sd_spi))
    {
      TRACE("SDCARD init error !\r\n");
      return false;
    }
  #endif

  //sdcard.clockspeed = 25000000;
  //sdcard.clockspeed = 50000000;
  //sdcard.clockspeed = 800000000;
  //sdcard.forced_clockspeed = 25000000;
  //sdcard.bus_width = 4;

  TRACE("Waiting for SDCARD initialization...\r\n");

  uint32_t t0 = micros();
  while (!sdcard.card_initialized)
  {
    if (micros() - t0 > ausecs)
    {
      TRACE("Timeout waiting for SDCARD init!\r\n");
      return false;
    }
    sdcard.Run();
  }

  TRACE("SDCARD initialized, size = %u MB\r\n", sdcard.card_megabytes);
  return true;
}
