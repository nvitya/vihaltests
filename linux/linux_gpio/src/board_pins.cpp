/*
 *  file:     board_pins.cpp
 *  brief:    Application Board pins
 *  date:     2024-07-03
 *  authors:  nvitya
*/

#include "board_pins.h"

unsigned  pin_led_count = 1;

TGpioPin  pin_led[MAX_LEDS] = { TGpioPin(), TGpioPin(), TGpioPin(), TGpioPin() };

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
// ARM Cortex-A
//-------------------------------------------------------------------------------

#elif defined(BOARD_BEAGLEB_BLACK)

void board_pins_init()
{
  pin_led_count = 4;
  pin_led[0].Assign(1, 21, false);
  pin_led[1].Assign(1, 22, false);
  pin_led[2].Assign(1, 23, false);
  pin_led[3].Assign(1, 24, false);
  board_pins_init_leds();
}

#else
  #error "Define board_pins_init here"
#endif


