#include "board_pins.h"

THwUart   conuart;  // console uart

void board_pins_init()
{
  hwpinctrl.PinSetup(PORTNUM_B, 6,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_B, 7,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1
}
