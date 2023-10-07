/*
 * file:     main_pio_wave.cpp
 * brief:    Square wave generation with RP2040 PIO
 * created:  2023-08-13
 * authors:  nvitya
*/

#include "string.h"
#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "board_pins.h"
#include "traces.h"
#include "spi_self_flashing.h"

#include "pio_test.h"

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

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

	TRACE("\r\n-----------------------------\r\n");
	TRACE("VIHAL PICO-W Comm Test\r\n");
  TRACE("Board: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

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

  pio_test_init();

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

		pio_test_run();

		if (t1-t0 > hbclocks)
		{
			++hbcounter;

			pin_led.SetTo(hbcounter & 1);

			//TRACE("hbcounter=%u\r\n", hbcounter);

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------

