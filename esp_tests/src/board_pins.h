/*
 *  file:     board_pins.h (blinkled)
 *  brief:    Blinking LEDS Test Board pins
 *  version:  1.00
 *  date:     2021-10-30
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"

extern THwUart    conuart;  // console uart

#define MAX_LEDS  4

extern TGpioPin  pin_led[MAX_LEDS];
extern unsigned  pin_led_count;

void board_show_hexnum(unsigned ahexnum); // for FPGA-based boards
void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
