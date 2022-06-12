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
#include "ledandkey.h"
#include "timedisp_tm1637.h"
#include "max7219.h"
#include "d7s_595.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

#if SPI_SELF_FLASHING
  #include "spiflash.h"
  extern TSpiFlash  spiflash;
#endif

void board_pins_init();

#if 0 == DISP_SEL

  extern TLedAndKey  disp;

#elif 1 == DISP_SEL

  extern TTimeDisp_tm1637   disp;

#elif 2 == DISP_SEL

  extern TMax7219  disp;

#elif 3 == DISP_SEL

  extern Td7s_595  disp;

#else
  #error "undefined display type"
#endif

#endif /* SRC_BOARD_PINS_H_ */
