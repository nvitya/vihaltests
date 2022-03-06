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
#include "oleddisp_i2c.h"
#include "monolcd_spi.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

void board_pins_init();

#ifdef DISP_I2C_OLED

  extern THwI2c   i2c;
  extern TOledDisp_i2c   disp;

#elif defined(DISP_SPI_MONO)

  extern THwSpi          spi;
  extern TMonoLcd_spi    disp;

#else

  #error "undefined inerface type"

#endif

#endif /* SRC_BOARD_PINS_H_ */
