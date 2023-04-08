/*
 * test_dma.cpp
 *
 *  Created on: Apr 7, 2023
 *      Author: vitya
 */

#include "hwspi.h"
#include "hwdma.h"
#include "test_spi.h"

#include "clockcnt.h"

#include "traces.h"

THwDmaChannel   txdma;
THwDmaChannel   rxdma;

THwDmaTransfer  rxfer;
THwDmaTransfer  txfer;

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

static uint8_t rxbuf[1024];
static uint8_t txbuf[1024];

void test_dma()
{
  int i;
  TRACE("Testing DMA\r\n");

  for (i = 0; i < 1024; ++i)
  {
    txbuf[i] = 0x80 + i;
    rxbuf[i] = 0x55;
  }

  txdma.Init(1, 0x00);
  txdma.Prepare(true, nullptr, 0);
  rxdma.Init(1, 0x00);
  txdma.Prepare(false, nullptr, 0);

  txfer.bytewidth = 1;
  txfer.count = 256;
  txfer.srcaddr = &txbuf[0];
  txfer.dstaddr = &rxbuf[0];
  txfer.flags = DMATR_MEM_TO_MEM;

  txdma.PrepareTransfer(&txfer);

  TRACE("Waiting for the DMA transfer complete...\r\n");
  delay_ms(100);

  show_mem(&rxbuf[0], txfer.count);

  TRACE("DMA Test finished.\r\n");
}



