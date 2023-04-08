/*
 * test_spi.cpp
 *
 *  Created on: Apr 3, 2023
 *      Author: vitya
 */

#include "hwspi.h"
#include "hwdma.h"
#include "test_spi.h"

#include "traces.h"

THwSpi          spi;
THwDmaChannel   spi_txdma;
THwDmaChannel   spi_rxdma;

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

void spi_init()
{
  hwpinctrl.PadSetup(PAD_GPIO0, FSPICS0_OUT_IDX, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_GPIO1, FSPICLK_OUT_IDX, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_GPIO2, FSPID_OUT_IDX,   PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_GPIO3, FSPIQ_IN_IDX,    PINCFG_INPUT);
  spi.Init(2);

  spi_txdma.Init(0, GDMA_PERI_SEL_SPI2);
  spi_rxdma.Init(0, GDMA_PERI_SEL_SPI2);

  spi.DmaAssign(true,  &spi_txdma);
  spi.DmaAssign(false, &spi_rxdma);
}


void test_spi()
{
  int i;
  TRACE("Testing SPI\r\n");

  spi_init();

  for (i = 0; i < 1024; ++i)
  {
    txbuf[i] = 0x80 + i;
    rxbuf[i] = 0x55;
  }

  unsigned len = 256;

  spi.StartTransfer(0xAA, 0x12345678,  SPITR_CMD1 | SPITR_ADDR3, len, &txbuf[0], &rxbuf[0]);
  spi.WaitFinish();

  show_mem(&rxbuf[0], len);

  TRACE("SPI Test finished.\r\n");
}
