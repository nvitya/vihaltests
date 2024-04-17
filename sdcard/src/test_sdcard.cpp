
#include "string.h"

#include "stdlib.h"
#include "board_pins.h"
#include "test_sdcard.h"
#include "clockcnt.h"
#include "hwuscounter.h"
#include "traces.h"

#include "test_common.h"

static uint32_t databuf_size = 0;

static uint8_t * databuf;
static uint8_t * databuf2;

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

  if (!sdcard_init_wait(3000000))
  {
    return;  // error message is already printed.
  }

  //TRACE("SD card test return....\r\n");
  //return;

  #if 1
    TRACE("Read tests...\r\n");
    //sdcard_read_single_blocks();
    sdcard_read_multiple_blocks();
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



