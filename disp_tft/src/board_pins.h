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

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

void board_pins_init();

#if 0

#elif defined(BOARD_DISCOVERY_F746) || defined(BOARD_DISCOVERY_F750)

#include "framebuffer16.h"
extern TFrameBuffer16  disp;

#elif defined(BOARD_DEV_STM32F407ZE)

#include "tftlcd_mm16_f407ze.h"
extern TTftLcd_mm16_F407ZE  disp;

#elif defined(BOARD_ARDUINO_DUE)

#include "tftlcd_gp16_due.h"
extern TTftLcd_gp16_due  disp;

#else
  #error "Board not defined."
#endif

#endif /* SRC_BOARD_PINS_H_ */
