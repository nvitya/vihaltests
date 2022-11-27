/*
 *  file:     board_pins.cpp (wifi_uart)
 *  brief:    WIFI UART DEMO board specific initializations
 *  date:     2022-11-27
 *  authors:  nvitya
 *  notes:
 *    This example requires an ESP-01S WiFi module (ESP8266) with AT Firmware
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "espwifi_uart.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;

extern TEspWifiUart  wifi;

void board_pins_init();
void board_show_hexnum(unsigned ahexnum);

#endif /* SRC_BOARD_PINS_H_ */
