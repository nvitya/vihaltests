/*
 *  file:     board_pins.h
 *  brief:    Stepper Motor Test board pins
 *  date:     2022-03-26
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "stepper_4u.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

extern TStepper_4u  stepper;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
