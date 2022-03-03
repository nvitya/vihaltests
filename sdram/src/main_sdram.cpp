// file:     main.cpp (sdram)
// brief:    VIHAL SDRAM Test
// created:  2022-03-03
// authors:  nvitya

#include "platform.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

#include "board_pins.h"
#include "hwsdram.h"
#include "sdram_test.h"


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
  //if (!hwclk_init(EXTERNAL_XTAL_HZ, 32000000))  // special for STM32F3, STM32F1
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

	TRACE("\r\n--------------------------------------\r\n");
	TRACE("VIHAL SDRAM TEST\r\n");
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

#if 1
  // The D-Cache must be enabled for effective SDRAM sequential (burst) transfers
  // the SDRAM sequential read performance is 4x (!) times better this way

  mcu_enable_dcache();
  TRACE("D-CACHE Enabled. Keep in mind for DMA transfers.\r\n");  // you have been warned. :)

  // the NVCM libraries does not contain cache invalidation for DMA transfers so this could be
  // a problem sometimes. This test does not use DMA transfers.
#endif


#if 1
  if (hwsdram.initialized)
  {
    TRACE("SDRAM initialized, size = %u kByte\r\n", hwsdram.byte_size >> 10);
    sdram_tests();
  }
  else
  {
    TRACE("SDRAM Init failed !\r\n");
  }
#endif

  TRACE("Starting main loop.\r\n");

	mcu_enable_interrupts();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast

	unsigned t0, t1;

	t0 = CLOCKCNT;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		if (t1-t0 > hbclocks)
		{
			++hbcounter;

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
