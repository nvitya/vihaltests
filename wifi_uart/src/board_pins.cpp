/*
 *  file:     board_pins.cpp (wifi_uart)
 *  brief:    WIFI UART DEMO board specific initializations
 *  date:     2022-11-27
 *  authors:  nvitya
 *  notes:
 *    This example requires an ESP-01S WiFi module (ESP8266) with AT Firmware
*/

#include "board_pins.h"

unsigned  pin_led_count = 1;

TGpioPin  pin_led[MAX_LEDS] = { TGpioPin(), TGpioPin(), TGpioPin(), TGpioPin() };

THwUart   conuart;  // console uart

TEspWifiUart  wifi;

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

#elif defined(BOARD_LONGAN_NANO)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_C, 13, true);
  pin_led[1].Assign(PORTNUM_A,  1, true);
  pin_led[2].Assign(PORTNUM_A,  2, true);
  board_pins_init_leds();
}

bool TEspWifiUart::InitHw()
{
  hwpinctrl.PinSetup(PORTNUM_A,  9, PINCFG_OUTPUT | PINCFG_AF_0);
  hwpinctrl.PinSetup(PORTNUM_A, 10, PINCFG_INPUT  | PINCFG_AF_0);
  uart.Init(0); // USART0

  dma_tx.Init(0, 3);  // dma0, ch3 = USART3_TX
  dma_rx.Init(0, 4);  // dma0, ch4 = USART3_RX

  return true;
}

#elif defined(BOARD_WIO_LITE_RV)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A,  8, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART0_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_0);  // USART0_RX
  conuart.Init(0); // USART0
}

bool TEspWifiUart::InitHw()
{
  RCU->APB2EN |= RCU_APB2EN_AFEN;
  uint32_t tmp = AFIO->PCF0;
  tmp &= ~(7 << 24); // disable the NJRST
  tmp |=  (1 << 24); // anable debug but without NJRST, because it occupies the B4
  AFIO->PCF0 = tmp;

  // ESP-01S UART WiFi Module connections
  pin_rst.Assign(PORTNUM_B, 4, false);
  pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  //pin_en.Assign( PORTNUM_D, 3, false);

  hwpinctrl.PinSetup(PORTNUM_A,  2, PINCFG_OUTPUT | PINCFG_AF_0); // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3, PINCFG_INPUT  | PINCFG_AF_0 | PINCFG_PULLUP); // USART1_RX
  uart.Init(1); // USART1

  dma_tx.Init(0, 6);  // dma0, ch6 = USART1_TX
  dma_rx.Init(0, 5);  // dma0, ch5 = USART1_RX

  return true;
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B,  0, false);
  pin_led[1].Assign(PORTNUM_B,  7, false);
  pin_led[2].Assign(PORTNUM_B, 14, false);
  board_pins_init_leds();

  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX
  conuart.Init(3); // USART3
}

bool TEspWifiUart::InitHw()
{
  // ESP-01S UART WiFi Module connections
  pin_rst.Assign(PORTNUM_D, 4, false);
  pin_en.Assign( PORTNUM_D, 3, false);

  hwpinctrl.PinSetup(PORTNUM_D, 6,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART2_RX <- ESP-01S-TX !
  uart.Init(2); // USART2
  hwpinctrl.PinSetup(PORTNUM_D, 5,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART2_TX -> ESP-01S-RX !

  dma_tx.Init(1, 6, 4);  // dma1, stream6, ch4 = USART2_TX
  dma_rx.Init(1, 5, 4);  // dma1, stream5, ch4 = USART2_RX

  return true;
}

#elif defined(BOARD_MIN_F103)  // blue pill

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();
}

bool TUartComm::InitHw()
{
  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  // on this older silicon it must be configured as input and no Alternate Function:
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT | PINCFG_PULLUP);  // USART1_RX
  uart.Init(1);

  // the third parameter is ignored here
  dma_tx.Init(1, 4, 4);  // dma1, ch4 = USART1_TX
  dma_rx.Init(1, 5, 4);  // dma1, ch5 = USART1_RX

  return true;
}


#elif    defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO48_STM32F303) \
      || defined(BOARD_MIBO64_STM32F405) \
      || defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();
}

bool TUartComm::InitHw()
{
  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  uart.Init(1);

  dma_tx.Init(2, 7, 4);  // dma2, stream7, ch4 = USART1_TX
  dma_rx.Init(2, 5, 4);  // dma2, stream5, ch4 = USART1_RX

  return true;
}


// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();
}

bool TUartComm::InitHw()
{
#if 1
  // UART - On the Arduino programmer interface
  hwpinctrl.PinSetup(PORTNUM_A, 8, PINCFG_INPUT | PINCFG_AF_0);  // UART_RXD
  hwpinctrl.PinSetup(PORTNUM_A, 9, PINCFG_OUTPUT | PINCFG_AF_0); // UART_TXD
  uart.Init(0);  // UART

  // peripheral DMA only!
  // the dma_rx.Remaining() must be called reqularly in order to work properly
  // (it is called normally regularly for circular mode)
  uart.PdmaInit(true,  &dma_tx);
  uart.PdmaInit(false, &dma_rx);
#else
  // USART0 - D18=TX1, D19=RX1
  hwpinctrl.PinSetup(PORTNUM_A, 10, PINCFG_INPUT | PINCFG_AF_0);  // USART0_RXD
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_OUTPUT | PINCFG_AF_0); // USART0_TXD
  uart.Init(0x100);  // USART0 (+ 0x100 = use the USART unit instead of the UART)

  // DMAC
  dma_tx.Init(0, 11);  // perid: 11 = USART0_TX
  dma_rx.Init(1, 12);  // perid: 12 = USART0_RX
#endif

  return true;
}

#elif defined(BOARD_XPLAINED_SAME70)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 8, false);
  board_pins_init_leds();
}

bool TUartComm::InitHw()
{
  // USART1 - EDBG
  hwpinctrl.PinSetup(PORTNUM_A, 21, PINCFG_INPUT | PINCFG_AF_0);  // USART1_RXD
  MATRIX->CCFG_SYSIO |= (1 << 4); // select PB4 instead of TDI !!!!!!!!!
  hwpinctrl.PinSetup(PORTNUM_B,  4, PINCFG_OUTPUT | PINCFG_AF_3); // USART1_TXD
  uart.Init(0x101); // USART1 (+ 0x100 = use the USART unit instead of the UART)

  // the DMA channel (first parameter) can be chosen freely
  dma_tx.Init(0,  9);  // perid:  9 = USART1_TX
  dma_rx.Init(1, 10);  // perid: 10 = USART1_RX

  return true;
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();
}

bool TUartComm::InitHw()
{
  // SERCOM0
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_OUTPUT | PINCFG_AF_3);  // PAD[0] = TX
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_INPUT  | PINCFG_AF_3);  // PAD[1] = RX
  uart.Init(0);

  // the DMA channel (first parameter) can be chosen freely
  // the Trigger sources (second parameter) can be found in the DMA datasheet
  // at the TRIGSRC[7:0] field definition
  dma_tx.Init(0, 0x05);  // 0x05 = SERCOM0_TX
  dma_rx.Init(1, 0x04);  // 0x04 = SERCOM0_RX

  return true;
}

// RP

#elif defined(BOARD_RPI_PICO)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 25, false);
  board_pins_init_leds();
}

bool TUartComm::InitHw()
{
  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  uart.Init(0);

  // the DMA channel (first parameter) can be chosen freely
  // the Trigger sources (second parameter) can be found in the chapter "2.5.3.1 System DREQ Table"
  dma_tx.Init(0, DREQ_UART0_TX);  // 20 = UART0_TX
  dma_rx.Init(1, DREQ_UART0_RX);  // 21 = UART0_RX

  return true;
}


#else
  #error "Define board_pins_init here"
#endif

#ifndef HEXNUM_DISPLAY

void board_show_hexnum(unsigned ahexnum)
{
  // nothing
}

#endif

