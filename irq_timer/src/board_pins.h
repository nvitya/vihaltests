/*
 *  file:     board_pins.h (irq timer)
 *  brief:    External Pin IRQ Test Board pins
 *  created:  2022-06-18
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

extern volatile uint32_t   g_mscounter;  // incremented in a periodic 1ms IRQ

#if SPI_SELF_FLASHING
  #include "spiflash.h"
  extern TSpiFlash  spiflash;
#endif

void board_pins_init();
void board_show_hexnum(unsigned ahexnum);

#endif /* SRC_BOARD_PINS_H_ */
