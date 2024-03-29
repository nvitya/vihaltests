/*
 *  file:     board_pins.cpp (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"

THwUart   conuart;  // console uart

unsigned  pin_led_count = 1;

// common LED array

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
// Risc-V: RV32I
//-------------------------------------------------------------------------------

#elif defined(BOARD_NANO_ESP32C3)

void board_pins_init()
{
  // warning the board has only a WS2812B intelligent RGB led (single pin time pattern control)
  pin_led_count = 1;
  pin_led[0].Assign(0, 8, true);

  board_pins_init_leds();
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

#elif defined(BOARD_WEMOS_C3MINI)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 7, false);
  board_pins_init_leds();

  // for now, external UART adapter is required

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

  //hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);
}

#else
  #error "Define board_pins_init here"
#endif

