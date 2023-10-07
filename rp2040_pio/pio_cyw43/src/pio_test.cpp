/*
 * pio_test.cpp
 *
 *  Created on: Aug 13, 2023
 *      Author: vitya
 */

#include "board_pins.h"
#include "traces.h"
#include "pio_test.h"

#include "pioapp_cyw43_spi.h"

TPioAppCyw43Spi  pioapp;

uint32_t  txbuf[64];
uint32_t  rxbuf[64];

void pio_test_init()
{
  unsigned n;

  TRACE("CYW43 Test Init...\r\n");

  pioapp.prgoffset = 0;
  pioapp.frequency = 4000000; // 4MHz for light scoping
  pioapp.Init(0, 0);

  uint32_t txcnt = 1;
  uint32_t rxcnt = 1;

  TRACE("Testing transfer TX=%u, RX=%u\r\n", txcnt, rxcnt);

  txbuf[0] = 0xA0044000;
  rxbuf[0] = 0x55555555;

  pioapp.StartTransfer(&txbuf[0], txcnt, &rxbuf[0], rxcnt);

  for (n = 0; n < txcnt; ++n)
  {
    TRACE("--> %08X\r\n", txbuf[n]);
  }

  for (n = 0; n < rxcnt; ++n)
  {
    TRACE("<-- %08X\r\n", rxbuf[n]);
  }

  TRACE("CYW43 test finished.\r\n");
}

void pio_test_run()
{
#if 0
  uint8_t b;
  if (piospi.TryRecv8(&b))
  {
    TRACE("SPI RX=%02X\r\n", b);
  }
#endif
}
