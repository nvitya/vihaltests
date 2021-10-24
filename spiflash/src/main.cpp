/*
 * file:     main.cpp (spiflash)
 * brief:    VIHAL SPI Flash Test
 * created:  2021-10-03
 * authors:  nvitya
*/

#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

#include "test_spi.h"

THwUart   conuart;  // console uart

#if defined(BOARD_VRV100_441)

TGpioPin  pin_led1(PORTNUM_A, 0, false);

#define LED_COUNT 1

void setup_board()
{
	pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

	conuart.Init(1); // UART1
}

#endif

#if defined(BOARD_MIN_F401)

TGpioPin  pin_led1(2, 13, false); // PC13

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
}
#endif


#ifndef LED_COUNT
  #define LED_COUNT 1
#endif

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
	setup_board();

	TRACE("\r\n-----------------------------\r\n");
	TRACE("VIHAL SPI Flash Test\r\n");

	test_spi();

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
			//*hexnum = hbcounter;

      pin_led1.SetTo(hbcounter & 1);
      #if LED_COUNT > 1
        pin_led2.SetTo((hbcounter >> 1) & 1);
      #endif
      #if LED_COUNT > 2
        pin_led3.SetTo((hbcounter >> 2) & 1);
      #endif

			//TRACE("hbcounter=%u\r\n", hbcounter);

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
