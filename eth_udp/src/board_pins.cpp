/*
 *  file:     board_pins.cpp (eth_raw)
 *  brief:    Board Specific Settings
 *  version:  1.00
 *  date:     2022-03-14
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "traces.h"
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

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

THwEth    eth;

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

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

  /* Ethernet pins configuration ************************************************

          RMII_REF_CLK ----------------------> PA1
          RMII_MDIO -------------------------> PA2
          RMII_MDC --------------------------> PC1
          RMII_MII_CRS_DV -------------------> PA7
          RMII_MII_RXD0 ---------------------> PC4
          RMII_MII_RXD1 ---------------------> PC5
          RMII_MII_RXER ---------------------> PG2
          RMII_MII_TX_EN --------------------> PG11
          RMII_MII_TXD0 ---------------------> PG13
          RMII_MII_TXD1 ---------------------> PB13
  */

  uint32_t pinfl = PINCFG_SPEED_FAST | PINCFG_AF_11;  // do not use PINCFG_SPEED_VERYFAST !

  hwpinctrl.PinSetup(PORTNUM_A,  1, pinfl); // REF CLK
  hwpinctrl.PinSetup(PORTNUM_A,  2, pinfl); // MDIO
  hwpinctrl.PinSetup(PORTNUM_C,  1, pinfl); // MDC
  hwpinctrl.PinSetup(PORTNUM_A,  7, pinfl); // CRS_DV
  hwpinctrl.PinSetup(PORTNUM_C,  4, pinfl); // RXD0
  hwpinctrl.PinSetup(PORTNUM_C,  5, pinfl); // RXD1
  hwpinctrl.PinSetup(PORTNUM_G,  2, pinfl); // RXER
  hwpinctrl.PinSetup(PORTNUM_G, 11, pinfl); // TX_EN
  hwpinctrl.PinSetup(PORTNUM_G, 13, pinfl); // TXD0
  hwpinctrl.PinSetup(PORTNUM_B, 13, pinfl); // TXD1

  /* Enable the Ethernet global Interrupt */
  //HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
  //HAL_NVIC_EnableIRQ(ETH_IRQn);

  eth.phy_address = 0;
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

  /* Ethernet pins configuration ************************************************

          RMII_REF_CLK ----------------------> PD0
          RMII_MDIO -------------------------> PD9
          RMII_MDC --------------------------> PD8
          RMII_MII_CRS_DV -------------------> PD4
          RMII_MII_RXD0 ---------------------> PD5
          RMII_MII_RXD1 ---------------------> PD6
          RMII_MII_RXER ---------------------> PD7
          RMII_MII_TX_EN --------------------> PD1
          RMII_MII_TXD0 ---------------------> PD2
          RMII_MII_TXD1 ---------------------> PD3
  */

  uint32_t pinfl = PINCFG_SPEED_FAST | PINCFG_AF_0;

  hwpinctrl.PinSetup(PORTNUM_D, 0, pinfl); // REF CLK
  hwpinctrl.PinSetup(PORTNUM_D, 9, pinfl); // MDIO
  hwpinctrl.PinSetup(PORTNUM_D, 8, pinfl); // MDC
  hwpinctrl.PinSetup(PORTNUM_D, 4, pinfl); // CRS_DV
  hwpinctrl.PinSetup(PORTNUM_D, 5, pinfl); // RXD0
  hwpinctrl.PinSetup(PORTNUM_D, 6, pinfl); // RXD1
  hwpinctrl.PinSetup(PORTNUM_D, 7, pinfl); // RXER
  hwpinctrl.PinSetup(PORTNUM_D, 1, pinfl); // TX_EN
  hwpinctrl.PinSetup(PORTNUM_D, 2, pinfl); // TXD0
  hwpinctrl.PinSetup(PORTNUM_D, 3, pinfl); // TXD1

  // Extra PHY Signals
  hwpinctrl.PinSetup(PORTNUM_C, 10, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // PHY RESET
  hwpinctrl.PinSetup(PORTNUM_A, 14, PINCFG_INPUT | PINCFG_PULLUP); // PHY INTERRUPT

  eth.phy_address = 0;
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

#if 1
  hwpinctrl.PinSetup(PORTNUM_A,  6,  PINCFG_OUTPUT | PINCFG_AF_1);  // PCK0 = FPGA.CLK_IN

  PMC->PMC_SCER = (1 << 8); // enable PCK0

  PMC->PMC_PCK[0] = 0
    | (1 << 0)  // CSS(3): 1 = MAIN CLK
    | (9 << 4)  // PRES(8): divisor - 1
  ;
#endif

  /* Ethernet pins configuration ************************************************

          RMII_REF_CLK ----------------------> PD0
          RMII_MDIO -------------------------> PD9
          RMII_MDC --------------------------> PD8
          RMII_MII_CRS_DV -------------------> PD4
          RMII_MII_RXD0 ---------------------> PD5
          RMII_MII_RXD1 ---------------------> PD6
          RMII_MII_RXER ---------------------> PD7
          RMII_MII_TX_EN --------------------> PD1
          RMII_MII_TXD0 ---------------------> PD2
          RMII_MII_TXD1 ---------------------> PD3
  */

  uint32_t pinfl = PINCFG_SPEED_FAST | PINCFG_AF_0;

  hwpinctrl.PinSetup(PORTNUM_D, 0, pinfl); // REF CLK
  hwpinctrl.PinSetup(PORTNUM_D, 9, pinfl); // MDIO
  hwpinctrl.PinSetup(PORTNUM_D, 8, pinfl); // MDC
  hwpinctrl.PinSetup(PORTNUM_D, 4, pinfl); // CRS_DV
  hwpinctrl.PinSetup(PORTNUM_D, 5, pinfl); // RXD0
  hwpinctrl.PinSetup(PORTNUM_D, 6, pinfl); // RXD1
  hwpinctrl.PinSetup(PORTNUM_D, 7, pinfl); // RXER       // Tie to the GND !!!
  hwpinctrl.PinSetup(PORTNUM_D, 1, pinfl); // TX_EN
  hwpinctrl.PinSetup(PORTNUM_D, 2, pinfl); // TXD0
  hwpinctrl.PinSetup(PORTNUM_D, 3, pinfl); // TXD1

  eth.phy_address = 1;


}

#elif defined(BOARD_ENEBO_A)

TGpioPin  pin_eth_reset(PORTNUM_A, 19, false);

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

  pin_eth_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  // Ethernet clock output:
  hwpinctrl.PinSetup(PORTNUM_A,  18,  PINCFG_OUTPUT | PINCFG_AF_B);  // PCK2 = Ethernet 25 M Clock

  PMC->PMC_SCER = (1 << 10); // enable PCK2

  PMC->PMC_PCK[2] = 0
    | (2 << 0)  // CSS(3): 2 = PLLA
    | ((12 - 1) << 4)  // PRES(8): divisor - 1
  ;

  /* Ethernet pins configuration ************************************************

          RMII_REF_CLK ----------------------> PD0
          RMII_MDIO -------------------------> PD9
          RMII_MDC --------------------------> PD8
          RMII_MII_CRS_DV -------------------> PD4
          RMII_MII_RXD0 ---------------------> PD5
          RMII_MII_RXD1 ---------------------> PD6
          RMII_MII_RXER ---------------------> PD7
          RMII_MII_TX_EN --------------------> PD1
          RMII_MII_TXD0 ---------------------> PD2
          RMII_MII_TXD1 ---------------------> PD3
  */

  uint32_t pinfl = PINCFG_SPEED_FAST | PINCFG_AF_0;

  hwpinctrl.PinSetup(PORTNUM_D, 0, pinfl); // REF CLK
  hwpinctrl.PinSetup(PORTNUM_D, 9, pinfl); // MDIO
  hwpinctrl.PinSetup(PORTNUM_D, 8, pinfl); // MDC
  hwpinctrl.PinSetup(PORTNUM_D, 4, pinfl); // CRS_DV
  hwpinctrl.PinSetup(PORTNUM_D, 5, pinfl); // RXD0
  hwpinctrl.PinSetup(PORTNUM_D, 6, pinfl); // RXD1
  hwpinctrl.PinSetup(PORTNUM_D, 7, pinfl); // RXER       // Tie to the GND !!!
  hwpinctrl.PinSetup(PORTNUM_D, 1, pinfl); // TX_EN
  hwpinctrl.PinSetup(PORTNUM_D, 2, pinfl); // TXD0
  hwpinctrl.PinSetup(PORTNUM_D, 3, pinfl); // TXD1

  eth.phy_address = 1;

  delay_us(10);

  pin_eth_reset.Set1(); // start the phy

  delay_us(100);
}

#else
  #error "Define board_pins_init here"
#endif
