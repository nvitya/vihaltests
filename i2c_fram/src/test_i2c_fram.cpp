/*
 *  file:     test_i2c_fram.cpp
 *  brief:    FM20V02A I2C FRAM test
 *  date:     2022-03-26
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2c.h"
#include "traces.h"
#include "clockcnt.h"

#include "board_pins.h"
#include "test_i2c_fram.h"

#define I2CADDR  0x50

uint8_t rxbuf[32];
uint8_t txbuf[32];

TI2cTransaction itra;

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

void test_i2c_fram()
{
  TRACE("I2C test for FM20V02A FRAM\r\n");

  unsigned addr = 0x0000; // byte order = MSB First
  unsigned len = 16;

  TRACE("Reading memory at %04X...\r\n", addr);

  i2c.StartRead(&itra, I2CADDR, addr | I2CEX_2, &rxbuf[0], len);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("  I2C error = %i\r\n", itra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
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

  i2c.StartWrite(&itra, I2CADDR, addr + incoffs | I2CEX_2, &txbuf[0], 4);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("  I2C error = %i\r\n", itra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
  }

  TRACE("Write finished.\r\n");

  TRACE("Reading memory at %04X...\r\n", addr);

  i2c.StartRead(&itra, I2CADDR, addr | I2CEX_2, &rxbuf[0], len);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("  I2C error = %i\r\n", itra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
  }

  show_mem(&rxbuf[0], len);

#if 1
  TRACE("Testing wrong device read...\r\n");

  i2c.StartRead(&itra, 0x7E, addr | I2CEX_2, &rxbuf[0], len);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("  I2C read error: %i\r\n", itra.error);
  }
  else
  {
    TRACE("  I2C ERROR MISSING!\\r\n");
    show_mem(&rxbuf[0], len);
  }
#endif

  TRACE("Reading memory after error again at %04X...\r\n", addr);
  i2c.StartRead(&itra, I2CADDR, addr | I2CEX_2, &rxbuf[0], len);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE(" !!! I2C error = %i\r\n", itra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
    show_mem(&rxbuf[0], len);
  }

#endif

  TRACE("I2C test finished.\r\n");
}
