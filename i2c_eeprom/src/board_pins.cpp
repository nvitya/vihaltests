/*
 *  file:     board_pins.cpp (i2c_eeprom)
 *  brief:    I2C EEPROM Test Board pins
 *  version:  1.00
 *  date:     2021-11-01
 *  authors:  nvitya
*/

#include "board_pins.h"

THwI2c         i2c;
THwDmaChannel  i2c_txdma;
THwDmaChannel  i2c_rxdma;

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

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_MIN_F103)  // = blue pill

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_0 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // I2C1
  // open drain mode have to be used, otherwise it won't work
  // External pull-ups are required !
  hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_AF_0 | PINCFG_OPENDRAIN | PINCFG_SPEED_FAST); // I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_AF_0 | PINCFG_OPENDRAIN | PINCFG_SPEED_FAST); // I2C1_SDA

  i2c.Init(1); // I2C1

  i2c.txdma.Init(1, 6, 3);  // DMA1/CH6 = I2C1_TX
  i2c.rxdma.Init(1, 7, 3);  // DMA1/CH7 = I2C1_RX
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

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);
}

// ATSAM

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

  #define TWI_NUM  1

  #if 0 == TWI_NUM
    // TWI0 (I2C0), the first two pins closest to the reset button
    hwpinctrl.PinSetup(PORTNUM_A, 17, PINCFG_AF_A);  // TWI0_SDA (TWD)
    hwpinctrl.PinSetup(PORTNUM_A, 18, PINCFG_AF_A);  // TWI0_SCL (TWCK)
    i2c.speed = 100000; // 100 kHz
    i2c.Init(0);

    #if 1
      i2c_txdma.Init(2, 7);
      i2c_rxdma.Init(3, 8);

      i2c.DmaAssign(true,  &i2c_txdma);
      i2c.DmaAssign(false, &i2c_rxdma);
    #endif

  #elif 1 == TWI_NUM
    // TWI1 (I2C1), the pins 20, 21
    hwpinctrl.PinSetup(PORTNUM_B, 12, PINCFG_AF_A);  // TWI1_SDA (TWD)
    hwpinctrl.PinSetup(PORTNUM_B, 13, PINCFG_AF_A);  // TWI1_SCL (TWCK)
    i2c.speed = 100000; // 100 kHz
    i2c.Init(1);

    #if 1
      i2c.PdmaInit(true,  &i2c_txdma);
      i2c.PdmaInit(false, &i2c_rxdma);
    #endif
  #else
    #error "invalid TWI / I2C"
  #endif
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
}

#elif defined(BOARD_ENEBO_A)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_A, 20, true);
  pin_led[1].Assign(PORTNUM_D, 14, true);
  pin_led[2].Assign(PORTNUM_D, 13, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.baudrate = 115200;
  conuart.Init(0);
}

// LPC

#else
  #error "Define board_pins_init here"
#endif

#ifndef HEXNUM_DISPLAY

void board_show_hexnum(unsigned ahexnum)
{
  // nothing
}

#endif

