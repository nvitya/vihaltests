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
#include "hwi2c.h"
#include "hwspi.h"
#include "charlcd_i2c.h"
#include "charlcd_bb4.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

extern THwI2c     i2c;

void board_pins_init();

#if defined(DISP_16X2_I2C)

  extern TCharLcd_i2c   disp;

#elif defined(DISP_16X2_BB4)

  extern TCharLcd_bb4   disp;

#else
  #error "undefined inerface type"
#endif

#endif /* SRC_BOARD_PINS_H_ */
