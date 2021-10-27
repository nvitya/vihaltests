// file:     main.cpp (uart)
// brief:    VIHAL UART Test
// created:  2021-10-03
// authors:  nvitya

#include "platform.h"
//#include "hwclkctrl.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

THwUart   conuart;  // console uart

#if defined(BOARD_VRV100_103) || defined(BOARD_VRV100_104) || defined(BOARD_VRV100_241) \
    || defined(BOARD_VRV100_403) || defined(BOARD_VRV100_441)|| defined(BOARD_VRV100_443) || defined(BOARD_VRV100_543)

TGpioPin  pin_led1(PORTNUM_A, 0, false);

#define LED_COUNT 1

#if SPI_SELF_FLASHING

  #include "spiflash.h"
  #include "hwspi.h"
  #include "self_flashing.h"

  THwSpi    spi;
  TSpiFlash spiflash;

#endif

void show_hexnum(unsigned ahexnum)
{
  volatile uint32_t *  hexnum = (volatile uint32_t *)0xF1000000;
  *hexnum = ahexnum;
}

#define HEXNUM_DISPLAY

void setup_board()
{
	pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

	conuart.Init(1); // UART1

  #if SPI_SELF_FLASHING

    spi.speed = 10000000;
    spi.Init(1); // flash

    spiflash.spi = &spi;
    spiflash.has4kerase = false; // warning some ECP devices does not have 4k erase !
    spiflash.Init();

  #endif
}

#endif

#if defined(BOARD_LONGAN_NANO)

TGpioPin  pin_led1(PORTNUM_C, 13, true);
TGpioPin  pin_led2(PORTNUM_A,  1, true);
TGpioPin  pin_led3(PORTNUM_A,  2, true);

#define LED_COUNT 3

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led2.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led3.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  hwpinctrl.PinSetup(PORTNUM_A,  9, PINCFG_OUTPUT | PINCFG_AF_0);
  hwpinctrl.PinSetup(PORTNUM_A, 10, PINCFG_INPUT  | PINCFG_AF_0);
  conuart.Init(0); // USART0
}

#endif

#if defined(BOARD_MIN_F103)

TGpioPin  pin_led1(2, 13, false); // PC13

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_0 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
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

#if defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

TGpioPin  pin_led1(PORTNUM_B, 0, false);
TGpioPin  pin_led2(PORTNUM_B, 7, false);
TGpioPin  pin_led3(PORTNUM_B, 14, false);

#define LED_COUNT 3

void setup_board()
{
  // nucleo board leds
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led2.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led3.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  conuart.Init(3); // USART3
}

#endif

#if defined(BOARD_NUCLEO_H723)

TGpioPin  pin_led1(PORTNUM_B,  0, false);  // PB0 or PA5
TGpioPin  pin_led2(PORTNUM_E,  1, false);
TGpioPin  pin_led3(PORTNUM_B, 14, false);

#define LED_COUNT 3

void setup_board()
{
  // nucleo board leds
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led2.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led3.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  conuart.Init(3); // USART3
}

#endif

#if defined(BOARD_MIBO48_STM32G473) || defined(BOARD_MIBO48_STM32F303) || defined(BOARD_MIBO64_STM32F405)

TGpioPin  pin_led1(PORTNUM_C, 13, false);

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
}
#endif

#if defined(BOARD_MIBO20_STM32F030) || defined(BOARD_MIBO20_STM32F070)

TGpioPin  pin_led1(PORTNUM_B, 1, false);

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
}
#endif

#if defined(BOARD_XPLAINED_SAME70)

TGpioPin  pin_led1(2, 8, false);  // C8

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1 - EDBG
  hwpinctrl.PinSetup(0, 21, PINCFG_INPUT | PINCFG_AF_0);  // USART1_RXD
  MATRIX->CCFG_SYSIO |= (1 << 4); // select PB4 instead of TDI !!!!!!!!!
  hwpinctrl.PinSetup(1,  4, PINCFG_OUTPUT | PINCFG_AF_3); // USART1_TXD
  conuart.Init(0x101); // USART1

  // UART3 - Arduino shield
  //hwpinctrl.PinSetup(3, 28, PINCFG_INPUT | PINCFG_AF_0);  // UART3_RXD
  //hwpinctrl.PinSetup(3, 30, PINCFG_OUTPUT | PINCFG_AF_0); // UART3_TXD
  //uartx2.Init(3); // UART3
}

#endif

#if defined(BOARD_MIBO64_ATSAME5X)

TGpioPin  pin_led1(PORTNUM_A, 1, false);

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SERCOM0
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_OUTPUT | PINCFG_AF_3);  // PAD[0] = TX
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_INPUT  | PINCFG_AF_3);  // PAD[1] = RX
  conuart.Init(0);

  // SERCOM2
  //hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_AF_2);  // PAD[0] = TX
  //hwpinctrl.PinSetup(PORTNUM_A, 13, PINCFG_AF_2);  // PAD[1] = RX
  //conuart.Init(2);
}
#endif

#if defined(BOARD_XPRESSO_LPC54608)

TGpioPin  pin_led1(2, 2, true);
TGpioPin  pin_led2(3, 3, true);
TGpioPin  pin_led3(3, 14, true);

#define LED_COUNT 3

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led2.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led3.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  hwpinctrl.PinSetup(0, 30, PINCFG_OUTPUT | PINCFG_AF_1); // UART_TX:
  hwpinctrl.PinSetup(0, 29, PINCFG_INPUT  | PINCFG_AF_1); // UART_RX:
  conuart.Init(0);
}

#endif

#ifndef LED_COUNT
  #define LED_COUNT 1
#endif

#ifndef HEXNUM_DISPLAY
void show_hexnum(unsigned ahexnum)
{
}
#endif

volatile unsigned hbcounter = 0;

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator
  mcu_disable_interrupts();

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

#if defined(MCU_FIXED_SPEED)

  SystemCoreClock = MCU_FIXED_SPEED;

#else
  #if 0
    SystemCoreClock = MCU_INTERNAL_RC_SPEED;
  #else
    //if (!hwclk_init(0, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
    //if (!hwclk_init(0, 64000000))  // special for STM32F3, STM32F1
    if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
    {
      while (1)
      {
        // error
      }
    }
  #endif
#endif

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations
	setup_board();

	TRACE("\r\n--------------------------------------\r\n");
	TRACE("Hello From VIHAL !\r\n");
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

      pin_led1.SetTo(hbcounter & 1);
      #if LED_COUNT > 1
        pin_led2.SetTo((hbcounter >> 1) & 1);
      #endif
      #if LED_COUNT > 2
        pin_led3.SetTo((hbcounter >> 2) & 1);
      #endif

      show_hexnum(hbcounter);

			TRACE("hbcounter=%u\r\n", hbcounter);

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
