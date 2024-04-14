
#include "string.h"

#include "stdlib.h"
#include "board_pins.h"
#include "test_sdcard.h"
#include "clockcnt.h"
#include "hwuscounter.h"
#include "traces.h"
#include "hwmemprot.h"

uint32_t g_sdram_allocated = 0;
uint32_t databuf_size = 0;

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


#if 0
static void show_mem(void * addr, unsigned len)
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
#endif

uint8_t * databuf;
uint8_t * databuf2;

void display_bm_res(uint32_t ausecs, uint32_t abytesize)
{
  float ul_rw_speed = ((float)abytesize * 1000.0) / ausecs;
  TRACE("  Speed: %8.0f kB/s\n\r", ul_rw_speed);
}

void sdcard_read_single_blocks()
{
  unsigned block_count = 16 * 1024;  // 8 MByte
  unsigned bytesize = block_count * 512;
  unsigned block_addr  = 0;
  unsigned error_count = 0;

  TRACE("Reading %u single blocks...\r\n", block_count);

  unsigned t0 = micros();

  while (block_count > 0)
  {
    sdcard.StartReadBlocks(block_addr,  &databuf[0], 1);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      ++error_count;
    }
    ++block_addr;
    --block_count;
  }

  unsigned t1 = micros();

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}

void sdcard_read_multiple_blocks()
{
  unsigned error_count = 0;
  unsigned multi_blocks = databuf_size / 512;
  unsigned repeat_count = 16 * 1024 * 1024 / (multi_blocks * 512);
  if (repeat_count < 1)  repeat_count = 1;
  unsigned bytesize = repeat_count * multi_blocks * 512;
  unsigned block_addr  = 0;

  TRACE("Reading %u * %u blocks...\r\n", repeat_count, multi_blocks);

  unsigned t0 = micros();

  while (repeat_count > 0)
  {
    sdcard.StartReadBlocks(block_addr,  &databuf[0], multi_blocks);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      ++error_count;
    }
    block_addr += multi_blocks;
    --repeat_count;
  }

  unsigned t1 = micros();

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}


void sdcard_write_single_blocks()
{
  unsigned block_count = 8 * 1024;  // 4 MByte
  unsigned bytesize = block_count * 512;
  unsigned block_addr  = 1;
  unsigned error_count = 0;

  TRACE("Writing %u single blocks...\r\n", block_count);

  unsigned t0 = micros();

  while (block_count > 0)
  {
    sdcard.StartWriteBlocks(block_addr,  &databuf[0], 1);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      ++error_count;
    }
    ++block_addr;
    --block_count;
  }

  unsigned t1 = micros();

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}

void sdcard_write_multiple_blocks()
{
  unsigned error_count = 0;
  unsigned multi_blocks = databuf_size / 512;
  unsigned repeat_count = 16 * 1024 * 1024 / (multi_blocks * 512);
  if (repeat_count < 1)  repeat_count = 1;
  unsigned bytesize = repeat_count * multi_blocks * 512;
  unsigned block_addr  = multi_blocks;

  TRACE("Writing %u * %u blocks...\r\n", repeat_count, multi_blocks);

  unsigned t0 = micros();

  while (repeat_count > 0)
  {
    sdcard.StartWriteBlocks(block_addr,  &databuf[0], multi_blocks);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      ++error_count;
    }
    block_addr += multi_blocks;
    --repeat_count;
  }

  unsigned t1 = micros();

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}

void sdcard_verify_multiple_blocks()
{
  unsigned multi_blocks = databuf_size / 512;
  unsigned repeat_count = 16 * 1024 * 1024 / (multi_blocks * 512);
  if (repeat_count < 1)  repeat_count = 1;
  unsigned bytesize = repeat_count * multi_blocks * 512;
  unsigned block_addr  = multi_blocks;
  unsigned error_count = 0;
  unsigned mismatch_count = 0;

  TRACE("Veryfying %u * %u blocks...\r\n", repeat_count, multi_blocks);

  unsigned read_us = 0;
  unsigned compare_us = 0;

  unsigned t0 = micros();
  unsigned t1;

  while (repeat_count > 0)
  {
    sdcard.StartReadBlocks(block_addr,  &databuf2[0], multi_blocks);
    sdcard.WaitForComplete();
    t1 = micros();
    read_us += t1 - t0;

    if (sdcard.errorcode)
    {
      ++error_count;
    }
    else
    {
      // this might take very long time using sdram without d-cache!
      t0 = micros();

      #if 0
        if (memcmp(&databuf[0], &databuf2[0], databuf_size) != 0)
        {
          ++mismatch_count;
        }
      #else
        uint32_t * psrc1 = (uint32_t *)&databuf[0];
        uint32_t * psrc2 = (uint32_t *)&databuf2[0];
        uint32_t * psrc1_end = psrc1 + (databuf_size >> 2);
        while (psrc1 < psrc1_end)
        {
          if (*psrc1++ != *psrc2++)
          {
            ++mismatch_count;
            break;
          }
        }
      #endif
      t1 = micros();
      compare_us += t1 - t0;
    }
    block_addr += multi_blocks;
    --repeat_count;
  }

  TRACE("  Finished, read errors = %u\r\n", error_count);
  if (mismatch_count)
  {
    TRACE("  MISMATCH COUNT = %u\r\n", mismatch_count);
  }
  else
  {
    TRACE("  verify ok.\r\n");
  }
  TRACE("  Reading");
  display_bm_res(read_us, bytesize);
  TRACE("  Compare");
  display_bm_res(compare_us, bytesize);
}

void sdcard_init()
{
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

  //sdcard.clockspeed = 25000000;
  //sdcard.clockspeed = 50000000;
  //sdcard.clockspeed = 800000000;
  //sdcard.forced_clockspeed = 25000000;
  //sdcard.bus_width = 4;

  TRACE("Waiting for SDCARD initialization...\r\n");
  while (!sdcard.card_initialized)
  {
    sdcard.Run();
  }

  TRACE("SDCARD initialized, size = %u MB\r\n", sdcard.card_megabytes);
}

void mpu_setup()
{
  TRACE("Enabling Caching for the SDRAM\r\n");

  //show_mpu();
  MPU->CTRL = 0; // disable the MPU

  MPU->RNR = 0;
  MPU->RBAR = hwsdram.address;

  uint32_t attr = 0
    | (1 << 0) // shareable
    | (1 << 1) // inner cache policy(2): 0 = non-cacheable, 1 = Write back, write and Read- Allocate, 2 = Write through, no Write-Allocate, 3 = Write back, no Write-Allocate
    | (1 << 3) // outer cache policy(2): 0 = non-cacheable, 1 = Write back, write and Read- Allocate, 2 = Write through, no Write-Allocate, 3 = Write back, no Write-Allocate
    | (1 << 5) // cached region
  ;

  MPU->RASR = 0
    | (1  <<  0) // ENABLE
    | (27 <<  1) // SIZE(5): region size = 2^(1 + SIZE),  2^28 = 256 MByte
    | (0  <<  8) // SRD(8): subregion disable bits
    | (attr << 16) // attributes(6), B, S, C, TEX
    | (3  << 24) // AP(3): permissions, 3 = RW/RW (full access)
    | (0  << 28) // XN: 1 = code execution disabled
  ;

  MPU->CTRL = (1 << 0) | (1 << 2); // enable MPU
}

void test_sdcard()
{
  int i;

  TRACE("SD Card Test Start\r\n");

  if (hwsdram.initialized)
  {
    databuf_size = hwsdram.byte_size / 2;
    databuf  = (uint8_t *)sdram_alloc(databuf_size);
    databuf2 = (uint8_t *)sdram_alloc(databuf_size);
  }
  else
  {
    databuf_size = 16 * 1024;
    databuf  = (uint8_t *)malloc(databuf_size);
    databuf2 = (uint8_t *)malloc(databuf_size);
  }

  TRACE("Test buffer size: %u\r\n", databuf_size);

  if (!databuf or !databuf2)
  {
    TRACE("Test buffer allocation failed!\r\n");
    return;
  }


  sdcard_init();

  #if 1

    //mpu_setup();
    //hwmemprot.Init();
    //hwmemprot.AddRegion(hwsdram.address, hwsdram.byte_size, MEMPROT_ATTR_MEM_NOCACHE, MEMPROT_PERM_RW);
    //hwmemprot.AddRegion(hwsdram.address, hwsdram.byte_size, MEMPROT_ATTR_MEM_CACHED, MEMPROT_PERM_RW);
    //hwmemprot.Enable();
  #endif

  //TRACE("SD card test return....\r\n");
  //return;

  #if 1
    TRACE("Read tests...\r\n");
    //sdcard_read_single_blocks();
    //sdcard_read_multiple_blocks();
  #endif

  #if 1
    TRACE("Write tests...\r\n");

    uint32_t * pu32 = (uint32_t *)databuf;
    for (i = 0; i < (databuf_size >> 2); ++i)
    {
      //*pu32++ = 0x55555555 + i;
      *pu32++ = 0xAAAAAAAA + i;
    }

    pu32 = (uint32_t *)databuf2;
    for (i = 0; i < (databuf_size >> 2); ++i)
    {
      *pu32++ = 0x11111111 + i;
    }

    //sdcard_write_single_blocks();
    sdcard_write_multiple_blocks();

    #if 1
      TRACE("Verify written data...\r\n");
      sdcard_verify_multiple_blocks();
    #endif

  #endif


	TRACE("SDCard test finished.\r\n");
}



