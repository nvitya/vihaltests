/*
 *  file:     board_pins.h (i2c_eeprom)
 *  brief:    I2C EEPROM Test Board pins
 *  version:  1.00
 *  date:     2021-11-01
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hwi2c.h"
#include "hwdma.h"
#include "i2c_eeprom_app.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern TGpioPin   pin_i2c_irq;
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

void board_pins_init();
void board_init_i2c_slave_irq();
void board_show_hexnum(unsigned ahexnum);

#endif /* SRC_BOARD_PINS_H_ */
