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

#define FL_SPI_USE_DMA      1  // 0 = normal polling, 1 = DMA, (qspi always uses DMA)

#endif /* BOARD_H_ */
