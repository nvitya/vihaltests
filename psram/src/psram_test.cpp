/*
 * psram_test.cpp
 *
 *  Created on: Mar 23, 2022
 *      Author: vitya
 */

#include "board_pins.h"
#include "spipsram.h"
#include "traces.h"

#define TEST_START_ADDR  0x000000

TSpiPsram  psram;
TPsramTra  tra;

uint8_t  databuf[8 * 1024] __attribute__((aligned(8)));

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

void psram_test_simple()
{
  unsigned i;

  unsigned testsize = sizeof(databuf);
  //unsigned testsize = 256;
  unsigned showlen = 256;
  if (showlen > testsize)  showlen = testsize;

#if 1
  for (i = 0; i < testsize; ++i)
  {
    databuf[i] = uint8_t(0xF0 + i);
  }

  TRACE("Filling RAM...\r\n");

  psram.StartWriteMem(&tra, 0, &databuf[0], testsize);
  psram.WaitFinish(&tra);

#endif

  TRACE("Reading memory...\r\n");

  // set the memory contents to some invalid one
  for (i = 0; i < sizeof(databuf); ++i)
  {
    databuf[i] = uint8_t(0x55 + i);
  }

  psram.StartReadMem(&tra, 0, &databuf[0], testsize);
  psram.WaitFinish(&tra);

  TRACE("Memory read finished\r\n");

  show_mem(&databuf[0], showlen);

#if 0
  TRACE("Reading memory / 2 ...\r\n");

  // set the memory contents to some invalid one
  for (i = 0; i < sizeof(databuf); ++i)
  {
    databuf[i] = uint8_t(0x55 + i);
  }

  psram.StartReadMem(&tra, 0, &databuf[0], testsize);
  psram.WaitFinish(&tra);

  TRACE("Memory read finished\r\n");

  show_mem(&databuf[0], showlen);
#endif

  unsigned errorcnt = 0;
  for (i = 0; i < testsize; ++i)
  {
    if (databuf[i] != uint8_t(0xF0 + i))
    {
      ++errorcnt;
    }
  }

  if (errorcnt)
  {
    TRACE("ERROR COUNT: %u\r\n", errorcnt);
  }
  else
  {
    TRACE("content ok.\r\n");
  }

  TRACE("Test finished.\r\n");
  TRACE("\r\n");
}

void psram_test_init()
{
  TRACE("SPI PSRAM test begin\r\n");

  unsigned spispeed = 0;
  unsigned lanes = 1;
  const char * driver = "SPI";

  if (fl_qspi.initialized)
  {
    psram.qspi = &fl_qspi;
    spispeed = fl_qspi.speed;
    lanes = fl_qspi.multi_line_count;
    driver = "QSPI";
  }
  else
  {
    psram.spi = &fl_spi;
    spispeed = fl_spi.speed;
  }

  TRACE("driver = %s, speed = %u, lanes = %u\r\n", driver, spispeed, lanes);

  //test_dma();
  //test_dma();
  //return;

  if (!psram.Init())
  {
    TRACE("SPI PSRAM init failed!\r\n");
    return;
  }

  TRACE("PSRAM ID = %08X:%08X, kbyte size = %u\r\n", psram.idcode, psram.idcode2, (psram.bytesize >> 10));
  TRACE("  max chunk size: %u\r\n", psram.maxchunksize);
  TRACE("Test flash address: 0x%08X\r\n", TEST_START_ADDR);
  TRACE("Test buffer size: %u\r\n", sizeof(databuf));

  psram_test_simple();

  //test_spiflash_reliability();

  TRACE("SPI PSRAM Test end\r\n");
}

void psram_test_run()
{

}

