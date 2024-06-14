/*
 *  file:     board_pins.cpp (uart_mp_c0)
 *  brief:    UART Multi-Core Test Core0 Board pins
 *  date:     2024-06-14
 *  authors:  nvitya
*/

#include "board_pins.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 1;

TGpioPin  pin_led[MAX_LEDS] = { TGpioPin(), TGpioPin(), TGpioPin(), TGpioPin() };

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if SPI_SELF_FLASHING

  #include "hwspi.h"
  TSpiFlash  spiflash;

#endif

#if 0  // to use elif everywhere

// RP

#elif defined(BOARD_RPI_PICO)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 2, false);  // use the GP2 here
  board_pins_init_leds();

  hwpinctrl.PinSetup(0,  4, PINCFG_OUTPUT | PINCFG_AF_2); // UART1_TX:
  hwpinctrl.PinSetup(0,  5, PINCFG_INPUT  | PINCFG_AF_2); // UART1_RX:
  conuart.Init(1);
}

#elif defined(BOARD_RPI_PICOW)

THwQspi    fl_qspi;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 25, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0,  4, PINCFG_OUTPUT | PINCFG_AF_2); // UART1_TX:
  hwpinctrl.PinSetup(0,  5, PINCFG_INPUT  | PINCFG_AF_2); // UART1_RX:
  conuart.Init(1);
}

#else
  #error "Define board_pins_init here"
#endif

