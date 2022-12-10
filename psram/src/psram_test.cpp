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
  //unsigned testsize = 2 * 256;
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
    psram.single_line_read = true;
    TRACE("Warning: QSPI single line read!\r\n");
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

unsigned g_addr = 0;
unsigned g_mode = 0;
unsigned g_phase = 0;
unsigned g_chunksize = sizeof(databuf);
unsigned g_errcnt = 0;
unsigned g_testsize = 1024 * 64;
unsigned g_starttime = 0;
uint32_t g_startvalue = 0x12345678;
uint32_t g_value = 0;
uint32_t g_waitcnt = 0;

void psram_test_run()
{
  if (!psram.initialized)
  {
    return;
  }

  unsigned i;

  if (0 == g_phase)  // filling the memory
  {
    if (0 == g_addr)
    {
      TRACE("Filling...");

      #if 1
        // overriding the calculated max chunk size
        // it seems that the device can handle (in single line mode) any access
        // and it is not sensible to the TCEM (maximal chip enable time) violation

        //#warning "TCEM violation is active !"
        psram.maxchunksize = psram.pagesize;  // this is the maximum (internally limited)
      #endif

      g_testsize = psram.bytesize;
      g_startvalue += 101;
      g_value = g_startvalue;
      g_errcnt = 0;
      g_starttime = CLOCKCNT;
    }

    if (g_addr < g_testsize)
    {
      register uint32_t * pdst = (uint32_t *)&databuf[0];
      register uint32_t * pend = pdst + (g_chunksize >> 2);
      register uint32_t v = g_value;
      while (pdst < pend)
      {
        *pdst++ = v;
        v = v + 0x01010101;
      }
      g_value = v;

      psram.StartWriteMem(&tra, g_addr, &databuf[0], g_chunksize);
      g_phase = 1;
    }
    else
    {
      unsigned clkcnt = CLOCKCNT - g_starttime;
      TRACE("%u us\r\n", clkcnt / (SystemCoreClock / 1000000));
      g_addr = 0;
      g_waitcnt = 0;

      #if 1
        g_phase = 10;  // read back
      #else
        g_phase = 5;   // refresh deny
      #endif
    }
  }
  else if (1 == g_phase) // wait chunk fill complete
  {
    psram.Run();
    if (!tra.completed)
    {
      return;
    }

    g_addr += g_chunksize;
    g_phase = 0;
  }

  else if (5 == g_phase)  // repeating a single transaction which should lead to refresh error
  {
    if (0 == g_waitcnt)
    {
      TRACE("Denying refresh...\r\n");
    }

    psram.StartReadMem(&tra, 0, &databuf[0], 1024);
    g_phase = 6;
  }
  else if (6 == g_phase)
  {
    psram.Run();
    if (!tra.completed)
    {
      return;
    }

    ++g_waitcnt;
    if (g_waitcnt > 16 * 1024)
    {
      g_phase = 10;  // go to read
    }
    else
    {
      g_phase = 5;
    }
  }

  else if (10 == g_phase)  // reading back
  {
    if (0 == g_addr)
    {
      TRACE("Reading...");
      g_starttime = CLOCKCNT;
      g_value = g_startvalue;
    }

    if (g_addr < g_testsize)
    {
      psram.StartReadMem(&tra, g_addr, &databuf[0], g_chunksize);
      g_phase = 11;
    }
    else
    {
      unsigned clkcnt = CLOCKCNT - g_starttime;
      TRACE("%u us\r\n", clkcnt / (SystemCoreClock / 1000000));

      if (g_errcnt)
      {
        TRACE("  errors: %u\r\n", g_errcnt);
      }
      else
      {
        TRACE("  no errors.\r\n");
      }

      g_addr = 0;
      g_phase = 0;
    }
  }
  else if (11 == g_phase)  // wait chunk read complete
  {
    psram.Run();
    if (!tra.completed)
    {
      return;
    }

    // compare
    register uint32_t * pdst = (uint32_t *)&databuf[0];
    register uint32_t * pend = pdst + (g_chunksize >> 2);
    register uint32_t v = g_value;
    while (pdst < pend)
    {
      if (*pdst++ != v)
      {
        ++g_errcnt;
      }
      v = v + 0x01010101;
    }
    g_value = v;

    g_addr += g_chunksize;
    g_phase = 10;
  }

}

