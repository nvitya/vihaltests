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

//#define DISP_I2C_OLED
#define DISP_SPI_MONO

#if defined(DISP_I2C_OLED)
  #define OLED_DISPLAY_WIDTH    128
  #define OLED_DISPLAY_HEIGHT    64
  //#define OLED_DISPLAY_HEIGHT  32

  #define REFRESH_TIME_MS  17  // ~60 FPS

#elif defined(DISP_SPI_MONO)
  #define DISPLAY_WIDTH    128
  #define DISPLAY_HEIGHT    64

  #define REFRESH_TIME_MS   250  // ~4 FPS so that the character changes are still visible

#else
  #error "Define the display type"
#endif


#include "boards_builtin.h"

#endif /* BOARD_H_ */
