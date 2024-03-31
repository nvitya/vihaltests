/*
 * file:     main.cpp (spiflash)
 * brief:    VIHAL SPI Flash Test Main
 * created:  2021-10-03
 * authors:  nvitya
*/

#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "board_pins.h"
#include "traces.h"

#include "test_sdcard.h"

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

#if defined(MCU_FIXED_SPEED)

  SystemCoreClock = MCU_FIXED_SPEED;

#else

  if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
  {
    while (1)
    {
      // error
    }
  }

#endif

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations
	board_pins_init();

	TRACE("\r\n-----------------------------\r\n");
	TRACE("VIHAL SDCARD FatFS Test\r\n");
  TRACE("Board: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

	test_sdcard();

	TRACE("Starting main cycle...\r\n");

	mcu_interrupts_enable();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast
	unsigned hbcounter = 0;

	unsigned t0, t1;

	t0 = CLOCKCNT;

	//volatile uint32_t *  hexnum = (volatile uint32_t *)0xF1000000;

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

			//TRACE("hbcounter=%u\r\n", hbcounter);

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
