// file:     main.cpp (uart)
// brief:    VIHAL UART Test
// created:  2021-10-03
// authors:  nvitya

#include "platform.h"
//#include "hwclkctrl.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

#include "test_spi.h"

THwUart   conuart;  // console uart

#if defined(BOARD_VRV153)

TGpioPin  pin_led1(PORTNUM_A, 0, false);

#define LED_COUNT 1

void setup_board()
{
	pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

	conuart.Init(1); // UART1
}

#endif

#ifndef LED_COUNT
  #define LED_COUNT 1
#endif

// the C libraries require "_start" so we keep it as the entry point
extern "C" __attribute__((noreturn)) void _start(void)
{
	// after ram setup, region copy the cpu jumps here, with probably RC oscillator

	// TODO: !!!
  mcu_preinit_code(); // inline code for preparing the MCU, RAM regions. Without this even the stack does not work on some MCUs.

	// run the C/C++ initialization:
	cppinit();

	// Set the interrupt vector table offset, so that the interrupts and exceptions work
	mcu_init_vector_table();

	unsigned cpu_clock_speed;

#if defined(MCU_FIXED_SPEED)

	cpu_clock_speed = MCU_FIXED_SPEED;

#else

  #error "Setup CPU clock!"
	{
		while (1)
		{
			// the external oscillator did not start.
		}
	}
#endif


	// provide info to the system about the clock speed:
	SystemCoreClock = cpu_clock_speed;

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations
	setup_board();

	TRACE("VIHAL SPI Flash Test\r\n");

	test_spi();

	TRACE("Starting main cycle...\r\n");

	mcu_enable_interrupts();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast
	unsigned hbcounter = 0;

	unsigned t0, t1;

	t0 = CLOCKCNT;

	volatile uint32_t *  hexnum = (volatile uint32_t *)0xF1000000;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		if (t1-t0 > hbclocks)
		{
			++hbcounter;
			*hexnum = hbcounter;

			pin_led1.Toggle();

			//TRACE("hbcounter=%u\r\n", hbcounter);

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
