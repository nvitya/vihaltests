/*
 * board_pins.h
 *
 *  Created on: Jul 3, 2024
 *      Author: vitya
 */

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"

#define MAX_LEDS  4

extern TGpioPin  pin_led[MAX_LEDS];
extern unsigned  pin_led_count;

void board_pins_init();
void board_display_init();

#define DISP_SPI
#define SPI_DISPLAY_WIDTH    128  // for 128x160

#if defined(BOARD_MILKV_DUO)

	#include "tftlcd_sg_spinor.h"

	extern TTftLcd_sg_spinor  disp;

#else

  #include "tftlcd_spi.h"

  extern TTftLcd_spi  disp;

#endif

#endif /* SRC_BOARD_PINS_H_ */
