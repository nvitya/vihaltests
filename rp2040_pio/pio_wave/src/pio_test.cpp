/*
 * pio_test.cpp
 *
 *  Created on: Aug 13, 2023
 *      Author: vitya
 */

#include "board_pins.h"
#include "traces.h"
#include "pio_test.h"

#include "pioapp_wave.h"

void pio_test_init()
{
  TRACE("PIO Test Init...\r\n");

  unsigned wavepin = 16;

  piowave.frequency = 3000000;  // 15 MHz wave

  piowave.Init(0, 0, 16);  // PIO=0, SM=0, PIN=16
  piowave.Start();

  TRACE("%u Hz square wave started at pin %u\r\n", piowave.frequency, wavepin);
}

void pio_test_run()
{

}



