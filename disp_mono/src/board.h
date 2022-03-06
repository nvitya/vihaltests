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

  #define DISP_SEL  3
  //   0 = 3" 128 x  64 with UC1701
  //   1 = 5" 256 x 160 with ST75256
  //   2 = 4" 256 x  64 with ST75256
  //   3 = 3" 192 x  64 with ST75256

  #if 0 == DISP_SEL

    #define DISPLAY_WIDTH    128
    #define DISPLAY_HEIGHT    64
    #define DISPLAY_CTRL     MLCD_CTRL_UC1701

    #define REFRESH_TIME_MS  250  // ~4 FPS so that the character changes are still visible

  #elif 1 == DISP_SEL // 5" 256 x 160 with ST75256

    #define DISPLAY_WIDTH       256
    #define DISPLAY_HEIGHT      160
    #define DISPLAY_CONTRAST  0x170
    #define DISPLAY_CTRL     MLCD_CTRL_ST75256

    #define REFRESH_TIME_MS  125  // ~8 FPS so that the character changes are still visible

  #elif 2 == DISP_SEL // 4" 256 x  64 with ST75256

    #define DISPLAY_WIDTH       256
    #define DISPLAY_HEIGHT       64
    #define DISPLAY_CONTRAST  0x0B8
    #define DISPLAY_CTRL     MLCD_CTRL_ST75256

    #define REFRESH_TIME_MS  125  // ~8 FPS so that the character changes are still visible

  #elif 3 == DISP_SEL // 3" 192 x  64 with ST75256

    #define DISPLAY_WIDTH       192
    #define DISPLAY_HEIGHT       64
    #define DISPLAY_CONTRAST   0x00
    #define DISPLAY_CTRL     MLCD_CTRL_UC1609
    //#define DISPLAY_CTRL     MLCD_CTRL_UC1701
    #define DISPLAY_ROTATION      0

    #define REFRESH_TIME_MS  125  // ~8 FPS so that the character changes are still visible

  #else
    #error "Invalid display selection"
  #endif

#else
  #error "Define the display type"
#endif


#include "boards_builtin.h"

#endif /* BOARD_H_ */
