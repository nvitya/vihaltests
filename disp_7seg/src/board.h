/*
 *  file:     board.h
 *  brief:    Board / MCU definition / special settings
 *  created:  2022-11-18
 *  authors:  nvitya
 *  notes:
 *    the platform.h includes this very early, it must at least define the used MCU
*/

#ifndef BOARD_H_
#define BOARD_H_

#define DISP_SEL    0
// 0 = LED & KEY module with TM1638 controller (8 digits + 8 leds + 8 keys)
// 1 = Simple 4-digit Clock Display with TM1637 controller
// 2 = 8-digit display with MAX7219
// 3 = 4-digit display with 2x74HC595, (manual multiplexing, only one digit is visible when MCU is stopped)

// Define the MCU trough the built-in boards
#include "boards_builtin.h"

#endif /* BOARD_H_ */
