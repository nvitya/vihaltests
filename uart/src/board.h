/*
 *  file:     board.h (uart)
 *  brief:    Board / MCU definition
 *  version:  1.00
 *  date:     2021-10-02
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define BOARD_VRV153
#define BOARD_NAME "VRV153 on Cyclone IV"
#define MCU_VRV153
#define MCU_FIXED_SPEED  100000000

#define BOOTBLOCK_STADDR    0x00100000  // load the application from 1M

#endif /* BOARD_H_ */
