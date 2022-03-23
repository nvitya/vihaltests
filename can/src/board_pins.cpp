/*
 *  file:     board_pins.cpp (can)
 *  brief:    Internal Flash Write Test Board pins (just leds and uart console)
 *  version:  1.00
 *  date:     2021-11-07
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "clockcnt.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 1;
int       can_devnum = 0;

TGpioPin  pin_led[MAX_LEDS] =
{
  TGpioPin(),
  TGpioPin(),
  TGpioPin(),
  TGpioPin()
};

THwCan     can;

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
    pin_led[n].Set0();
  }
}

#if 0  // to use elif everywhere

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

  // CAN PINS
  hwpinctrl.PinSetup(PORTNUM_D,  0, PINCFG_INPUT | PINCFG_AF_9);  // CAN1 RX
  hwpinctrl.PinSetup(PORTNUM_D,  1, PINCFG_INPUT | PINCFG_AF_9);  // CAN1 TX
  can_devnum = 1;
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

  // CAN

  // Set CAN remap!

  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // enable AFIO clock

  uint32_t tmp;
  tmp = AFIO->MAPR;
  tmp &= ~(3 << 13);
  tmp |=  (2 << 13);  // remap CAN pints from A11, A12 to B8, B9
  AFIO->MAPR = tmp;

  hwpinctrl.PinSetup(PORTNUM_B, 8, PINCFG_INPUT | PINCFG_AF_0);  // CANRX
  hwpinctrl.PinSetup(PORTNUM_B, 9, PINCFG_INPUT | PINCFG_AF_0);  // CANTX
  can_devnum = 1;
}

#elif defined(BOARD_MIBO48_STM32F303)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // CAN PINS
  hwpinctrl.PinSetup(PORTNUM_B,  8, PINCFG_INPUT | PINCFG_AF_9);  // CAN RX
  hwpinctrl.PinSetup(PORTNUM_B,  9, PINCFG_INPUT | PINCFG_AF_9);  // CAN TX
  can_devnum = 1;
}

#elif defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // CAN PINS
  hwpinctrl.PinSetup(PORTNUM_B,  8, PINCFG_INPUT | PINCFG_AF_9);  // CAN1 RX
  hwpinctrl.PinSetup(PORTNUM_B,  9, PINCFG_INPUT | PINCFG_AF_9);  // CAN1 TX
  can_devnum = 1;
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

  // dedicated HS USB pins, no pin setup required
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

  // CAN
  hwpinctrl.PinSetup(PORTNUM_B, 2, PINCFG_AF_A); // MCAN0_TX
  hwpinctrl.PinSetup(PORTNUM_B, 3, PINCFG_AF_A); // MCAN0_RX
  can_devnum = 0;
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

  // CAN
  hwpinctrl.PinSetup(PORTNUM_B, 2, PINCFG_AF_A); // MCAN0_TX
  hwpinctrl.PinSetup(PORTNUM_B, 3, PINCFG_AF_A); // MCAN0_RX
  can_devnum = 0;
}

#elif defined(BOARD_ECCAN_E51)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B, 31, true);  // blue
  pin_led[1].Assign(PORTNUM_A, 21, true);  // green
  pin_led[2].Assign(PORTNUM_A, 20, true);  // red
  board_pins_init_leds();

  // SERCOM1
  hwpinctrl.PinSetup(PORTNUM_A, 0, PINCFG_OUTPUT | PINCFG_AF_D);  // SERCOM1/PAD0 = TX
  hwpinctrl.PinSetup(PORTNUM_A, 1, PINCFG_INPUT  | PINCFG_AF_D);  // SERCOM1/PAD1 = RX
  conuart.Init(1);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 24, PINCFG_AF_7);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_AF_7);  // USB DP

  // CAN
  hwpinctrl.PinSetup(PORTNUM_A, 22, PINCFG_AF_I); // CAN0_TX
  hwpinctrl.PinSetup(PORTNUM_A, 23, PINCFG_AF_I); // CAN0_RX
  can_devnum = 0;

  //hwpinctrl.PinSetup(PORTNUM_B, 14, PINCFG_AF_H); // CAN1_TX
  //hwpinctrl.PinSetup(PORTNUM_B, 15, PINCFG_AF_H); // CAN1_RX
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

  // CAN
  hwpinctrl.PinSetup(PORTNUM_A, 22, PINCFG_AF_I); // CAN0_TX
  hwpinctrl.PinSetup(PORTNUM_A, 23, PINCFG_AF_I); // CAN0_RX
  can_devnum = 0;
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

  // CAN
  hwpinctrl.PinSetup(PORTNUM_B, 2, PINCFG_AF_A); // MCAN0_TX
  hwpinctrl.PinSetup(PORTNUM_B, 3, PINCFG_AF_A); // MCAN0_RX
  can_devnum = 0;
}

#else
  #error "Define board_pins_init here"
#endif

