// file:     main.cpp (uart)
// brief:    VIHAL UART Test
// created:  2021-10-03
// authors:  nvitya

#include "platform.h"
//#include "hwclkctrl.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "hwuscounter.h"
#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

#include "board_pins.h"

#if SPI_SELF_FLASHING
  #include "spi_self_flashing.h"
#endif

volatile unsigned hbcounter = 0;

void check_us_counter()
{
  const int uscap_len = 64;
  uint32_t  usarr[uscap_len];


  TRACE("Testing us counter...\r\n");

  uint32_t  ccnt = 0;
  unsigned t0, t1;
  unsigned usclocks = SystemCoreClock / 1000000;

  t0 = CLOCKCNT;
  while (ccnt < uscap_len)
  {
    t1 = CLOCKCNT;
    if (t1 - t0 > usclocks)
    {
      usarr[ccnt]  = uscounter.Get32();
      ++ccnt;
      t0 += usclocks;
    }
  }

  // displaying
  for (ccnt = 0; ccnt < uscap_len; ++ccnt)
  {
    TRACE("%3i. = %09u\r\n", ccnt, usarr[ccnt]);
  }
}

void check_us_counter_readout_speed()
{
  uint32_t t0, t1, t2;

  t0 = CLOCKCNT;
  if (uscounter.Get32()) {}
  t1 = CLOCKCNT;
  if (uscounter.Get32()) {}
  t2 = CLOCKCNT;
  TRACE("uscounter read clocks: %u, %u\r\n", t1-t0, t2-t1);
}

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
	uscounter.Init();

	// go on with the hardware initializations
	board_pins_init();

	TRACE("\r\n--------------------------------------\r\n");
	TRACE("VIHAL UART Multi-Core Test\r\n");
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

#if SPI_SELF_FLASHING

  if (spiflash.initialized)
  {
    TRACE("SPI Flash ID CODE: %08X, size = %u\r\n", spiflash.idcode, spiflash.bytesize);
  }
  else
  {
    TRACE("Error initializing SPI Flash !\r\n");
  }

  if (self_flashing)
  {
    spi_self_flashing(&spiflash);
  }
#endif

  //check_us_counter();
  check_us_counter_readout_speed();

	mcu_interrupts_enable();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast

	unsigned t0, t1;

	t0 = CLOCKCNT;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		char c;
		if (conuart.TryRecvChar(&c))
		{
		  conuart.printf("you pressed \"%c\"\r\n", c);
		}

		if (t1-t0 > hbclocks)
		{
			++hbcounter;

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

			TRACE("hbcounter=%u, uscounter=%u\r\n", hbcounter, uscounter.Get32()); // = conuart.printf()

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
