/*
 *  file:     board_pins.cpp (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
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

THwSpi    fl_spi;
THwQspi   fl_qspi;

#if defined(MCUSF_VRV100) //BOARD_VRV100_441)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 0, false);
  pin_led[0].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  conuart.Init(1); // UART1

  fl_spi.speed = 8000000;
  fl_spi.Init(1); // flash
}

#endif

#if defined(BOARD_MIBO48_STM32G473) || defined(BOARD_MIBO48_STM32F303) || defined(BOARD_MIBO64_STM32F405)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  pin_led[0].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  uint32_t qspipincfg = PINCFG_SPEED_FAST;

  hwpinctrl.PinSetup(PORTNUM_B, 11, qspipincfg | PINCFG_AF_10);   // NCS
  hwpinctrl.PinSetup(PORTNUM_B, 10, qspipincfg | PINCFG_AF_10);   // CLK

  hwpinctrl.PinSetup(PORTNUM_B,  1, qspipincfg | PINCFG_AF_10);   // IO0
  hwpinctrl.PinSetup(PORTNUM_B,  0, qspipincfg | PINCFG_AF_10);   // IO1
  hwpinctrl.PinSetup(PORTNUM_A,  7, qspipincfg | PINCFG_AF_10);   // IO2
  hwpinctrl.PinSetup(PORTNUM_A,  6, qspipincfg | PINCFG_AF_10);   // IO3

  fl_qspi.speed = 32000000;
  fl_qspi.multi_line_count = 4;
  fl_qspi.Init();
}
#endif

#if defined(BOARD_MIN_F401)

TGpioPin       fl_spi_cs_pin(PORTNUM_A, 4, false);
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  pin_led[0].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // by STM32 the CS must be controlled manually (in single SPI master operation)
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_AF_5);  // SPI1_NSS (CS)
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_AF_5);  // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 6, PINCFG_AF_5);  // SPI1_MISO
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_AF_5);  // SPI1_MOSI

  fl_spi.manualcspin = &fl_spi_cs_pin;
  fl_spi.speed = 8000000;
  fl_spi.Init(1);

  #if USE_DMA

    fl_spi_txdma.Init(2, 5, 3);  // dma2/stream5/ch3
    fl_spi_rxdma.Init(2, 0, 3);  // dma2/stream0/ch3

    fl_spi.DmaAssign(true,  &fl_spi_txdma);
    fl_spi.DmaAssign(false, &fl_spi_rxdma);

  #endif

}
#endif

