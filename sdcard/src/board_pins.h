/*
 *  file:     board_pins.h
 *  brief:    SDCARD Test Board pins
 *  date:     2022-11-12
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "board.h"
#include "hwpins.h"
#include "hwuart.h"

#define MAX_LEDS  4

extern TGpioPin  pin_led[MAX_LEDS];
extern unsigned  pin_led_count;

extern THwUart   conuart;  // console uart

#if SDCARD_SDMMC

  #include "hwsdmmc.h"
  #include "sdcard_sdmmc.h"

  extern THwSdmmc      sd_mmc;
  extern TSdCardSdmmc  sdcard;

#elif SDCARD_SPI

  #include "hwspi.h"
  #include "sdcard_spi.h"

  extern THwSpi        sd_spi;
  extern TSdCardSpi    sdcard;

#else
  #error "Define SDCARD_SPI=1 or SDCARD_SDMMC=1"
#endif

void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
