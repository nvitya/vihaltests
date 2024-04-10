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

#include "boards_builtin.h"

#if    defined(BOARD_DISCOVERY_F746) \
    || defined(BOARD_DISCOVERY_H747) \
    || defined(BOARD_DISCOVERY_F750) \
    || defined(BOARD_XPLAINED_SAME70) \
    || defined(BOARD_VERTIBO_A) \
    || defined(BOARD_ENEBO_A) \
    || defined(BOARD_DEV_STM32F407ZE) \
    || defined(BOARD_EVK_IMXRT1020) \
    || defined(BOARD_EVK_IMXRT1050) \
    || defined(BOARD_EVK_IMXRT1050A)

  #define SDCARD_SDMMC 1

#else
  #define SDCARD_SPI 1
#endif

#endif /* BOARD_H_ */
