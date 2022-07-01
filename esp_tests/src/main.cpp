
#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "board_pins.h"

#include "esp_tests.h"

#include "hwpins.h"
#include "hwuart.h"
#include "hwspi.h"
#include "traces.h"

THwSpi    spi;
TGpioPin  pin_spi_cs;

uint8_t txbuf[256];
uint8_t rxbuf[256];

void spi_prepare()
{
  pin_spi_cs.Assign(0, 3, false);
  pin_spi_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // CS as GPIO
  //hwpinctrl.PadSetup(PAD_GPIO3, SIG_GPIO_OUT_IDX, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);  // CS as GPIO
  hwpinctrl.PadSetup(PAD_GPIO2, FSPIQ_IN_IDX,     PINCFG_INPUT  | PINCFG_AF_2);         // MISO, AF_2 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO7, FSPID_OUT_IDX,    PINCFG_OUTPUT | PINCFG_AF_2);         // MOSI, AF_2 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO6, FSPICLK_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_2);         // CLK,  AF_2 = direct routing

  spi.manualcspin = &pin_spi_cs;
  spi.speed = 1000000;
  spi.Init(2);
}

void spi_test()
{
  unsigned n;

  TRACE("Testing SPI...\r\n");

  spi_prepare();

  for (n = 0; n < sizeof(txbuf); ++n)
  {
    txbuf[n] = 0xA0 + n;
    rxbuf[n] = 0x55;
  }

  spi.StartTransfer(0, 0, 0, 256, &txbuf[0], &rxbuf[0]);
  spi.WaitFinish();

  TRACE("Spi finished.\r\n");
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

	//if (!hwclk_init(EXTERNAL_XTAL_HZ, 160000000))
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

  TRACE("\r\n--------------------------------------\r\n");
  TRACE("ESP Tests\r\n");
  TRACE("Board: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

  spi_test();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast
	unsigned hbcounter = 0;

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

      //TRACE("hbcounter=%u\r\n", hbcounter); // = conuart.printf()

      t0 = t1;

      if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
    }
	}
}

// ----------------------------------------------------------------------------
