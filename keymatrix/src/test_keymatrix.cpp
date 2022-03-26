/*
 *  file:     test_keymatrix.cpp
 *  brief:    Keyboard Matrix Test
 *  date:     2022-03-26
 *  authors:  nvitya
*/

#include "stdint.h"
#include "stdlib.h"
#include "hwpins.h"
#include "clockcnt.h"
#include "board_pins.h"
#include "traces.h"

uint32_t prev_keys = 0;

#if 0

// this might help to find out the layout

TGpioPin kpin1(PORTNUM_A, 0, false);
TGpioPin kpin2(PORTNUM_A, 1, false);

void pintest_init()
{
  kpin1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  kpin2.Setup(PINCFG_INPUT | PINCFG_PULLUP);
}

void pintest_run()
{
  TRACE("\rPIN 1-2 value: %u", kpin2.Value());
}

#endif

void test_keymatrix_init()
{
  //pintest_init();
  //return;

  if (!keymatrix.Init(KEYMATRIX_ROWS, KEYMATRIX_COLS))
  {
    TRACE("Keyboard matrix initialization failed!\r\n  (probably pin assignments missing)\r\n");
    return;
  }

  TRACE("Keyboard matrix initialized.\r\n");
}

void test_keymatrix_run()
{
  //pintest_run();
  //return;

  keymatrix.Run();

  if (prev_keys != keymatrix.keys32[0])
  {
    TRACE("Keys changed: %08X\r\n", keymatrix.keys32[0]);
    prev_keys = keymatrix.keys32[0];
  }
}


// ----------------------------------------------------------------------------
