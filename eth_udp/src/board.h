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

#if defined(BOARD_VRV100_443)

  // FPGA specific settings

  #define NV_APB_ETH_BASE_ADDR    0xF1010000
  #define HWETH_MAX_PACKET_SIZE   1360  // 3 packet fits into the default 4k packet memories

#endif

#endif /* BOARD_H_ */
