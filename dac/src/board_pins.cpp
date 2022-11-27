/*
 *  file:     board_pins.cpp
 *  brief:    SPI PSRAM Test Board pins
 *  date:     2022-03-23
 *  authors:  nvitya
*/

#include "board_pins.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 0;

TGpioPin  pin_led[MAX_LEDS] =
{
  TGpioPin(),
  TGpioPin(),
  TGpioPin(),
  TGpioPin()
};

THwDacChannel dac;

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0 // to use elif everywhere

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

  // DAC
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT1
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT2
  hwpinctrl.PinSetup(PORTNUM_A, 6, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC2_OUT1

  // DMAMUX: DAC1_CH1 = 6
  // DMAMUX: DAC1_CH2 = 7
  // DMAMUX: DAC2_CH1 = 41

  dac.dmach.Init(1,  2,  6); // 6 = DAC1_CH1
  dac.Init(1, 1);  // DAC1_CH1
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

  // DAC
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT1
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT2

  dac.dmach.Init(2, 3, 3); // dma2ch3 = DAC1_CH1/TIM6
  dac.Init(1, 1);  // DAC1_CH1
}

#elif defined(BOARD_MIBO64_STM32F405)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // DAC
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT1
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT2

  dac.dmach.Init(1, 5, 7); // dma1st5ch3 = DAC1_CH1
  dac.Init(1, 1);  // DAC1_CH1
}

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746)

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

  // DAC
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT1
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT2

  dac.dmach.Init(1, 5, 7); // dma1st5ch3 = DAC1_CH1
  dac.Init(1, 1);  // DAC1_CH1
}

#elif defined(BOARD_NUCLEO_H743)

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

  // DAC
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT1
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_ANALOGUE | PINCFG_OUTPUT);  // DAC1_OUT2

  // Warning the DMA1/DMA2 cannot access the DTCM, use linker scipt where the data placed to AXI RAM or AHB RAM
  dac.dmach.Init(1, 1, 67); // use dma1, stream1 for old-style DMA with DMAMUX 67 = DAC1_CH1
  dac.Init(1, 1);  // DAC1_CH1
}

#elif defined(BOARD_ARDUINO_DUE)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();

  // UART - On the Arduino programmer interface
  hwpinctrl.PinSetup(PORTNUM_A, 8, PINCFG_INPUT | PINCFG_AF_0);  // UART_RXD
  hwpinctrl.PinSetup(PORTNUM_A, 9, PINCFG_OUTPUT | PINCFG_AF_0); // UART_TXD
  conuart.Init(0);  // UART
}

#elif defined(BOARD_MIBO64_ATSAM4S)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  conuart.Init(0);
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
}

#else
  #error "Define board_pins_init here"
#endif

