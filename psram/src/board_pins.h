/*
 *  file:     board_pins.h
 *  brief:    SPI PSRAM Test Board pins
 *  date:     2022-03-23
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hwspi.h"
#include "hwqspi.h"

#define MAX_LEDS  4

extern TGpioPin  pin_led[MAX_LEDS];
extern unsigned  pin_led_count;

extern THwUart   conuart;  // console uart

extern THwSpi    fl_spi;
extern THwQspi   fl_qspi;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
