/*
 *  file:     board_pins.cpp (uart)
 *  brief:    UART Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "hwsdram.h"

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
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_DISCOVERY_F746) || defined(BOARD_DISCOVERY_F750)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_I,  1, false);
  board_pins_init_leds();

  // turn off LCD backlight:
  hwpinctrl.PinSetup(PORTNUM_K,  3, PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PinSetup(PORTNUM_A, 9,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_B, 7,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1

  // SDRAM pins

  #if defined(BOARD_DISCOVERY_F746)
    unsigned pin_flags = PINCFG_AF_12 | PINCFG_SPEED_MEDIUM; // does not work with fast ?
  #else
    unsigned pin_flags = PINCFG_AF_12 | PINCFG_SPEED_FAST;
  #endif

  hwpinctrl.PinSetup(PORTNUM_C,  3, pin_flags);  // CKE0

  hwpinctrl.PinSetup(PORTNUM_D,  0, pin_flags);  // D2
  hwpinctrl.PinSetup(PORTNUM_D,  1, pin_flags);  // D3
  hwpinctrl.PinSetup(PORTNUM_D,  8, pin_flags);  // D13
  hwpinctrl.PinSetup(PORTNUM_D,  9, pin_flags);  // D14
  hwpinctrl.PinSetup(PORTNUM_D, 10, pin_flags);  // D15
  hwpinctrl.PinSetup(PORTNUM_D, 14, pin_flags);  // D0
  hwpinctrl.PinSetup(PORTNUM_D, 15, pin_flags);  // D1


  hwpinctrl.PinSetup(PORTNUM_E,  0, pin_flags);  // NBL0
  hwpinctrl.PinSetup(PORTNUM_E,  1, pin_flags);  // NBL1
  hwpinctrl.PinSetup(PORTNUM_E,  7, pin_flags);  // D4
  hwpinctrl.PinSetup(PORTNUM_E,  8, pin_flags);  // D5
  hwpinctrl.PinSetup(PORTNUM_E,  9, pin_flags);  // D6
  hwpinctrl.PinSetup(PORTNUM_E, 10, pin_flags);  // D7
  hwpinctrl.PinSetup(PORTNUM_E, 11, pin_flags);  // D8
  hwpinctrl.PinSetup(PORTNUM_E, 12, pin_flags);  // D9
  hwpinctrl.PinSetup(PORTNUM_E, 13, pin_flags);  // D10
  hwpinctrl.PinSetup(PORTNUM_E, 14, pin_flags);  // D11
  hwpinctrl.PinSetup(PORTNUM_E, 15, pin_flags);  // D12

  hwpinctrl.PinSetup(PORTNUM_F,  0, pin_flags);  // A0
  hwpinctrl.PinSetup(PORTNUM_F,  1, pin_flags);  // A1
  hwpinctrl.PinSetup(PORTNUM_F,  2, pin_flags);  // A2
  hwpinctrl.PinSetup(PORTNUM_F,  3, pin_flags);  // A3
  hwpinctrl.PinSetup(PORTNUM_F,  4, pin_flags);  // A4
  hwpinctrl.PinSetup(PORTNUM_F,  5, pin_flags);  // A5
  hwpinctrl.PinSetup(PORTNUM_F, 11, pin_flags);  // SDNRAS
  hwpinctrl.PinSetup(PORTNUM_F, 12, pin_flags);  // A6
  hwpinctrl.PinSetup(PORTNUM_F, 13, pin_flags);  // A7
  hwpinctrl.PinSetup(PORTNUM_F, 14, pin_flags);  // A8
  hwpinctrl.PinSetup(PORTNUM_F, 15, pin_flags);  // A9

  hwpinctrl.PinSetup(PORTNUM_G,  0, pin_flags);  // A10
  hwpinctrl.PinSetup(PORTNUM_G,  1, pin_flags);  // A11
  hwpinctrl.PinSetup(PORTNUM_G,  4, pin_flags);  // BA0
  hwpinctrl.PinSetup(PORTNUM_G,  5, pin_flags);  // BA1
  hwpinctrl.PinSetup(PORTNUM_G,  8, pin_flags);  // SDCLK
  hwpinctrl.PinSetup(PORTNUM_G, 15, pin_flags);  // SDNCAS

  hwpinctrl.PinSetup(PORTNUM_H,  3, pin_flags);  // SDNE0
  hwpinctrl.PinSetup(PORTNUM_H,  5, pin_flags);  // SDNWE

  // config the SDRAM device: 8 MByte

  hwsdram.row_bits = 12;
  hwsdram.column_bits = 8;
  hwsdram.bank_count = 4;
  hwsdram.cas_latency = 2;  // 2 ?

  hwsdram.row_precharge_delay = 2;
  hwsdram.row_to_column_delay = 2;
  hwsdram.recovery_delay = 2;
  hwsdram.row_cycle_delay = 6;
  hwsdram.exit_self_refresh_delay = 6;
  hwsdram.active_to_precharge_delay = 2; // TRAS

  hwsdram.burst_length = 1;  // it does not like when it bigger than 1

  hwsdram.Init();
}

#elif defined(BOARD_DISCOVERY_F429)

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_G, 13, false);
  pin_led[1].Assign(PORTNUM_G, 14, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A, 9,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_B, 7,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1

  // it does not run with PINCFG_SPEED_FAST !
  unsigned pin_flags = PINCFG_AF_12 | PINCFG_SPEED_MED2;

  hwpinctrl.PinSetup(PORTNUM_B,  5, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_B,  6, pin_flags);

  hwpinctrl.PinSetup(PORTNUM_C,  0, pin_flags);

  hwpinctrl.PinSetup(PORTNUM_D,  0, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_D,  1, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_D,  8, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_D,  9, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_D, 10, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_D, 14, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_D, 15, pin_flags);

  hwpinctrl.PinSetup(PORTNUM_E,  0, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E,  1, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E,  7, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E,  8, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E,  9, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E, 10, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E, 11, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E, 12, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E, 13, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E, 14, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_E, 15, pin_flags);

  hwpinctrl.PinSetup(PORTNUM_F,  0, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F,  1, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F,  2, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F,  3, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F,  4, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F,  5, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F, 11, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F, 12, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F, 13, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F, 14, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_F, 15, pin_flags);

  hwpinctrl.PinSetup(PORTNUM_G,  0, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_G,  1, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_G,  4, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_G,  5, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_G,  8, pin_flags);
  hwpinctrl.PinSetup(PORTNUM_G, 15, pin_flags);

  // config the SDRAM device: 8 MByte

  hwsdram.bank = 2; // it is connected to bank 2!

  hwsdram.hclk_div = 2;

  hwsdram.row_bits = 12;
  hwsdram.column_bits = 8;
  hwsdram.bank_count = 4;
  hwsdram.cas_latency = 3;

  hwsdram.row_precharge_delay = 2;
  hwsdram.row_to_column_delay = 2;
  hwsdram.recovery_delay = 2;
  hwsdram.row_cycle_delay = 7;
  hwsdram.exit_self_refresh_delay = 7;
  hwsdram.active_to_precharge_delay = 4; // TRAS

  hwsdram.burst_length = 1;

  hwsdram.Init();
}

// ATSAM

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

  // SDRAM

  // it does not work with strong drive !
  uint32_t pincfgbase = 0; // PINCFG_DRIVE_STRONG;

  hwpinctrl.PinSetup(PORTNUM_A, 20, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // A16/BA0

  hwpinctrl.PinSetup(PORTNUM_C, 0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D0
  hwpinctrl.PinSetup(PORTNUM_C, 1, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D1
  hwpinctrl.PinSetup(PORTNUM_C, 2, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D2
  hwpinctrl.PinSetup(PORTNUM_C, 3, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D3
  hwpinctrl.PinSetup(PORTNUM_C, 4, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D4
  hwpinctrl.PinSetup(PORTNUM_C, 5, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D5
  hwpinctrl.PinSetup(PORTNUM_C, 6, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D6
  hwpinctrl.PinSetup(PORTNUM_C, 7, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D7

  hwpinctrl.PinSetup(PORTNUM_E, 0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D8
  hwpinctrl.PinSetup(PORTNUM_E, 1, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D9
  hwpinctrl.PinSetup(PORTNUM_E, 2, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D10
  hwpinctrl.PinSetup(PORTNUM_E, 3, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D11
  hwpinctrl.PinSetup(PORTNUM_E, 4, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D12
  hwpinctrl.PinSetup(PORTNUM_E, 5, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D13

  hwpinctrl.PinSetup(PORTNUM_A, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D14
  hwpinctrl.PinSetup(PORTNUM_A, 16, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D15

  hwpinctrl.PinSetup(PORTNUM_C, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // NCS1/SDCS

  hwpinctrl.PinSetup(PORTNUM_C, 18, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A0/NBS0
  //hwpinctrl.PinSetup(PORTNUM_C, 19, PINCFG_OUTPUT | PINCFG_AF_0);  // A1

  hwpinctrl.PinSetup(PORTNUM_C, 20, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A2
  hwpinctrl.PinSetup(PORTNUM_C, 21, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A3
  hwpinctrl.PinSetup(PORTNUM_C, 22, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A4
  hwpinctrl.PinSetup(PORTNUM_C, 23, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A5
  hwpinctrl.PinSetup(PORTNUM_C, 24, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A6
  hwpinctrl.PinSetup(PORTNUM_C, 25, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A7
  hwpinctrl.PinSetup(PORTNUM_C, 26, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A8
  hwpinctrl.PinSetup(PORTNUM_C, 27, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A9
  hwpinctrl.PinSetup(PORTNUM_C, 28, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A10
  hwpinctrl.PinSetup(PORTNUM_C, 29, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A11

  hwpinctrl.PinSetup(PORTNUM_D, 13, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDA10
  hwpinctrl.PinSetup(PORTNUM_D, 14, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDCKE
  hwpinctrl.PinSetup(PORTNUM_D, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // NWR1/NBS1
  hwpinctrl.PinSetup(PORTNUM_D, 16, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // RAS
  hwpinctrl.PinSetup(PORTNUM_D, 17, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // CAS
  hwpinctrl.PinSetup(PORTNUM_D, 23, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDCK
  hwpinctrl.PinSetup(PORTNUM_D, 29, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDWE


  // config for 2 MByte onboard SDRAM

  hwsdram.row_bits = 11;
  hwsdram.column_bits = 8;
  hwsdram.bank_count = 2;
  hwsdram.cas_latency = 3;

  hwsdram.row_precharge_delay = 5;
  hwsdram.row_to_column_delay = 5;
  hwsdram.recovery_delay = 5;
  hwsdram.row_cycle_delay = 13;

  hwsdram.burst_length = 1;  // SDRAM does not work properly when larger than 1, but no speed degradation noticed

  hwsdram.Init();
}

#elif defined(BOARD_VERTIBO_A)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 29, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.baudrate = 115200;
  conuart.Init(0);

  // SDRAM

  // it does not work with strong drive !
  uint32_t pincfgbase = 0; // PINCFG_DRIVE_STRONG;

  hwpinctrl.PinSetup(PORTNUM_A, 20, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // A16/BA0
  hwpinctrl.PinSetup(PORTNUM_A,  0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // A17/BA1

  hwpinctrl.PinSetup(PORTNUM_C, 0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D0
  hwpinctrl.PinSetup(PORTNUM_C, 1, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D1
  hwpinctrl.PinSetup(PORTNUM_C, 2, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D2
  hwpinctrl.PinSetup(PORTNUM_C, 3, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D3
  hwpinctrl.PinSetup(PORTNUM_C, 4, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D4
  hwpinctrl.PinSetup(PORTNUM_C, 5, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D5
  hwpinctrl.PinSetup(PORTNUM_C, 6, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D6
  hwpinctrl.PinSetup(PORTNUM_C, 7, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D7

  hwpinctrl.PinSetup(PORTNUM_E, 0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D8
  hwpinctrl.PinSetup(PORTNUM_E, 1, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D9
  hwpinctrl.PinSetup(PORTNUM_E, 2, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D10
  hwpinctrl.PinSetup(PORTNUM_E, 3, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D11
  hwpinctrl.PinSetup(PORTNUM_E, 4, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D12
  hwpinctrl.PinSetup(PORTNUM_E, 5, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D13

  hwpinctrl.PinSetup(PORTNUM_A, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D14
  hwpinctrl.PinSetup(PORTNUM_A, 16, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D15

  hwpinctrl.PinSetup(PORTNUM_C, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // NCS1/SDCS

  hwpinctrl.PinSetup(PORTNUM_C, 18, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A0/NBS0
  //hwpinctrl.PinSetup(PORTNUM_C, 19, PINCFG_OUTPUT | PINCFG_AF_0);  // A1

  hwpinctrl.PinSetup(PORTNUM_C, 20, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A2
  hwpinctrl.PinSetup(PORTNUM_C, 21, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A3
  hwpinctrl.PinSetup(PORTNUM_C, 22, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A4
  hwpinctrl.PinSetup(PORTNUM_C, 23, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A5
  hwpinctrl.PinSetup(PORTNUM_C, 24, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A6
  hwpinctrl.PinSetup(PORTNUM_C, 25, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A7
  hwpinctrl.PinSetup(PORTNUM_C, 26, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A8
  hwpinctrl.PinSetup(PORTNUM_C, 27, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A9
  hwpinctrl.PinSetup(PORTNUM_C, 28, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A10
  hwpinctrl.PinSetup(PORTNUM_C, 29, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A11
  hwpinctrl.PinSetup(PORTNUM_C, 30, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A12
  hwpinctrl.PinSetup(PORTNUM_C, 31, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A13
  hwpinctrl.PinSetup(PORTNUM_A, 18, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // A14

  hwpinctrl.PinSetup(PORTNUM_D, 13, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDA10
  hwpinctrl.PinSetup(PORTNUM_D, 14, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDCKE
  hwpinctrl.PinSetup(PORTNUM_D, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // NWR1/NBS1
  hwpinctrl.PinSetup(PORTNUM_D, 16, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // RAS
  hwpinctrl.PinSetup(PORTNUM_D, 17, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // CAS
  hwpinctrl.PinSetup(PORTNUM_D, 23, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDCK
  hwpinctrl.PinSetup(PORTNUM_D, 29, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_2);  // SDWE


  // config for MT48LC16M16A2-6A: 32 MByte

  hwsdram.row_bits = 13;
  hwsdram.column_bits = 9;
  hwsdram.bank_count = 4;
  hwsdram.cas_latency = 3;

  hwsdram.row_precharge_delay = 3;
  hwsdram.row_to_column_delay = 3;
  hwsdram.recovery_delay = 2;
  hwsdram.row_cycle_delay = 9;

  hwsdram.burst_length = 1;  // SDRAM does not work properly when larger than 1, but no speed degradation noticed

  hwsdram.Init();
}

// LPC

#elif defined(BOARD_XPRESSO_LPC54608)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(2,  2, true);
  pin_led[1].Assign(3,  3, true);
  pin_led[2].Assign(3, 14, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0, 30, PINCFG_OUTPUT | PINCFG_AF_1); // UART_TX:
  hwpinctrl.PinSetup(0, 29, PINCFG_INPUT  | PINCFG_AF_1); // UART_RX:
  conuart.Init(0);
}

// IMXRT

#elif defined(BOARD_EVK_IMXRT1020)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(1, 5, false);  // GPIO_AD_B0_05 = GPIO_1_5
  board_pins_init_leds();

  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_06_LPUART1_TX, 0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_07_LPUART1_RX, 0);
  conuart.Init(1); // UART1

  // SDRAM

  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_00_SEMC_DATA00,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_01_SEMC_DATA01,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_02_SEMC_DATA02,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_03_SEMC_DATA03,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_04_SEMC_DATA04,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_05_SEMC_DATA05,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_06_SEMC_DATA06,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_07_SEMC_DATA07,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_08_SEMC_DM00,    0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_09_SEMC_WE,      0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_10_SEMC_CAS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_11_SEMC_RAS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_12_SEMC_CS0,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_13_SEMC_BA0,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_14_SEMC_BA1,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_15_SEMC_ADDR10,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_16_SEMC_ADDR00,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_17_SEMC_ADDR01,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_18_SEMC_ADDR02,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_19_SEMC_ADDR03,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_20_SEMC_ADDR04,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_21_SEMC_ADDR05,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_22_SEMC_ADDR06,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_23_SEMC_ADDR07,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_24_SEMC_ADDR08,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_25_SEMC_ADDR09,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_26_SEMC_ADDR11,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_27_SEMC_ADDR12,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_28_SEMC_DQS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_29_SEMC_CKE,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_30_SEMC_CLK,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_31_SEMC_DM01,    0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_32_SEMC_DATA08,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_33_SEMC_DATA09,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_34_SEMC_DATA10,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_35_SEMC_DATA11,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_36_SEMC_DATA12,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_37_SEMC_DATA13,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_38_SEMC_DATA14,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_39_SEMC_DATA15,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_40_SEMC_CSX00,   0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_41_SEMC_READY,   0);

  // config for MT48LC16M16A2-6A: 32 MByte

  hwsdram.row_bits = 13;
  hwsdram.column_bits = 9;
  hwsdram.bank_count = 4;
  hwsdram.cas_latency = 3;

  hwsdram.row_precharge_delay = 3;
  hwsdram.row_to_column_delay = 3;
  hwsdram.recovery_delay = 2;
  hwsdram.row_cycle_delay = 9;

  hwsdram.burst_length = 8;

  hwsdram.Init();
}

#elif defined(BOARD_EVK_IMXRT1050) || defined(BOARD_EVK_IMXRT1050A)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(1, 9, false);  // GPIO_AD_B0_09 = GPIO_1_9
  board_pins_init_leds();

  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_12_LPUART1_TX, 0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_13_LPUART1_RX, 0);
  conuart.Init(1); // UART1

  // SDRAM

  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_00_SEMC_DATA00,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_01_SEMC_DATA01,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_02_SEMC_DATA02,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_03_SEMC_DATA03,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_04_SEMC_DATA04,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_05_SEMC_DATA05,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_06_SEMC_DATA06,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_07_SEMC_DATA07,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_08_SEMC_DM00,    0);

  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_09_SEMC_ADDR00,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_10_SEMC_ADDR01,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_11_SEMC_ADDR02,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_12_SEMC_ADDR03,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_13_SEMC_ADDR04,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_14_SEMC_ADDR05,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_15_SEMC_ADDR06,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_16_SEMC_ADDR07,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_17_SEMC_ADDR08,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_18_SEMC_ADDR09,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_19_SEMC_ADDR11,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_20_SEMC_ADDR12,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_21_SEMC_BA0,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_22_SEMC_BA1,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_23_SEMC_ADDR10,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_24_SEMC_CAS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_25_SEMC_RAS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_26_SEMC_CLK,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_27_SEMC_CKE,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_28_SEMC_WE,      0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_29_SEMC_CS0,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_30_SEMC_DATA08,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_31_SEMC_DATA09,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_32_SEMC_DATA10,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_33_SEMC_DATA11,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_34_SEMC_DATA12,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_35_SEMC_DATA13,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_36_SEMC_DATA14,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_37_SEMC_DATA15,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_38_SEMC_DM01,    0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_39_SEMC_DQS,     0);

  // config for MT48LC16M16A2-6A: 32 MByte

  hwsdram.row_bits = 13;
  hwsdram.column_bits = 9;
  hwsdram.bank_count = 4;
  hwsdram.cas_latency = 3;

  hwsdram.row_precharge_delay = 3;
  hwsdram.row_to_column_delay = 3;
  hwsdram.recovery_delay = 2;
  hwsdram.row_cycle_delay = 9;

  hwsdram.burst_length = 8;

  hwsdram.Init();
}

#else
  #error "Define board_pins_init here"
#endif

