/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM Tests project: https://github.com/nvitya/nvcmtests
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     test_i2c_eeprom.cpp
 *  brief:    LC93LC46B I2C EEPROM test
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2c.h"
#include "traces.h"
#include "clockcnt.h"
#include "i2c_eeprom.h"  // from vihal/modules

#include "board_pins.h"
#include "test_i2c_eeprom.h"

#define I2CADDR  0x50

TI2cEeprom   eeprom;

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

static uint8_t rxbuf[1024];
static uint8_t txbuf[1024];

void test_i2c_interface()
{
#if 1 // very simple read test with 1 byte addressing
  i2c.StartReadData(I2CADDR, 0 | I2CEX_1, &rxbuf[0], 4); // read byte with 1 byte addressing
  i2c.WaitFinish();
  if (i2c.error)
  {
    TRACE("Read error: %i\r\n", i2c.error);
  }

  show_mem(&rxbuf[0], 4);

  return;

#endif

#if 0

  txbuf[0] = 0xDA;
  i2c.StartWriteData(I2CADDR, 0 | I2CEX_1, &txbuf[0], 1); // write byte with addressing
  i2c.WaitFinish();
  if (i2c.error)
  {
    TRACE("Write without addressing error: %i\r\n", i2c.error);
  }

  //TRACE("Read without addressing ok.\r\n");

  //TRACE("Reading byte without addressing...\r\n");
  i2c.StartReadData(I2CADDR, 0, &rxbuf[0], 1); // read byte without addressing
  i2c.WaitFinish();
  if (i2c.error)
  {
    TRACE("Read without addressing error: %i\r\n", i2c.error);
  }

  //TRACE("Read without addressing ok.\r\n");


  //TRACE("Reading byte with addressing...\r\n");
  i2c.StartReadData(I2CADDR, 0 | I2CEX_1, &rxbuf[0], 1); // read byte without addressing
  i2c.WaitFinish();
  if (i2c.error)
  {
    TRACE("Read with addressing error: %i\r\n", i2c.error);
  }

  TRACE("Read with addressing ok.\r\n");

  return;

#endif

#if 0
  int errcnt = 0;

  TRACE("1x short read test...\r\n");
  int n;
  for (n = 0; n < 8; ++n)
  {
    //i2c.StartReadData(I2CADDR, n | I2CEX_1, &rxbuf[0], 1);
    i2c.StartReadData(I2CADDR, n | I2CEX_1, &rxbuf[0], 1);
    i2c.WaitFinish();
    if (i2c.error)
    {
      ++errcnt;
      //TRACE("Read 1 I2C error: %i\r\n", i2c.error);
      //return;
    }

    delay_us(50);
  }

  TRACE("Test finished, errcnt = %i\r\n", errcnt);

  return;
#endif
}

void test_i2c_eeprom()
{
	int i;

	TRACE("I2C test for 24LC16B EEPROM\r\n");

	if (!i2c.initialized)
	{
	  TRACE("I2C is not initialized!\r\n");
	  return;
	}

  TRACE("  speed = %u\r\n", i2c.speed);

  //test_i2c_interface();
  //return;

  // 24LC16B:
  //   address 3..7:  1010
  //   address 0..2:      ppp = Page address
  //
  //   the chip uses 8 bit addressing (followed by the device address byte) within the page

  if (!eeprom.Init(&i2c, 0x50, 256 * 8))
  {
    TRACE("Error initializing the EEPROM: %08X\r\n", eeprom.errorcode);
    return;
  }

  TRACE("EEPROM Initialized.\r\n");

	unsigned addr = 0x0000; // byte order = MSB First
	unsigned len = 64;
	unsigned len2 = 1;
	int r;

#if 1 // I2C driver test
	TRACE("I2C Reading 1 byte from invalid address...\r\n");
	i2c.StartReadData(0x40, 0, &rxbuf[0], 1);
	r = i2c.WaitFinish();
	TRACE("Completed: %i\r\n", r);

	len2 = 1;
	TRACE("Reading %i byte at %04X...\r\n", len2, addr);
	eeprom.StartReadMem(addr, &rxbuf[0], len2);
	eeprom.WaitComplete();
  if (eeprom.errorcode)
  {
    TRACE(" EEPROM error = %i\r\n", eeprom.errorcode);
    return;
  }
  else
  {
	  show_mem(&rxbuf[0], len2);
  }

	len2 = 2;
	TRACE("Reading %i byte at %04X...\r\n", len2, addr);
	eeprom.StartReadMem(addr, &rxbuf[0], len2);
	eeprom.WaitComplete();
  if (eeprom.errorcode)
  {
    TRACE(" EEPROM error = %i\r\n", eeprom.errorcode);
    return;
  }
  else
  {
    show_mem(&rxbuf[0], len2);
  }

	len2 = 3;
	TRACE("Reading %i byte at %04X...\r\n", len2, addr);
	eeprom.StartReadMem(addr, &rxbuf[0], len2);
	eeprom.WaitComplete();
  if (eeprom.errorcode)
  {
    TRACE(" EEPROM error = %i\r\n", eeprom.errorcode);
    return;
  }
  else
  {
    show_mem(&rxbuf[0], len2);
  }
#endif

  TRACE("Reading memory at %04X...\r\n", addr);
  eeprom.StartReadMem(addr, &rxbuf[0], len);
  eeprom.WaitComplete();
  if (eeprom.errorcode)
  {
    TRACE(" EEPROM error = %i\r\n", eeprom.errorcode);
    return;
  }
  show_mem(&rxbuf[0], len);

#if 1
	unsigned incoffs = 4;

	TRACE("Incrementing memory at +%i...\r\n", incoffs);

	txbuf[0] = rxbuf[incoffs+0] + 1;
	txbuf[1] = rxbuf[incoffs+1] + 1;
	txbuf[2] = rxbuf[incoffs+2] + 1;
	txbuf[3] = rxbuf[incoffs+3] + 1;
	txbuf[4] = rxbuf[incoffs+4] + 1;
	txbuf[5] = rxbuf[incoffs+5] + 1;
	txbuf[6] = rxbuf[incoffs+6] + 1;
	txbuf[7] = rxbuf[incoffs+7] + 1;

	eeprom.StartWriteMem(addr + incoffs, &txbuf[0], 4);
	eeprom.WaitComplete();
	if (eeprom.errorcode)
	{
		TRACE(" EEPROM error = %i\r\n", eeprom.errorcode);
		return;
	}

	TRACE("Write finished.\r\n");

	TRACE("Reading memory at %04X...\r\n", addr);

	eeprom.StartReadMem(addr, &rxbuf[0], len);
	eeprom.WaitComplete();
	if (eeprom.errorcode)
	{
		TRACE(" EEPROM error = %i\r\n", eeprom.errorcode);
		return;
	}

	show_mem(&rxbuf[0], len);
#endif

#if 1  // multi page write + read
	len = 1024;
	addr = 0;

	TRACE("Filling %u bytes memory with 0xA5 + n ... \r\n", len);
	for (i = 0; i < len; ++i)
	{
		txbuf[i] = 0xA5 + i;
	}

	eeprom.StartWriteMem(addr, &txbuf[0], len);
	eeprom.WaitComplete();
	if (eeprom.errorcode)	{ TRACE(" EEPROM error = %i\r\n", eeprom.errorcode); }

	TRACE("Reading %u bytes memory at %04X...\r\n", len, addr);
	eeprom.StartReadMem(addr, &rxbuf[0], len);
	eeprom.WaitComplete();
	show_mem(&rxbuf[0], len);
#endif

	TRACE("EEPROM test finished.\r\n");
}
