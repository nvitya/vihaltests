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

#ifdef MCUF_VRV100
  #define SELF_FLASHING 1
#endif

#ifndef SELF_FLASHING
  #define SELF_FLASHING 0
#endif

#endif /* BOARD_H_ */
