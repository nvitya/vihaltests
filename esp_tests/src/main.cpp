
#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "board_pins.h"

#include "esp_tests.h"

#include "hwpins.h"
#include "hwuart.h"

THwUart  uart;

void uart_init()
{
  //hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT);
  //hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode

  uart.Init(0);
}

extern "C" int __getreent()
{
  return 0;
}


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

	//if (!hwclk_init(EXTERNAL_XTAL_HZ, 20000000))
	if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
	{
	  while (1)
	  {
	    // error
	  }
	}

#endif

	//run_esp_tests();

	//uart_init();

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations (board_pins.cpp)
	board_pins_init();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast
	unsigned hbcounter = 0;

	unsigned t0, t1;

	t0 = CLOCKCNT;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		if (t1-t0 > hbclocks)
		{
			++hbcounter;

			uart.TrySendChar('A');

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
