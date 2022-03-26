/*
 *  file:     board_pins.h (i2c_fram)
 *  brief:    I2C FRAM Test Board pins
 *  version:  1.00
 *  date:     2022-03-26
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hwi2c.h"
#include "hwdma.h"

#define MAX_LEDS  4

extern THwI2c         i2c;
extern THwDmaChannel  i2c_txdma;
extern THwDmaChannel  i2c_rxdma;

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
