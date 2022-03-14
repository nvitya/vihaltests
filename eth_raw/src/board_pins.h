/*
 *  file:     board_pins.h
 *  brief:    USB HID Test Board pins
 *  version:  1.00
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hweth.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart


#define ETH_RX_PACKETS  16
#define ETH_TX_PACKETS   4

extern THwEth    eth;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
