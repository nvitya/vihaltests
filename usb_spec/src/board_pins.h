/*
 *  file:     board_pins.h
 *  brief:    USB Special Device Board pins
 *  version:  1.00
 *  date:     2022-03-23
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

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
