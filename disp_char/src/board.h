/*
 *  file:     board.h
 *  brief:    MCU definition / Board Settings
 *  created:  2022-03-09
 *  authors:  nvitya
 *  notes:
 *    the platform.h includes this very early, it must at least define the used MCU
*/

#ifndef BOARD_H_
#define BOARD_H_

// select only one DISP_xxx here:

#define DISP_16X2_I2C  // 5V Module with I2C port extender
                       // with built in pull-ups (no external pullups required)
                       // the I2C can be driven with 3.3V

//#define DISP_16X2_BB4  // 5V module, level shifter required

#if defined(DISP_16X2_I2C) || defined(DISP_16X2_BB4)

#define DISPLAY_WIDTH     16
#define DISPLAY_HEIGHT     2

#else
  #error "Define the display type"
#endif


#include "boards_builtin.h"

#endif /* BOARD_H_ */
