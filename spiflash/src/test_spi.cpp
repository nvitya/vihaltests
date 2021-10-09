/*
 * test_spi.cpp
 *
 *  Created on: Oct 7, 2021
 *      Author: vitya
 */

#include "platform.h"
#include "hwspi.h"
#include "traces.h"

THwSpi  spi;

uint8_t spi_id[4];

void test_spi()
{
	TRACE("SPI test begin\r\n");

	spi.speed = 2000000;
	spi.Init(1); // flash

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

	TRACE("SPI test end\r\n");
}



