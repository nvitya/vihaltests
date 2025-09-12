/*
 * board_pins.h
 *
 *  Created on: Jul 3, 2024
 *      Author: vitya
 */

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hwpwm.h"
#include "hwspi.h"

#define MAX_LEDS  4

extern TGpioPin  pin_led[MAX_LEDS];
extern unsigned  pin_led_count;

extern THwUart   conuart;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
