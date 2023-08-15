/*
 * pio_test.cpp
 *
 *  Created on: Aug 13, 2023
 *      Author: vitya
 */

#include "board_pins.h"
#include "traces.h"
#include "pio_test.h"

#include "pioapp_spi.h"

#define TEST_PIO_WAVE 0
#define TEST_PIO_SPI  1

#if TEST_PIO_WAVE

#include "pioapp_wave.h"

TPioAppWave  piowave;
TPioAppWave  piowave2;

void pio_test_init()
{
  TRACE("PIO Test Init...\r\n");

  unsigned wavepin;

  wavepin = 16;
  piowave.frequency = 3000000;
  piowave.Init(0, 0, wavepin);  // PIO=0, SM=0, PIN=16
  piowave.Start();
  TRACE("%u Hz square wave 1 started at pin %u\r\n", piowave.frequency, wavepin);

  wavepin = 17;
  piowave2.prgoffset = 8;
  piowave2.frequency = 5000000;
  piowave2.Init(0, 1, wavepin);  // PIO=0, SM=0, PIN=16
  piowave2.Start();
  TRACE("%u Hz square wave 2 started at pin %u\r\n", piowave2.frequency, wavepin);

}

void pio_test_run()
{
}


#elif TEST_PIO_SPI

TPioAppSpi   piospi;

void pio_test_init()
{
  TRACE("PIO Test Init...\r\n");

  piospi.prgoffset = 0;
  piospi.pin_sck  = 16;
  piospi.pin_mosi = 17;
  piospi.pin_miso = 18;
  piospi.frequency = 1000000;
  piospi.Init(0, 0);
  piospi.Start();
  piospi.TrySend8(0x12);
  piospi.TrySend8(0x34);
  piospi.TrySend8(0x56);

  TRACE("%u Hz SPI start at pins %u, %u\r\n", piospi.frequency, piospi.pin_sck, piospi.pin_mosi);
}

void pio_test_run()
{
  uint8_t b;
  if (piospi.TryRecv8(&b))
  {
    TRACE("SPI RX=%02X\r\n", b);
  }
}

#else
  #error "Select test mode !"
#endif





