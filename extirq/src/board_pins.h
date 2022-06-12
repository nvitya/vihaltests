/*
 *  file:     board_pins.h (spiflash)
 *  brief:    External Pin IRQ Test Board pins
 *  created:  2021-10-29
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

#if SPI_SELF_FLASHING
  #include "spiflash.h"
  extern TSpiFlash  spiflash;
#endif

void board_pins_init();
void board_show_hexnum(unsigned ahexnum);

#endif /* SRC_BOARD_PINS_H_ */
