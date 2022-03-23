/*
 *  file:     board_pins.h
 *  brief:    USB HID Test Board pins
 *  version:  1.00
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "hwdma.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

extern THwUart              usbuart;
extern THwDmaChannel        usbuart_dma_tx;
extern THwDmaChannel        usbuart_dma_rx;


void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
