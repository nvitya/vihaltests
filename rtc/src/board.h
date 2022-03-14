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

#define BOARD_NAME "RolloCon"
#define MCU_STM32WB55VG
#define EXTERNAL_XTAL_HZ   32000000

#endif /* BOARD_H_ */
