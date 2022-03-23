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

#if defined(BOARD_ECCAN_E51)

  #define BOARD_NAME "ECCAN-E51 with ATSAME51"
  #define MCU_ATSAME51J18
  #define EXTERNAL_XTAL_HZ   12000000

#else

#include "boards_builtin.h"

#endif

#endif /* BOARD_H_ */
