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

#define DISP_I2C_OLED
//#define DISP_SPI_MONO
//#define DISP_BITBANG

#if defined(DISP_I2C_OLED)
  #define DISPLAY_WIDTH    128
  #define DISPLAY_HEIGHT    64
  //#define DISPLAY_HEIGHT  32

  #define REFRESH_TIME_MS  17  // ~60 FPS

#elif defined(DISP_SPI_MONO)

  #define DISP_SEL  2
  //   0 = 3" 128 x  64 with UC1701
  //   1 = 5" 256 x 160 with ST75256
  //   2 = 4" 256 x  64 with ST75256
  //   3 = 3" 192 x  64 with UC1609
  //   4 = 1"  84 x  48 with NOKIA5110

  #if 0 == DISP_SEL

    #define DISPLAY_WIDTH    128
    #define DISPLAY_HEIGHT    64
    #define DISPLAY_CTRL     MLCD_CTRL_UC1701
    #define DISPLAY_ROTATION      0

    #define REFRESH_TIME_MS  250  // ~4 FPS so that the character changes are still visible

  #elif 1 == DISP_SEL // 5" 256 x 160 with ST75256

    #define DISPLAY_WIDTH       256
    #define DISPLAY_HEIGHT      160
    #define DISPLAY_CONTRAST  0x170
    #define DISPLAY_CTRL     MLCD_CTRL_ST75256
    #define DISPLAY_ROTATION      0

    #define REFRESH_TIME_MS  125  // ~8 FPS so that the character changes are still visible

  #elif 2 == DISP_SEL // 4" 256 x  64 with ST75256

    #define DISPLAY_WIDTH       256
    #define DISPLAY_HEIGHT       64
    #define DISPLAY_CONTRAST  0x0B8
    #define DISPLAY_CTRL     MLCD_CTRL_ST75256
    #define DISPLAY_ROTATION      2

    #define REFRESH_TIME_MS  125  // ~8 FPS so that the character changes are still visible

  #elif 3 == DISP_SEL // 3" 192 x  64 with UC1609

    #define DISPLAY_WIDTH       192
    #define DISPLAY_HEIGHT       64
    #define DISPLAY_CONTRAST   0x00
    #define DISPLAY_CTRL     MLCD_CTRL_UC1609
    #define DISPLAY_ROTATION      0

    #define REFRESH_TIME_MS  125  // ~8 FPS so that the character changes are still visible

  #elif 4 == DISP_SEL // 1"  84 x  48 with NOKIA5110

    #define DISPLAY_WIDTH        84
    #define DISPLAY_HEIGHT       48
    //#define DISPLAY_CONTRAST   0x00
    #define DISPLAY_CTRL     MLCD_CTRL_NOKIA5110 // = MLCD_CTRL_PCD8544
    #define DISPLAY_ROTATION      0

    #define REFRESH_TIME_MS      50  // ~20 FPS so that the character changes are still visible

  #else
    #error "Invalid display selection"
  #endif

#elif defined(DISP_BITBANG)

  #define DISPLAY_WIDTH     96
  #define DISPLAY_HEIGHT    68
  #define DISPLAY_CTRL     MLCD_CTRL_HX1230
  #define DISPLAY_ROTATION   0

  #define REFRESH_TIME_MS  250  // ~4 FPS so that the character changes are still visible


#else
  #error "Define the display type"
#endif


#include "boards_builtin.h"

#endif /* BOARD_H_ */
