/*
 * file:     main_can.cpp
 * brief:    VIHAL CAN Test
 * created:  2022-03-23
 * authors:  nvitya
 *
 * description:
 *   Simple CAN example, runs with 1 MBit/s, sends one message and then prints
 *   the received messages to the trace UART
*/

#include "platform.h"
#include "hwpins.h"
#include "hwclk.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "hwusbctrl.h"
#include "board_pins.h"
#include "can_test.h"

#include "traces.h"

volatile unsigned hbcounter = 0;


extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator
  mcu_disable_interrupts();

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

  //if (!hwclk_init(0, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
  //if (!hwclk_init(EXTERNAL_XTAL_HZ, 72000000))  // special for STM32F3, STM32F1
  if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
  {
    while (1)
    {
      // error
    }
  }

  mcu_enable_fpu();    // enable coprocessor if present
  mcu_enable_icache(); // enable instruction cache if present

  clockcnt_init();

  // go on with the hardware initializations
  board_pins_init();
  traces_init();

  //tracebuf.waitsend = false;  // force to buffered mode (default)
  //tracebuf.waitsend = true;  // better for basic debugging

  TRACE("\r\n--------------------------------------\r\n");
  TRACE("VIHAL CAN Test\r\n");
  TRACE("Board: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

  TRACE_FLUSH();

  mcu_enable_interrupts();

  can_test_init();

  TRACE("\r\nStarting main cycle...\r\n");

  unsigned hbclocks = SystemCoreClock / 2;

  unsigned t0, t1;

  t0 = CLOCKCNT;

  // Infinite loop
  while (1)
  {
    t1 = CLOCKCNT;

    can_test_run();

    tracebuf.Run();

    if (t1-t0 > hbclocks)
    {

      t0 = t1;
    }
  }
}

// ----------------------------------------------------------------------------
