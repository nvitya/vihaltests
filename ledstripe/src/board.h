/*
 *  file:     board.h
 *  brief:    Board / MCU definition
 *  created:  2021-11-18
 *  authors:  nvitya
 *  notes:
 *    the platform.h includes this very early, it must at least define the used MCU
*/

#ifndef BOARD_H_
#define BOARD_H_

#include "boards_builtin.h"

#define LEDSTRIPE_SEL 0

#if 0 == LEDSTRIPE_SEL

  #define LEDSTRIPE_WS2813   1

  // The MCU Frequency must be dividible by 2.4 MHz for proper SPI timing !
  #if defined(BOARD_MIN_F401)
    #define MCU_CLOCK_SPEED   72000000
  #endif

  #define SPI_SPEED  2400000
  #define LED_COUNT  30

#elif 1 == LEDSTRIPE_SEL

  #define LEDSTRIPE_APA102   1
  #define SPI_SPEED  4000000
  #define LED_COUNT  75

#else
  #error "wrong configuration"
#endif

#if LEDSTRIPE_WS2813
  #define LED_DATABUF_SIZE ((LED_COUNT + 16) * 9)
#elif LEDSTRIPE_APA102
  #define LED_DATABUF_SIZE (LED_COUNT * 4 + 8)
#else
  #error "wrong configuration"
#endif

#endif /* BOARD_H_ */
