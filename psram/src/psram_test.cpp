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

uint8_t spi_id[4];

unsigned readlen = 256;

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

#if 0

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
  TRACE("Erase complete. Erase check...\r\n");
  memset(&databuf[0], 0x55, sizeof(databuf));
  spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
  spiflash.WaitForComplete();

  unsigned mismatch_cnt = 0;

  dp = (uint32_t *)&databuf[0];
  while (dp < endp)
  {
    if (*dp++ != 0xFFFFFFFF)
    {
      ++mismatch_cnt;
    }
  }

  if (mismatch_cnt)
  {
    TRACE("Erase ERROR! Mismatch count: %u\r\n", mismatch_cnt);
  }
  else
  {
    TRACE("Erase ok.\r\n");
  }

  TRACE("Writing blocks...\r\n");
  // fill the buffer:
  dp = (uint32_t *)&databuf[0];
  value = begin_value;
  while (dp < endp)
  {
    *dp++ = value;
    value += value_add;
  }
  spiflash.StartWriteMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
  spiflash.WaitForComplete();

#endif

  TRACE("Reading back...\r\n");
  memset(&databuf[0], 0, sizeof(databuf));
  spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
  spiflash.WaitForComplete();

  TRACE("Comparing...\r\n");

  mismatch_cnt = 0;
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

#if 1
  TRACE("Writing memory...\r\n");

  for (i = 0; i < sizeof(databuf); ++i)
  {
    databuf[i] = uint8_t(0xF0 + i);
  }

  spiflash.StartWriteMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
  spiflash.WaitForComplete();
#endif

  TRACE("Reading memory...\r\n");

  // set the memory contents to some invalid one
  for (i = 0; i < sizeof(databuf); ++i)
  {
    databuf[i] = uint8_t(0x55 + i);
  }

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

#if 0

uint8_t tdmabuf[8];
uint8_t rdmabuf[8];

THwDmaTransfer txfer;
THwDmaTransfer rxfer;

void test_dma()
{
  TRACE("DMA Test\r\n");

  if (!fl_spi.txdma || !fl_spi.rxdma)
  {
    TRACE("DMA channels are not assigned !\r\n");
    return;
  }

  tdmabuf[0] = 0x9F;
  tdmabuf[1] = 0x55;
  tdmabuf[2] = 0x56;
  tdmabuf[3] = 0x57;
  tdmabuf[4] = 0x58;

  rdmabuf[0] = 0x55;
  rdmabuf[1] = 0x56;
  rdmabuf[2] = 0x57;
  rdmabuf[3] = 0x58;
  rdmabuf[4] = 0x59;

  fl_spi.manualcspin->Set0();

  rxfer.dstaddr = &rdmabuf[0];
  rxfer.bytewidth = 1;
  rxfer.count = 5;
  rxfer.flags = 0;

  txfer.srcaddr = &tdmabuf[0];
  txfer.bytewidth = 1;
  txfer.count = 5;
  txfer.flags = 0;

  fl_spi.DmaStartRecv(&rxfer);
  fl_spi.DmaStartSend(&txfer);

  while (fl_spi.rxdma->Active() || fl_spi.txdma->Active())
  {
    // wait for the DMAs
  }

  delay_us(10);

  fl_spi.manualcspin->Set1();

  unsigned n;
  TRACE("DMA test result:\r\n");
  for (n = 0; n < 5; ++n)
  {
    TRACE(" %02X", rdmabuf[n]);
  }
  TRACE("\r\n");
}

#endif

#endif

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

  TRACE("SPI PSRAM initialized, ID CODE = %06X, kbyte size = %u\r\n", psram.idcode, (psram.bytesize >> 10));
  TRACE("Test flash address: 0x%08X\r\n", TEST_START_ADDR);
  TRACE("Test buffer size: %u\r\n", sizeof(databuf));

  //test_simple_rw();
  //test_spiflash_reliability();

  TRACE("SPI PSRAM Test end\r\n");
}

void psram_test_run()
{

}

