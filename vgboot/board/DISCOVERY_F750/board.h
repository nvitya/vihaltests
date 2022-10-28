/*
 *  file:     board.h
 *  brief:    Board definition for the DISCOVERY F750
 *  created:  2022-10-28
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define PRINTF_SUPPORT_FLOAT

#define BOARD_NAME "STM32F750 Discovery"
#define MCU_STM32F750N8
#define EXTERNAL_XTAL_HZ   25000000

#define MAX_CLOCK_SPEED   200000000  // less than the maximum 216 MHz

#define CONUART_UART_SPEED   115200
#define HAS_SDRAM                 1

#define QSPI_SPEED         50000000
#define QSPI_LINE_COUNT           4

#define FLADDR_APPLICATION        0  // app starts at the position 0 (app header first)

#endif /* BOARD_H_ */
