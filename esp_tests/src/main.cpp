
#include "platform.h"
#include "hwclk.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "board_pins.h"

#include "esp_tests.h"

#include "hwpins.h"
#include "hwqspi.h"
#include "hwuart.h"
#include "hwspi.h"
#include "traces.h"

THwQspi   qspi;
THwSpi    spi;
TGpioPin  pin_spi_cs;
TGpioPin  pin_spifl_cs;

uint8_t txbuf[4*1024];
uint8_t rxbuf[4*1024];

#if defined(BOARD_NODEMCU_ESP32C3)

void spi_prepare()
{
  pin_spifl_cs.Assign(0, 14, false);
  pin_spifl_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // CS as GPIO
  //hwpinctrl.PadSetup(PAD_GPIO14, SIG_GPIO_OUT_IDX, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);  // CS as GPIO

  // the FSPI = SPI2 = the general purpose SPI

  hwpinctrl.PadSetup(PAD_GPIO10, FSPICS0_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_2);         // CS0,  AF_2 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO2,  FSPIQ_IN_IDX,     PINCFG_INPUT  | PINCFG_AF_2);         // MISO, AF_2 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO7,  FSPID_OUT_IDX,    PINCFG_OUTPUT | PINCFG_AF_2);         // MOSI, AF_2 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO6,  FSPICLK_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_2);         // CLK,  AF_2 = direct routing


  pin_spi_cs.Assign(0, 3, false);
  pin_spi_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // CS as GPIO

  spi.manualcspin = &pin_spi_cs;
  spi.speed = 1000000;
  spi.Init(2);
}

void spiflash_prepare()
{
  // the normal SPI flash (SPI1) is coupled to the "SPIxxx" pins
  hwpinctrl.PadSetup(PAD_GPIO14, SPICS0_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_0);         // CS0,  AF_0 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO17, SPIQ_IN_IDX,     PINCFG_INPUT  | PINCFG_AF_0);         // MISO, AF_0 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO16, SPID_OUT_IDX,    PINCFG_OUTPUT | PINCFG_AF_0);         // MOSI, AF_0 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO15, SPICLK_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_0);         // CLK,  AF_0 = direct routing

  qspi.speed = 4000000;
  qspi.Init();
}

#elif defined(BOARD_ESP32_DEVKIT)

void spi_prepare()
{
  pin_spi_cs.Assign(0, 5, false);
  pin_spi_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // CS as GPIO
  //hwpinctrl.PadSetup(PAD_GPIO5, SIG_GPIO_OUT_IDX, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);  // CS as GPIO
  hwpinctrl.PadSetup(PAD_GPIO19, VSPIQ_IN_IDX,     PINCFG_INPUT  | PINCFG_AF_1);         // MISO, AF_1 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO23, VSPID_OUT_IDX,    PINCFG_OUTPUT | PINCFG_AF_1);         // MOSI, AF_1 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO18, VSPICLK_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_1);         // CLK,  AF_1 = direct routing

  spi.manualcspin = &pin_spi_cs;
  spi.speed = 1000000;
  spi.Init(3); // VSPI = SPI3
}

void spiflash_prepare()
{
  pin_spifl_cs.Assign(0, 11, false);
  pin_spifl_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // CS as GPIO

  //hwpinctrl.PadSetup(PAD_GPIO11, SPICS0_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_0);         // CS0,  AF_1 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO7,  SPIQ_IN_IDX,     PINCFG_INPUT  | PINCFG_AF_1);         // MISO, AF_1 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO8,  SPID_OUT_IDX,    PINCFG_OUTPUT | PINCFG_AF_1);         // MOSI, AF_1 = direct routing
  hwpinctrl.PadSetup(PAD_GPIO6,  SPICLK_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_1);         // CLK,  AF_1 = direct routing

  spi.manualcspin = &pin_spifl_cs;
  spi.speed = 1000000;
  spi.Init(1);  // 1 = SPI
}


#else
  #error "unhandled board."
#endif

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



void spiflash_test()
{
  unsigned n;

  TRACE("Testing SPI FLASH...\r\n");

  spiflash_prepare();

  qspi.StartReadData(0x9F, 0, &rxbuf[0], 4);
  qspi.WaitFinish();

  TRACE("SPI RX = %02X %02X %02X %02X\r\n", rxbuf[0], rxbuf[1], rxbuf[2], rxbuf[3]);

  TRACE("Reading 64 bytes:\r\n");

  unsigned rlen = 256;

  qspi.StartReadData(0x03 | QSPICM_ADDR3, 0, &rxbuf[0], rlen);
  //qspi.StartReadData(0x0B | QSPICM_ADDR3 | QSPICM_DUMMY1, 0, &rxbuf[0], 64);
  qspi.WaitFinish();

  TRACE("RX data:\r\n");
  for (n = 0; n < rlen; ++n)
  {
    TRACE(" %02X", rxbuf[n]);
    if (15 == (n & 15))
    {
      TRACE("\r\n");
    }
  }

  TRACE("SPI Flash finished.\r\n");
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

	//if (!hwclk_init(EXTERNAL_XTAL_HZ, 40000000))
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

  //spi_test();
  spiflash_test();

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
        //pin_led[n].SetTo((hbcounter >> n) & 1);
      }

      //TRACE("hbcounter=%u\r\n", hbcounter); // = conuart.printf()

      t0 = t1;

      if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
    }
	}
}

// ----------------------------------------------------------------------------
