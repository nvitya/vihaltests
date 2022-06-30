/*
 *  file:     board_pins.cpp (uart)
 *  brief:    UART Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 1;

TGpioPin  pin_led[MAX_LEDS] =
{
  TGpioPin(),
  TGpioPin(),
  TGpioPin(),
  TGpioPin()
};

/* NOTE:
     for direct GPIO pin definitions is simpler to define with port and pin number:

       TGpioPin  pin_mygpio(PORTNUM_C, 13, false);

     but don't forget to initialize it in the setup code:

       pin_mygpio.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
*/

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

#elif defined(BOARD_LONGAN_NANO)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_C, 13, true);
  pin_led[1].Assign(PORTNUM_A,  1, true);
  pin_led[2].Assign(PORTNUM_A,  2, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9, PINCFG_OUTPUT | PINCFG_AF_0);
  hwpinctrl.PinSetup(PORTNUM_A, 10, PINCFG_INPUT  | PINCFG_AF_0);
  conuart.Init(0); // USART0
}

#elif defined(MCUF_VRV100)

#define HEXNUM_DISPLAY
void board_show_hexnum(unsigned ahexnum)
{
  volatile uint32_t *  hexnum = (volatile uint32_t *)0xF1000000;
  *hexnum = ahexnum;
}

#if SPI_SELF_FLASHING

  #include "hwspi.h"
  #include "spiflash.h"

  THwSpi     fl_spi;
  TSpiFlash  spiflash;

#endif

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 0, false);
  board_pins_init_leds();

  board_show_hexnum(0);

  conuart.Init(1); // UART1

  #if SPI_SELF_FLASHING

    fl_spi.speed = 10000000;
    fl_spi.Init(1); // flash

    spiflash.spi = &fl_spi;
    spiflash.has4kerase = false; // warning some ECP devices does not have 4k erase !
    spiflash.Init();

  #endif
}

#elif defined(BOARD_NODEMCU_ESP32C3)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(0, 3, false);
  pin_led[1].Assign(0, 4, false);
  pin_led[2].Assign(0, 5, false);
  board_pins_init_leds();

  //hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);
}

//-------------------------------------------------------------------------------
// Xtensa (ESP32)
//-------------------------------------------------------------------------------

#elif defined(BOARD_ESP32_DEVKIT)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 2, false);  // GPIO2 = on board led
  board_pins_init_leds();

  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B,  0, false);
  pin_led[1].Assign(PORTNUM_B,  7, false);
  pin_led[2].Assign(PORTNUM_B, 14, false);
  board_pins_init_leds();

  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  conuart.Init(3); // USART3
}

#elif defined(BOARD_NUCLEO_H723)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B,  0, false);  // PB0 or PA5
  pin_led[1].Assign(PORTNUM_E,  1, false);
  pin_led[2].Assign(PORTNUM_B, 14, false);
  board_pins_init_leds();

  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  conuart.Init(3); // USART3
}

#elif defined(BOARD_NUCLEO_G474RE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A,  5, false);
  board_pins_init_leds();

#if 0
  // LPUART1: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_A, 2,  PINCFG_OUTPUT | PINCFG_AF_12);  // LPUART1.TX
  hwpinctrl.PinSetup(PORTNUM_A, 3,  PINCFG_INPUT  | PINCFG_AF_12);  // LPUART1.RX
  conuart.Init(0x101); // 0x101 = LPUART1, 0x001 = USART1
#else
  // USART2: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_A, 2,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART2.TX
  hwpinctrl.PinSetup(PORTNUM_A, 3,  PINCFG_INPUT  | PINCFG_AF_7);  // USART2.RX
  conuart.Init(2);
#endif
}

#elif defined(BOARD_NUCLEO_G431KB)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B,  8, false);
  board_pins_init_leds();

#if 0
  // LPUART1: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_A, 2,  PINCFG_OUTPUT | PINCFG_AF_12);  // LPUART1.TX
  hwpinctrl.PinSetup(PORTNUM_A, 3,  PINCFG_INPUT  | PINCFG_AF_12);  // LPUART1.RX
  conuart.Init(0x101); // 0x101 = LPUART1, 0x001 = USART1
#else
  // USART2: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_A, 2,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART2.TX
  hwpinctrl.PinSetup(PORTNUM_A, 3,  PINCFG_INPUT  | PINCFG_AF_7);  // USART2.RX
  conuart.Init(2);
#endif
}

#elif defined(BOARD_DISCOVERY_F746) || defined(BOARD_DISCOVERY_F750)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_I,  1, false);
  board_pins_init_leds();

  // turn off LCD backlight:
  hwpinctrl.PinSetup(PORTNUM_K,  3, PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PinSetup(PORTNUM_A, 9,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_B, 7,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1
}

#elif defined(BOARD_DISCOVERY_F429)

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_G, 13, false);
  pin_led[1].Assign(PORTNUM_G, 14, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A, 9,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_B, 7,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1
}

#elif defined(BOARD_DISCOVERY_WB5M)

void board_pins_init()
{
  // there is no simple MCU controlled LED here, so use the Arduino D0...
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C,  0, false);  // PC0 = Arduino D0
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_B, 6,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_B, 7,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1
}

#elif defined(BOARD_DISCOVERY_F072)

void board_pins_init()
{
  pin_led_count = 4;
  pin_led[0].Assign(PORTNUM_C, 6, false);
  pin_led[1].Assign(PORTNUM_C, 8, false);
  pin_led[2].Assign(PORTNUM_C, 9, false);
  pin_led[3].Assign(PORTNUM_C, 7, false);
  board_pins_init_leds();

  // USART1 - not availabe on the embedded debug probe
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1);  // USART1_RX
  conuart.Init(1);
}

#elif defined(BOARD_MIN_F103)  // = blue pill

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_PULLUP);  // USART1_RX, no AF here!
  conuart.Init(1);
}

#elif    defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO48_STM32F303) \
      || defined(BOARD_MIBO64_STM32F405) \
      || defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
}

#elif defined(BOARD_MIBO64_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1 - not availabe on the embedded debug probe
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1);  // USART1_RX
  conuart.Init(1);
}

#elif defined(BOARD_MIBO20_STM32F030) || defined(BOARD_MIBO20_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 1, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
}

#elif defined(BOARD_DEV_STM32F407VG)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_E, 0, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7);  // USART1_RX
  conuart.Init(1);
}

#elif defined(BOARD_DEV_STM32F407ZE)

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_F,  9, true);
  pin_led[1].Assign(PORTNUM_F, 10, true);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7);  // USART1_RX
  conuart.Init(1);
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();

  // UART - On the Arduino programmer interface
  hwpinctrl.PinSetup(0, 8, PINCFG_INPUT | PINCFG_AF_0);  // UART_RXD
  hwpinctrl.PinSetup(0, 9, PINCFG_OUTPUT | PINCFG_AF_0); // UART_TXD
  conuart.Init(0);  // UART
}

#elif defined(BOARD_MIBO64_ATSAM4S)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.Init(0);
}

#elif defined(BOARD_MIBO100_ATSAME70)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_D, 13, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  conuart.Init(0);
}

#elif defined(BOARD_XPLAINED_SAME70)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 8, false);
  board_pins_init_leds();

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

#elif defined(BOARD_MIBO64_ATSAME5X)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  // SERCOM0
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_OUTPUT | PINCFG_AF_3);  // PAD[0] = TX
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_INPUT  | PINCFG_AF_3);  // PAD[1] = RX
  conuart.Init(0);

  // SERCOM2
  //hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_AF_2);  // PAD[0] = TX
  //hwpinctrl.PinSetup(PORTNUM_A, 13, PINCFG_AF_2);  // PAD[1] = RX
  //conuart.Init(2);
}

#elif defined(BOARD_VERTIBO_A)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 29, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.baudrate = 115200;
  conuart.Init(0);
}

#elif defined(BOARD_ENEBO_A)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_A, 20, true);
  pin_led[1].Assign(PORTNUM_D, 14, true);
  pin_led[2].Assign(PORTNUM_D, 13, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.baudrate = 115200;
  conuart.Init(0);
}

// LPC

#elif defined(BOARD_XPRESSO_LPC54608)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(2,  2, true);
  pin_led[1].Assign(3,  3, true);
  pin_led[2].Assign(3, 14, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0, 30, PINCFG_OUTPUT | PINCFG_AF_1); // UART_TX:
  hwpinctrl.PinSetup(0, 29, PINCFG_INPUT  | PINCFG_AF_1); // UART_RX:
  conuart.Init(0);
}

#elif defined(BOARD_MIBO100_LPC546XX)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(1,  3, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0, 30, PINCFG_OUTPUT | PINCFG_AF_1); // UART_TX:
  hwpinctrl.PinSetup(0, 29, PINCFG_INPUT  | PINCFG_AF_1); // UART_RX:
  conuart.Init(0);
}

// RP

#elif defined(BOARD_RPI_PICO)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 25, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  conuart.Init(0);
}

#else
  #error "Define board_pins_init here"
#endif

#ifndef HEXNUM_DISPLAY

void board_show_hexnum(unsigned ahexnum)
{
  // nothing
}

#endif

