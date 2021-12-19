/*
 *  file:     board_pins.cpp (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"

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
}

#elif defined(MCUF_VRV100)

#define HEXNUM_DISPLAY
void board_show_hexnum(unsigned ahexnum)
{
  volatile uint32_t *  hexnum = (volatile uint32_t *)0xF1000000;
  *hexnum = ahexnum;
}

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 0, false);

  board_show_hexnum(0);
  board_pins_init_leds();
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B,  0, false);
  pin_led[1].Assign(PORTNUM_B,  7, false);
  pin_led[2].Assign(PORTNUM_B, 14, false);
  board_pins_init_leds();
}

#elif defined(BOARD_NUCLEO_H723)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B,  0, false);  // PB0 or PA5
  pin_led[1].Assign(PORTNUM_E,  1, false);
  pin_led[2].Assign(PORTNUM_B, 14, false);
  board_pins_init_leds();
}

#elif defined(BOARD_NUCLEO_G474RE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A,  5, false);
  board_pins_init_leds();
}

#elif defined(BOARD_NUCLEO_G431KB)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B,  8, false);
  board_pins_init_leds();
}

#elif defined(BOARD_DISCOVERY_F746)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_I,  1, false);
  board_pins_init_leds();
}

#elif defined(BOARD_DISCOVERY_F429)

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_G, 13, false);
  pin_led[1].Assign(PORTNUM_G, 14, false);
  board_pins_init_leds();
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
}

// STM32

#elif    defined(BOARD_MIN_F103) \
      || defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO48_STM32F303) \
   		|| defined(BOARD_MIBO64_STM32F405) \
		  || defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();
}

#elif defined(BOARD_MIBO64_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();
}

#elif defined(BOARD_MIBO64_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();
}

#elif defined(BOARD_MIBO20_STM32F030) || defined(BOARD_MIBO20_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 1, false);
  board_pins_init_leds();
}

#elif defined(BOARD_DEV_STM32F407VG)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_E, 0, false);
  board_pins_init_leds();
}

#elif defined(BOARD_DEV_STM32F407ZE)

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_F,  9, true);
  pin_led[1].Assign(PORTNUM_F, 10, true);
  board_pins_init_leds();
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();
}

#elif defined(BOARD_MIBO64_ATSAM4S)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();
}

#elif defined(BOARD_MIBO100_ATSAME70)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_D, 13, false);
  board_pins_init_leds();
}

#elif defined(BOARD_XPLAINED_SAME70)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 8, false);
  board_pins_init_leds();
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();
}

#elif defined(BOARD_VERTIBO_A)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 29, false);
  board_pins_init_leds();
}

#elif defined(BOARD_ENEBO_A)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_A, 20, true);
  pin_led[1].Assign(PORTNUM_D, 14, true);
  pin_led[2].Assign(PORTNUM_D, 13, true);
  board_pins_init_leds();
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
}

#elif defined(BOARD_MIBO100_LPC546XX)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(1,  3, false);
  board_pins_init_leds();
}

// RP

#elif defined(BOARD_RPI_PICO)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 25, false);
  board_pins_init_leds();
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

