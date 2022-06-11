/*
 *  file:     board_pins.cpp (intflash)
 *  brief:    Internal Flash Write Test Board pins (just leds and uart console)
 *  version:  1.00
 *  date:     2021-11-07
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "clockcnt.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 1;

TGpioPin  pin_led[MAX_LEDS] =
{
  TGpioPin(),
  TGpioPin(),
  TGpioPin(),
  TGpioPin()
};

/* NOTE:
     for direct GPIO pin definitions is simpler to define with port and pin number:

       TGpioPin  pin_mygpio(PORTNUM_C, 13, false);

     but don't forget to initialize it in the setup code:

       pin_mygpio.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
*/

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

THwUart              usbuart;
THwDmaChannel        usbuart_dma_tx;
THwDmaChannel        usbuart_dma_rx;


#if 0  // to use elif everywhere

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
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  conuart.Init(3); // USART3

  // USB_OTG_FS PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB_OTG_FS DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB_OTG_FS DP

  #error "define USB-UART pins"
}

#elif defined(BOARD_NUCLEO_H723)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_B,  0, false);  // PB0 or PA5
  pin_led[1].Assign(PORTNUM_E,  1, false);
  pin_led[2].Assign(PORTNUM_B, 14, false);
  board_pins_init_leds();

  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  conuart.Init(3); // USART3

  // USB_OTG_FS PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB_OTG_FS DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB_OTG_FS DP

  #error "define USB-UART pins"
}


#elif defined(BOARD_DISCOVERY_F072)

void board_pins_init()
{
  pin_led_count = 4;
  pin_led[0].Assign(PORTNUM_C, 6, false);
  pin_led[1].Assign(PORTNUM_C, 8, false);
  pin_led[2].Assign(PORTNUM_C, 9, false);
  pin_led[3].Assign(PORTNUM_C, 7, false);
  board_pins_init_leds();

  // USART1 - not availabe on the embedded debug probe
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1);  // USART1_RX
  conuart.Init(1);

  #error "define USB-UART pins"
}

#elif defined(BOARD_MIN_F103)  // = blue pill

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_PULLUP);  // USART1_RX, no AF here!
  conuart.Init(1);

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART2_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT | PINCFG_PULLUP);  // USART2_RX - do not set AF for inputs!
  usbuart.Init(2);

  usbuart_dma_tx.Init(1, 7, 2); // USART2_TX
  usbuart_dma_rx.Init(1, 6, 2); // USART2_RX

  // USB RE-CONNECT
  // The Blue Pill has a fix external pull-up on the USB D+ = PA12, which always signalizes a connected device
  // in order to reinit the device upon restart we pull this down:
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_OUTPUT | PINCFG_OPENDRAIN | PINCFG_GPIO_INIT_0);
  delay_us(10000); // 10 ms

  // Setup USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_14 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_14 | PINCFG_SPEED_FAST);  // USB DP
}

#elif defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_14 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_14 | PINCFG_SPEED_FAST);  // USB DP

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART2_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART2_RX
  usbuart.Init(2);

  usbuart_dma_tx.Init(1, 7, 27); // USART2_TX
  usbuart_dma_rx.Init(1, 6, 26); // USART2_RX
}

#elif defined(BOARD_MIBO48_STM32F303)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_14 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_14 | PINCFG_SPEED_FAST);  // USB DP

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART2_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART2_RX
  usbuart.Init(2);

  usbuart_dma_tx.Init(1, 7, 2); // USART2_TX
  usbuart_dma_rx.Init(1, 6, 2); // USART2_RX
}


#elif defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO64_STM32F405) \

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB DP

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART2_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART2_RX
  usbuart.Init(2);

  usbuart_dma_tx.Init(1, 6, 4); // USART2_TX
  usbuart_dma_rx.Init(1, 5, 4); // USART2_RX
}


#elif defined(BOARD_MIBO64_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1);  // USART1_RX
  conuart.Init(1);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_0 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_0 | PINCFG_SPEED_FAST);  // USB DP

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART2_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT  | PINCFG_AF_1 | PINCFG_PULLUP);  // USART2_RX
  usbuart.Init(2);

  usbuart_dma_tx.Init(1, 4, 2); // USART2_TX
  usbuart_dma_rx.Init(1, 5, 2); // USART2_RX
}

#elif defined(BOARD_MIBO20_STM32F070)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 1, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_1 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_0 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_0 | PINCFG_SPEED_FAST);  // USB DP

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_1);  // USART2_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT  | PINCFG_AF_1 | PINCFG_PULLUP);  // USART2_RX
  usbuart.Init(2);

  usbuart_dma_tx.Init(1, 4, 2); // USART2_TX
  usbuart_dma_rx.Init(1, 5, 2); // USART2_RX
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();

  // UART - On the Arduino programmer interface
  hwpinctrl.PinSetup(0, 8, PINCFG_INPUT | PINCFG_AF_0);  // UART_RXD
  hwpinctrl.PinSetup(0, 9, PINCFG_OUTPUT | PINCFG_AF_0); // UART_TXD
  conuart.Init(0);  // UART

  // dedicated HS USB pins, no pin setup required

  #error "define USB-UART pins"
}

#elif defined(BOARD_MIBO64_ATSAM4S)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.Init(0);

  hwpinctrl.PinSetup(PORTNUM_B,  2,  PINCFG_INPUT  | PINCFG_AF_A);  // URXD1
  hwpinctrl.PinSetup(PORTNUM_B,  3,  PINCFG_OUTPUT | PINCFG_AF_A);  // UTXD1
  usbuart.Init(1);

  usbuart.PdmaInit(true,  &usbuart_dma_tx);
  usbuart.PdmaInit(false, &usbuart_dma_rx);
}

#elif defined(BOARD_MIBO100_ATSAME70)

#warning "Imperfect USB implementation (config descriptor segmentation)"

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_D, 13, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  conuart.Init(0);

  // dedicated HS USB pins, no pin setup required

  hwpinctrl.PinSetup(PORTNUM_A,  5,  PINCFG_INPUT  | PINCFG_AF_C);  // UART1_RX
  hwpinctrl.PinSetup(PORTNUM_A,  4,  PINCFG_OUTPUT | PINCFG_AF_C);  // UART1_TX
  usbuart.Init(1);

  usbuart_dma_tx.Init(4, 22); // 22 = UART1_TX
  usbuart_dma_rx.Init(5, 23); // 23 = UART1_RX
}

#elif defined(BOARD_XPLAINED_SAME70)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 8, false);
  board_pins_init_leds();

  // USART1 - EDBG
  hwpinctrl.PinSetup(0, 21, PINCFG_INPUT | PINCFG_AF_0);  // USART1_RXD
  MATRIX->CCFG_SYSIO |= (1 << 4); // select PB4 instead of TDI !!!!!!!!!
  hwpinctrl.PinSetup(1,  4, PINCFG_OUTPUT | PINCFG_AF_3); // USART1_TXD
  conuart.Init(0x101); // USART1

  // UART3 - Arduino shield
  //hwpinctrl.PinSetup(3, 28, PINCFG_INPUT | PINCFG_AF_0);  // UART3_RXD
  //hwpinctrl.PinSetup(3, 30, PINCFG_OUTPUT | PINCFG_AF_0); // UART3_TXD
  //uartx2.Init(3); // UART3

  // dedicated HS USB pins, no pin setup required

  #error "define USB-UART pins"
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  // SERCOM0
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_OUTPUT | PINCFG_AF_3);  // PAD[0] = TX
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_INPUT  | PINCFG_AF_3);  // PAD[1] = RX
  conuart.Init(0);

  // SERCOM2
  //hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_AF_2);  // PAD[0] = TX
  //hwpinctrl.PinSetup(PORTNUM_A, 13, PINCFG_AF_2);  // PAD[1] = RX
  //conuart.Init(2);

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 24, PINCFG_AF_7);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_AF_7);  // USB DP

  // SERCOM2
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_AF_2);  // PAD[0] = TX
  hwpinctrl.PinSetup(PORTNUM_A, 13, PINCFG_AF_2);  // PAD[1] = RX
  usbuart.Init(2);

  usbuart_dma_tx.Init(4, 0x09); // 0x09 = SERCOM2_TX
  usbuart_dma_rx.Init(5, 0x08); // 0x08 = SERCOM2_RX
}

// RP

#elif defined(BOARD_RPI_PICO)

#if SPI_SELF_FLASHING
  THwQspi    fl_qspi;
  TSpiFlash  spiflash;
#endif

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 25, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  conuart.Init(0);

  #if SPI_SELF_FLASHING
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
  #endif

  // it has dedicated USB pins, so no USB pin setup required

  // UART1
  hwpinctrl.PinSetup(0, 20, PINCFG_AF_2);  // UART1_TX
  hwpinctrl.PinSetup(0, 21, PINCFG_AF_2);  // UART1_RX
  usbuart.Init(1);

  usbuart_dma_tx.Init(4, DREQ_UART1_TX);
  usbuart_dma_rx.Init(5, DREQ_UART1_RX);
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

