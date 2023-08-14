/*
 *  file:     board_pins.cpp (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "platform.h"
#include "board_pins.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 0;

THwQspi    fl_qspi;
TSpiFlash  spiflash;

#if 0 // to use elif everywhere

// RP

#elif defined(BOARD_RPI_PICO)

TGpioPin  pin_led(0, 25, false);

void board_pins_init()
{
  pin_led.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  conuart.Init(0);

  // because of the transfers are unidirectional the same DMA channel can be used here:
  fl_qspi.txdmachannel = 7;
  fl_qspi.rxdmachannel = 7;
  // for read speeds over 24 MHz dual or quad mode is required.
  // the writes are forced to single line mode (SSS) because the RP does not support SSM mode at write
  fl_qspi.multi_line_count = 4;
  fl_qspi.speed = 32000000;
  fl_qspi.Init();

  spiflash.qspi = &fl_qspi;
  spiflash.has4kerase = true;
  spiflash.Init();
}

#else
  #error "Define board_pins_init here"
#endif

