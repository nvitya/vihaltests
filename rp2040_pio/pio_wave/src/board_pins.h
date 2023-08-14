/*
 *  file:     board_pins.h (spiflash)
 *  brief:    PIO Test Board pins
 *  date:     2023-08-13
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hwqspi.h"
#include "spiflash.h"

extern TGpioPin   pin_led;
extern THwUart    conuart;  // console uart

extern THwQspi    fl_qspi;
extern TSpiFlash  spiflash;

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
