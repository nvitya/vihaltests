/*
 *  file:     board_pins.h (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
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

extern THwSpi    sd_spi;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
