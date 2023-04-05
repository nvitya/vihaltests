/*
 * test_i2c.cpp
 *
 *  Created on: Apr 1, 2023
 *      Author: vitya
 */

#include "hwi2c.h"
#include "traces.h"
#include "test_i2c.h"

#define I2CADDR  0x50

THwI2c  i2c;

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

TI2cTransaction itra;

void test_i2c_interface()
{

#if 0 // very simple read test with 2 byte addressing
  i2c.StartRead(&itra, I2CADDR, 0 | I2CEX_2, &rxbuf[0], 128); // read byte with 1 byte addressing
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("Read error: %i\r\n", itra.error);
    return;
  }

  show_mem(&rxbuf[0], 128);

  return;

#endif

#if 0 // long (chunked) write
  for (int i = 0; i < 128; ++i)
  {
    txbuf[i] = 0x80 + i;
  }

  TRACE("Long write test...\r\n");

  i2c.StartWrite(&itra, I2CADDR, 0x100 | I2CEX_2, &txbuf[0], 128);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("  i2c error: %i\r\n", itra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
  }

  return;
#endif

#if 1 // long (chunked) read
  TRACE("Long read test at 0x100 ...\r\n");

  i2c.StartRead(&itra, I2CADDR, 0x100 | I2CEX_2, &rxbuf[0], 128);
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("  i2c error: %i\r\n", itra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
  }
  show_mem(&rxbuf[0], 128);

  return;
#endif


#if 0

  txbuf[0] = 0xDA;
  i2c.StartWrite(&itra, I2CADDR, 0 | I2CEX_1, &txbuf[0], 1); // write byte with addressing
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("Write without addressing error: %i\r\n", itra.error);
  }

  //TRACE("Read without addressing ok.\r\n");

  //TRACE("Reading byte without addressing...\r\n");
  i2c.StartRead(&itra, I2CADDR, 0, &rxbuf[0], 1); // read byte without addressing
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("Read without addressing error: %i\r\n", itra.error);
  }

  //TRACE("Read without addressing ok.\r\n");


  //TRACE("Reading byte with addressing...\r\n");
  i2c.StartRead(&itra, I2CADDR, 0 | I2CEX_1, &rxbuf[0], 1); // read byte without addressing
  i2c.WaitFinish(&itra);
  if (itra.error)
  {
    TRACE("Read with addressing error: %i\r\n", itra.error);
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
    i2c.StartRead(&itra, I2CADDR, n | I2CEX_1, &rxbuf[0], 1);
    i2c.WaitFinish(&itra);
    if (itra.error)
    {
      ++errcnt;
      //TRACE("Read 1 I2C error: %i\r\n", itra.error);
      //return;
    }

    delay_us(50);
  }

  TRACE("Test finished, errcnt = %i\r\n", errcnt);

  return;
#endif
}

void test_i2c()
{
  TRACE("I2C Test\r\n");

  //hwpinctrl.PadSetup(PAD_GPIO1, I2CEXT0_SDA_IN_IDX,  PINCFG_INPUT);   // SDA
  hwpinctrl.PadSetup(PAD_GPIO1, I2CEXT0_SDA_OUT_IDX, PINCFG_OUTPUT | PINCFG_OPENDRAIN);  // SDA
  hwpinctrl.PadInput(PAD_GPIO1, I2CEXT0_SDA_IN_IDX);

  hwpinctrl.PadSetup(PAD_GPIO0, I2CEXT0_SCL_OUT_IDX, PINCFG_OUTPUT | PINCFG_OPENDRAIN);  // SCL
  hwpinctrl.PadInput(PAD_GPIO0, I2CEXT0_SCL_IN_IDX);

  i2c.Init(0);

  test_i2c_interface();

  TRACE("I2C test finished.\r\n");
}
