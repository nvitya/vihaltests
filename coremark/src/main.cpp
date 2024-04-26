// file:     main.cpp (coremark)
// brief:    VIHAL Coremark Main
// created:  2021-10-03
// authors:  nvitya

#include "platform.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "hwuscounter.h"

#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

THwUart   conuart;  // console uart

//-------------------------------------------------------------------------------
// RISC-V
//-------------------------------------------------------------------------------

#if defined(BOARD_VRV1_103) || defined(BOARD_VRV1_104) || defined(BOARD_VRV1_241) \
    || defined(BOARD_VRV1_403) || defined(BOARD_VRV1_441)|| defined(BOARD_VRV1_443) || defined(BOARD_VRV1_543)


TGpioPin  pin_led1(PORTNUM_A, 0, false);

#define LED_COUNT 1

void setup_board()
{
	pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

	conuart.Init(1); // UART1
}

#elif defined(BOARD_LONGAN_NANO)

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

#elif defined(BOARD_NODEMCU_ESP32C3)

TGpioPin  pin_led1(0, 3, false);
//TGpioPin  pin_led2(0, 4, false);
//TGpioPin  pin_led3(0, 5, false);

#define LED_COUNT 1

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //pin_led2.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //pin_led3.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);
}

//-------------------------------------------------------------------------------
// Risc-V: RV64G
//-------------------------------------------------------------------------------

#elif defined(BOARD_MAIX_BIT)

#define LED_COUNT 1

TGpioPin  pin_led1(0, 0, true);
TGpioPin  pin_led2(0, 1, true);
TGpioPin  pin_led3(0, 2, true);

void setup_board()
{
  // K210 specific pad routing, using GPIOHS
  hwpinctrl.PadSetup(12, FUNC_GPIOHS0, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(13, FUNC_GPIOHS1, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(14, FUNC_GPIOHS2, PINCFG_OUTPUT);
  // Assign the GPIOHS pins
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led2.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_led3.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  hwpinctrl.PadSetup(4, FUNC_UART3_RX, PINCFG_INPUT);
  hwpinctrl.PadSetup(5, FUNC_UART3_TX, PINCFG_OUTPUT);
  conuart.Init(3);
}

#elif defined(BOARD_MILKV_DUO)

#define LED_COUNT 1

TGpioPin  pin_led1(PORTNUM_C, 24, true);

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PadFuncSetup(PAD_SD1_GPIO1, FMUX_SD1_GPIO1__UART4_TX, 0);  // Milk-V Duo Board pin 4: UART4_TX
  hwpinctrl.PadFuncSetup(PAD_SD1_GPIO0, FMUX_SD1_GPIO0__UART4_RX, PINCFG_PULLUP);  // Milk-V Duo Board pin 5: UART4_RX
  conuart.Init(4);
}


//-------------------------------------------------------------------------------
// Xtensa (ESP32)
//-------------------------------------------------------------------------------

#elif defined(BOARD_ESP32_DEVKIT)

TGpioPin  pin_led1(0, 2, false);  // GPIO2 = on board led

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

TGpioPin  pin_led1(1, 0, false);
TGpioPin  pin_led2(1, 7, false);
TGpioPin  pin_led3(1, 14, false);

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

// RP

#elif defined(BOARD_RPI_PICO)

TGpioPin  pin_led1(0, 25, false);

void setup_board()
{
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  conuart.Init(0);
}

// IMXRT

#elif defined(BOARD_EVK_IMXRT1020) || defined(BOARD_EVK_IMXRT1024)

TGpioPin  pin_led1;

void setup_board()
{
#if defined(BOARD_EVK_IMXRT1024)
  pin_led1.Assign(1, 24, false);  // GPIO_AD_B1_08 = GPIO_1_24
#else
  pin_led1.Assign(1, 5, false);   // GPIO_AD_B0_05 = GPIO_1_5
#endif
  pin_led1.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_06_LPUART1_TX, 0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_07_LPUART1_RX, 0);
  conuart.Init(1); // UART1
}

#else

#error "unhandled board"

#endif

#ifndef LED_COUNT
  #define LED_COUNT 1
#endif

extern "C" void uart_send_char(char c)
{
  if (c == '\n')
  {
    conuart.SendChar('\r');
  }
  conuart.SendChar(c);
}

extern "C" void portable_calculate(unsigned total_ticks, unsigned iterations)
{
  TRACE("\r\n");
  TRACE("Precise coremark values:\r\n");
  unsigned t_ms = (total_ticks / 1000);
  unsigned cm_x100 = (100 * 1000 * iterations) / t_ms;
  unsigned mhz = (SystemCoreClock / 1000000);
  TRACE("  total millisecs     = %u\r\n", t_ms);
  TRACE("  CM (Iterations / s) = %1.2f\r\n", float(cm_x100) / 100.0);
  TRACE("  CM / MHz            = %1.2f\r\n", float(cm_x100) / float(100 * mhz));
  TRACE("\r\n");
}

// CoreMark Main:
extern "C" int main(int argc, char *argv[]);

// the C libraries require "_start" so we keep it as the entry point
extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required
{
	// after ram setup, region copy the cpu jumps here, with probably RC oscillator

	// TODO: !!!
  mcu_preinit_code(); // inline code for preparing the MCU, RAM regions. Without this even the stack does not work on some MCUs.

	// run the C/C++ initialization:
	cppinit();

	// Set the interrupt vector table offset, so that the interrupts and exceptions work
	mcu_init_vector_table();

#if defined(MCU_FIXED_SPEED)

  SystemCoreClock = MCU_FIXED_SPEED;

#else

  if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))
  {
    while (1)
    {
      // error
    }
  }

#endif

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present
	mcu_enable_dcache(); // enable data cache if present
	//mcu_disable_dcache();

#if defined(CMCC)
	// enable cache
	CMCC->CTRL.bit.CEN = 1;
#endif

	clockcnt_init();
	uscounter.Init();

	// go on with the hardware initializations
	setup_board();

	TRACE("\r\n--------------------------\r\n");
	TRACE("VIHAL Coremark\r\n");
	TRACE("Board: \"%s\"\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

	TRACE("Executing the CoreMark...\r\n");

	//SysTick_Config(SystemCoreClock / 50);  // slow tick for a few interrupt
	//mcu_interrupts_enable();

  pin_led1.Set1();
	main(0, nullptr);

	//TRACE("WARNING: for precise coremark value take the \"Total Ticks\" divide it with %u

	TRACE("\r\nStarting main cycle...\r\n");

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
