/*
 * test_spi.cpp
 *
 *  Created on: Oct 7, 2021
 *      Author: vitya
 */

#include "string.h"

#include "platform.h"
#include "hwspi.h"
#include "hwqspi.h"
#include "hwdma.h"
#include "hwpins.h"
#include "clockcnt.h"
#include "traces.h"
#include "spiflash.h"

#include "board_pins.h"

#if defined(MCU_VRV100)
  #define TEST_START_ADDR  0x100000  // start at 1M, bitstream is about 512k
#else
  #define TEST_START_ADDR  0x000000
#endif

TSpiFlash spiflash;

uint8_t spi_id[4];

unsigned readlen = 256;

uint8_t  databuf[8192];

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

void test_spiflash_reliability()
{
  TRACE("Reliability test begin\r\n");

  unsigned value;
  const unsigned begin_value = 0xc35a7291 + 1;
  const unsigned value_add = 33;
  uint32_t * dp;
  uint32_t * endp = (uint32_t *)&databuf[sizeof(databuf)];

  const unsigned test_length = sizeof(databuf);

#if 1

  TRACE("Erasing the first %u k...\r\n", test_length / 1024);
  spiflash.StartEraseMem(TEST_START_ADDR, test_length);
  spiflash.WaitForComplete();
  TRACE("Erase complete.\r\n");


  // fill the buffer:
  dp = (uint32_t *)&databuf[0];
  value = begin_value;
  while (dp < endp)
  {
    *dp++ = value;
    value += value_add;
  }

  TRACE("Writing blocks...\r\n");
  spiflash.StartWriteMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
  spiflash.WaitForComplete();

#endif

  TRACE("Reading back...\r\n");
  memset(&databuf[0], 0, sizeof(databuf));
  spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
  spiflash.WaitForComplete();

  TRACE("Comparing...\r\n");

  unsigned mismatch_cnt = 0;

  value = begin_value;
  dp = (uint32_t *)&databuf[0];
  while (dp < endp)
  {
    if (*dp++ != value)
    {
      ++mismatch_cnt;
    }
    value += value_add;
  }

  if (mismatch_cnt)
  {
    TRACE("ERROR! Mismatch count: %u\r\n", mismatch_cnt);
  }
  else
  {
    TRACE("Content check ok.\r\n");
  }

  TRACE("Reliability Test Finished.\r\n");
}

void test_simple_rw()
{
	int i;

	TRACE("Testing simple Read, Erase, Write\r\n");

	TRACE("Reading memory...\r\n");

	spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
	spiflash.WaitForComplete();

	TRACE("Memory read finished\r\n");

	show_mem(&databuf[0], readlen);

	TRACE("Erase sector...\r\n");
	spiflash.StartEraseMem(TEST_START_ADDR, sizeof(databuf));
	spiflash.WaitForComplete();
	TRACE("Erase complete.\r\n");

	TRACE("Writing memory...\r\n");

	for (i = 0; i < sizeof(databuf); ++i)
	{
		databuf[i] = uint8_t(0xF0 + i);
	}

	spiflash.StartWriteMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
	spiflash.WaitForComplete();

	TRACE("Reading memory...\r\n");

	spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
	spiflash.WaitForComplete();

	TRACE("Memory read finished\r\n");

  show_mem(&databuf[0], readlen);

	unsigned errorcnt = 0;
  for (i = 0; i < sizeof(databuf); ++i)
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

void test_spiflash()
{
	TRACE("SPI test begin\r\n");

  unsigned spispeed = 0;
  unsigned lanes = 1;
  const char * driver = "SPI";

  if (fl_qspi.initialized)
  {
    spiflash.qspi = &fl_qspi;
    spispeed = fl_qspi.speed;
    lanes = fl_qspi.multi_line_count;
    driver = "QSPI";
  }
  else
  {
    spiflash.spi = &fl_spi;
    spispeed = fl_spi.speed;
  }

  TRACE("driver = %s, speed = %u, lanes = %u\r\n", driver, spispeed, lanes);

#ifdef MCUF_VRV100
  spiflash.has4kerase = false;  // some FPGA config chips does not support 4k erase
#else
  spiflash.has4kerase = true;
#endif
  if (!spiflash.Init())
  {
    TRACE("SPI Flash init failed!\r\n");
    return;
  }

  TRACE("SPI Flash initialized, ID CODE = %06X, kbyte size = %u\r\n", spiflash.idcode, (spiflash.bytesize >> 10));
  TRACE("Test flash address: 0x%08X\r\n", TEST_START_ADDR);
  TRACE("Test buffer size: %u\r\n", sizeof(databuf));

	test_simple_rw();
	test_spiflash_reliability();

	TRACE("SPI test end\r\n");
}



