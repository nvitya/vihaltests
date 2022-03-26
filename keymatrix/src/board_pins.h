/*
 *  file:     board_pins.h
 *  brief:    Keyboard Matrix Test board pins
 *  date:     2022-03-26
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "keymatrix.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

extern TKeyMatrix keymatrix;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
