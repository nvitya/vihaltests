/*
 * file:     main_self_flash.cpp
 * brief:    VIHAL Self-Flash Test Main
 * created:  2022-05-29
 * authors:  nvitya
 * notes:
 *   Self flashing method can only be used when the entire code runs from RAM, so a special linker script is
 *   required. This is usually fine for Flash-less devices with lot of RAM, like the RP2040.
 *
 *   You only need a debug probe which loads the code into the RAM and starts with the "soft_entry".
 *   The code itself writes from the RAM image into the flash, calculates the checksums if necessary.
 *
 *   Drawbacks:
 *     - A slight amount of the code RAM is redundant (=wasted)
 *     - The code size is limited by the available RAM
 *   Advantages:
 *     - The code runs always at the maximal speed
 *     - You can fully use the external Flash device without blocking any other code
 *     - You don't need a debug probe supporting the actual external Flash device
*/

#include "string.h"
#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "board_pins.h"
#include "traces.h"

#include "spi_self_flashing.h"

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
	TRACE("VIHAL Self-Flashing Test\r\n");
  TRACE("Board: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

  #ifdef SPI_SELF_FLASHING

    if (spiflash.initialized)
    {
      TRACE("SPI Flash ID CODE: %08X, size = %u\r\n", spiflash.idcode, spiflash.bytesize);
      if (self_flashing)
      {
        spi_self_flashing(&spiflash);
      }
    }
    else
    {
      TRACE("Error initializing SPI Flash !\r\n");
    }

  #endif

	TRACE("Starting main cycle...\r\n");

	mcu_enable_interrupts();

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

