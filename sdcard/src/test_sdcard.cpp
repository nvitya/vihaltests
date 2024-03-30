
#include "string.h"

#include "board_pins.h"
#include "clockcnt.h"
#include "traces.h"

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

#define DATABUF_SIZE   (2 * 16 * 1024)

uint8_t databuf[DATABUF_SIZE]   __attribute__((aligned(4)));
uint8_t databuf2[DATABUF_SIZE]  __attribute__((aligned(4)));

void display_bm_res(uint32_t aclocks, uint32_t abytesize)
{
  float ul_ticks = (float)aclocks / (SystemCoreClock / 1000);
  float ul_rw_speed = (float)abytesize / ul_ticks;
  TRACE("  Speed: %8.0f K/s\n\r", ul_rw_speed);
}

void sdcard_read_single_blocks()
{
  unsigned block_count = 16 * 1024;  // 8 MByte
  unsigned bytesize = block_count * 512;
  unsigned block_addr  = 0;
  unsigned error_count = 0;

  TRACE("Reading %u single blocks...\r\n", block_count);

  unsigned t0 = CLOCKCNT;

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

  unsigned t1 = CLOCKCNT;

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}

void sdcard_read_multiple_blocks()
{
  unsigned error_count = 0;
  unsigned multi_blocks = sizeof(databuf) / 512;
  unsigned repeat_count = 16 * 1024 * 1024 / (multi_blocks * 512);
  unsigned bytesize = repeat_count * multi_blocks * 512;
  unsigned block_addr  = 0;

  //block_addr += 1;  // +1 makes it unaligned, but for reads it does not matter

  TRACE("Reading %u * %u blocks...\r\n", repeat_count, multi_blocks);

  unsigned t0 = CLOCKCNT;

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

  unsigned t1 = CLOCKCNT;

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

  unsigned t0 = CLOCKCNT;

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

  unsigned t1 = CLOCKCNT;

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}

void sdcard_write_multiple_blocks()
{
  unsigned error_count = 0;
  unsigned multi_blocks = sizeof(databuf) / 512;
  unsigned repeat_count = 16 * 1024 * 1024 / (multi_blocks * 512);
  unsigned bytesize = repeat_count * multi_blocks * 512;
  unsigned block_addr  = multi_blocks;

  block_addr += 16; // +1 makes it unaligned, it does matter

  TRACE("Writing %u * %u blocks...\r\n", repeat_count, multi_blocks);

  unsigned t0 = CLOCKCNT;

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

  unsigned t1 = CLOCKCNT;

  TRACE("  Finished, errors = %u\r\n", error_count);
  display_bm_res(t1 - t0, bytesize);
}

void sdcard_verify_multiple_blocks()
{
  unsigned multi_blocks = 8; // 4k puffer
  unsigned repeat_count = 2 * 1024;
  unsigned bytesize = repeat_count * multi_blocks * 512;
  unsigned block_addr  = multi_blocks;
  unsigned error_count = 0;
  unsigned mismatch_count = 0;

  TRACE("Veryfying %u * %u blocks...\r\n", repeat_count, multi_blocks);

  unsigned t0 = CLOCKCNT;

  while (repeat_count > 0)
  {
    sdcard.StartReadBlocks(block_addr,  &databuf2[0], multi_blocks);
    sdcard.WaitForComplete();
    if (sdcard.errorcode)
    {
      ++error_count;
    }
    else
    {
      if (memcmp(&databuf[0], &databuf2[0], sizeof(databuf)) != 0)
      {
        ++mismatch_count;
      }
    }
    block_addr += multi_blocks;
    --repeat_count;
  }

  unsigned t1 = CLOCKCNT;

  TRACE("  Finished, read errors = %u\r\n", error_count);
  if (mismatch_count)
  {
    TRACE("  MISMATCH COUNT = %u\r\n", mismatch_count);
  }
  else
  {
    TRACE("  verify ok.\r\n");
  }
  display_bm_res(t1 - t0, bytesize);
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
  //sdcard.forced_clockspeed = 50000000;
  //sdcard.bus_width = 4;

  TRACE("Waiting for SDCARD initialization...\r\n");
  while (!sdcard.card_initialized)
  {
    sdcard.Run();
  }

  TRACE("SDCARD initialized, size = %u MB\r\n", sdcard.card_megabytes);
}

void test_sdcard()
{
  int i;

  sdcard_init();

  #if 1
    TRACE("Read tests...\r\n");
    //sdcard_read_single_blocks();
    sdcard_read_multiple_blocks();
  #endif

  #if 1
    TRACE("Write tests...\r\n");

    for (i = 0; i < sizeof(databuf); ++i)
    {
      databuf[i] = 0xA0 + i;
    }

    //sdcard_write_single_blocks();
    sdcard_write_multiple_blocks();

    #if 0
      TRACE("Verify written data...\r\n");
      sdcard_verify_multiple_blocks();
    #endif

  #endif


	TRACE("SDCard test finished.\r\n");
}



