/*
 * test_spi.cpp
 *
 *  Created on: Oct 7, 2021
 *      Author: vitya
 */

#include "platform.h"
#include "hwspi.h"
#include "hwdma.h"
#include "hwpins.h"
#include "clockcnt.h"
#include "traces.h"
#include "serialflash.h"
#include "spiflash.h"

#define TEST_START_ADDR  0x100000  // start at 1M, bitstream is about 512k
#define USE_DMA  1

THwSpi     spi;
TSpiFlash  spiflash;

TGpioPin       spi_cs_pin;
THwDmaChannel  spi_txdma;
THwDmaChannel  spi_rxdma;

uint8_t spi_id[4];
uint8_t spi_buf[4096];

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

#if defined(MCUF_VRV100)

void test_spi_direct()
{
	/*
	DATA -> 0x00 :
	  rxData -> R[7:0]
	  rxOccupancy -> R[30:16] rx fifo occupancy (include the rxData in the amount)
	  rxValid -> R[31] Inform that the readed rxData is valid
	  When you read this register it pop an byte of the rx fifo and provide its value (via rxData)

	  When you write this register, it push a command into the fifo. There is the commands that you can use :
	    0x000000xx =>  Send byte xx
	    0x010000xx =>  Send byte xx and also push the read data into the FIFO
	    0x1100000X =>  Enable the SS line X
	    0x1000000X =>  Disable the SS line X
	*/

	spi.regs->DATA = 0x11000000; // enable the CS
	spi.regs->DATA = 0x0000009F; // command: read JEDEC ID
	spi.regs->DATA = 0x01000000; // send and read data
	spi.regs->DATA = 0x01000000; // send and read data
	spi.regs->DATA = 0x01000000; // send and read data
	spi.regs->DATA = 0x10000000; // disable the CS

	uint32_t d;
	uint32_t cnt = 0;

	do
	{
		d = spi.regs->DATA;
		if (d & (1u << 31)) // data valid?
		{
			spi_id[cnt] = (d & 0xFF);
			++cnt;
		}
	}
	while (cnt < 3);

  TRACE("JEDEC ID = %02X %02X %02X\r\n", spi_id[0], spi_id[1], spi_id[2]);
}

#endif

void test_spi_vihal()
{
#if 1
	spi.StartTransfer(0x9F, 0, SPITR_CMD1, 3, nullptr, &spi_id[0]);
	spi.WaitFinish();

  TRACE("JEDEC ID = %02X %02X %02X\r\n", spi_id[0], spi_id[1], spi_id[2]);
#endif

#if 1
  TRACE("Reading the first block...\r\n");
  unsigned t0, t1;
  t0 = CLOCKCNT;
  spi.StartTransfer(0x03, 1024*1024, SPITR_CMD1 | SPITR_ADDR3 | SPITR_EXTRA0, 4096, nullptr, &spi_buf[0]);
  spi.WaitFinish();
  t1 = CLOCKCNT;
  TRACE("Read finished in %u clocks.\r\n", t1 - t0);
#endif
}

void test_spiflash()
{
	int i;

	spiflash.spi = &spi;
	spiflash.has4kerase = true;
	if (!spiflash.Init())
	{
		TRACE("SPI Flash init failed!\r\n");
		return;
	}

	TRACE("SPI Flash initialized, ID CODE = %06X, kbyte size = %u\r\n", spiflash.idcode, (spiflash.bytesize >> 10));

#if 1
	TRACE("Reading memory...\r\n");

	spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], readlen);
	spiflash.WaitForComplete();

	TRACE("Memory read finished\r\n");

	show_mem(&databuf[0], readlen);
#endif

#if 1
	TRACE("Erase sector...\r\n");
	spiflash.StartEraseMem(TEST_START_ADDR, readlen);
	spiflash.WaitForComplete();
	TRACE("Erase complete.\r\n");
#endif

	TRACE("Writing memory...\r\n");

	for (i = 0; i < sizeof(databuf); ++i)
	{
		databuf[i] = 0xF0 + i;
	}

	spiflash.StartWriteMem(TEST_START_ADDR, &databuf[0], sizeof(databuf));
	spiflash.WaitForComplete();

#if 1
	TRACE("Reading memory...\r\n");

	spiflash.StartReadMem(TEST_START_ADDR, &databuf[0], readlen);
	spiflash.WaitForComplete();

	TRACE("Memory read finished\r\n");

	show_mem(&databuf[0], readlen);

	//return;
#endif

}

void test_spi()
{
	TRACE("SPI test begin\r\n");

#ifdef MCUSF_VRV100
	spi.speed = 8000000;
	spi.Init(1); // flash

#elif defined(BOARD_MIN_F401)

  //hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_AF_5);  // SPI1_NSS (CS)
  spi_cs_pin.Assign(PORTNUM_A, 4, false);
  spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_AF_5);  // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 6, PINCFG_AF_5);  // SPI1_MISO
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_AF_5);  // SPI1_MOSI

  spi.manualcspin = &spi_cs_pin;
  spi.speed = 8000000;
  spi.Init(1);

#if USE_DMA

  spi_txdma.Init(2, 5, 3);  // dma2/stream5/ch3
  spi_rxdma.Init(2, 0, 3);  // dma2/stream0/ch3

  spi.DmaAssign(true,  &spi_txdma);
  spi.DmaAssign(false, &spi_rxdma);

#endif

#else

  #error "Define board specific init"

#endif

	//test_spi_vihal();
	test_spiflash();

	TRACE("SPI test end\r\n");
}



