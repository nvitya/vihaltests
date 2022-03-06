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

#define PRINTF_SUPPORT_FLOAT

#define VERTIBO_A_LCD_GPIO     0
#define VERTIBO_A_LCD_800x480  0

//#define SPI_DISPLAY_WIDTH    128  // for 128x160
#define SPI_DISPLAY_WIDTH    240  // for 240x320

#include "boards_builtin.h"

#endif /* BOARD_H_ */
