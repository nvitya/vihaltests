/*
 *  file:     board_pins.cpp (spiflash)
 *  brief:    SDCARD Test Board pins
 *  created:  2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 0;

TGpioPin  pin_led[MAX_LEDS] = { TGpioPin(), TGpioPin(), TGpioPin(), TGpioPin() };


#if SDCARD_SDMMC

  THwSdmmc      sd_mmc;
  TSdCardSdmmc  sdcard;

#elif SDCARD_SPI

  THwSpi        sd_spi;
  TSdCardSpi    sdcard;

#endif

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0 // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

#elif defined(BOARD_LONGAN_NANO)

TGpioPin       fl_spi_cs_pin;
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(PORTNUM_C, 13, true);
  pin_led[1].Assign(PORTNUM_A,  1, true);
  pin_led[2].Assign(PORTNUM_A,  2, true);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9, PINCFG_OUTPUT | PINCFG_AF_0);
  hwpinctrl.PinSetup(PORTNUM_A, 10, PINCFG_INPUT  | PINCFG_AF_0);
  conuart.Init(0); // USART0

  // the CS must be controlled manually (in single SPI master operation)
  fl_spi_cs_pin.Assign(PORTNUM_A, 4, false);
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_AF_0);  // SPI0_NSS (CS)
  // PINCFG_OUTPUT must be set for the AF outputs too!
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_OUTPUT | PINCFG_AF_0);  // SPI0_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 6, PINCFG_INPUT  | PINCFG_AF_0);  // SPI0_MISO (D1)
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_OUTPUT | PINCFG_AF_0);  // SPI0_MOSI (D0)

  fl_spi.manualcspin = &fl_spi_cs_pin;
  fl_spi.speed = 4000000; //SystemCoreClock / 4;
  fl_spi.Init(0);

  fl_spi_txdma.Init(0, 2);  // dma0/ch2
  fl_spi_rxdma.Init(0, 1);  // dma0/ch1

  fl_spi.DmaAssign(true,  &fl_spi_txdma);
  fl_spi.DmaAssign(false, &fl_spi_rxdma);

}

//-------------------------------------------------------------------------------
// Risc-V: RV64G
//-------------------------------------------------------------------------------

#elif defined(BOARD_MAIX_BIT)

void board_pins_init()
{
  pin_led_count = 3;
  // K210 specific pad routing, using GPIOHS
  hwpinctrl.PadSetup(12, FUNC_GPIOHS0, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(13, FUNC_GPIOHS1, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(14, FUNC_GPIOHS2, PINCFG_OUTPUT);
  // Assign the GPIOHS pins
  pin_led[0].Assign(0, 0, true);
  pin_led[1].Assign(0, 1, true);
  pin_led[2].Assign(0, 2, true);
  board_pins_init_leds();

  hwpinctrl.PadSetup(4, FUNC_UART3_RX, PINCFG_INPUT);
  hwpinctrl.PadSetup(5, FUNC_UART3_TX, PINCFG_OUTPUT);
  conuart.Init(3);

  // No pad setup required for the SPI3, it is tied to the external flash

  fl_spi.speed = 4000000; //SystemCoreClock / 4;
  fl_spi.Init(3);
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

#elif defined(BOARD_MIN_F103)

TGpioPin       fl_spi_cs_pin;
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // by STM32 the CS must be controlled manually (in single SPI master operation)
  fl_spi_cs_pin.Assign(PORTNUM_A, 4, false);
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_AF_5);  // SPI1_NSS (CS)
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_AF_5);  // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 6, PINCFG_AF_5);  // SPI1_MISO (D1)
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_AF_5);  // SPI1_MOSI (D0)

  fl_spi.manualcspin = &fl_spi_cs_pin;
  fl_spi.speed = SystemCoreClock / 3;
  fl_spi.Init(1);

  fl_spi_txdma.Init(1, 3, 0);  // dma1/ch3
  fl_spi_rxdma.Init(1, 2, 0);  // dma1/ch2

  fl_spi.DmaAssign(true,  &fl_spi_txdma);
  fl_spi.DmaAssign(false, &fl_spi_rxdma);
}

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

TGpioPin       sd_spi_cs_pin;
THwDmaChannel  sd_spi_txdma;
THwDmaChannel  sd_spi_rxdma;

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

  sd_spi_cs_pin.Assign(PORTNUM_A, 10, false);
  sd_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  hwpinctrl.PinSetup(PORTNUM_B, 3, PINCFG_AF_5);  // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_B, 4, PINCFG_AF_5);  // SPI1_MISO
  hwpinctrl.PinSetup(PORTNUM_B, 5, PINCFG_AF_5);  // SPI1_MOSI

  sd_spi.manualcspin = &sd_spi_cs_pin;
  sd_spi.speed = 8000000;
  sd_spi.Init(1);

  #if SD_SPI_USE_DMA

    sd_spi_txdma.Init(2, 5, 3);  // dma2/stream5/ch3
    sd_spi_rxdma.Init(2, 0, 3);  // dma2/stream0/ch3

    sd_spi.DmaAssign(true,  &sd_spi_txdma);
    sd_spi.DmaAssign(false, &sd_spi_rxdma);

  #endif
}

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

  // SDCARD Pins
  hwpinctrl.PinSetup(PORTNUM_C,  8, PINCFG_AF_12); // SDMMC_D0
  hwpinctrl.PinSetup(PORTNUM_C,  9, PINCFG_AF_12); // SDMMC_D1
  hwpinctrl.PinSetup(PORTNUM_C, 10, PINCFG_AF_12); // SDMMC_D2
  hwpinctrl.PinSetup(PORTNUM_C, 11, PINCFG_AF_12); // SDMMC_D3
  hwpinctrl.PinSetup(PORTNUM_C, 12, PINCFG_AF_12); // SDMMC_CK
  hwpinctrl.PinSetup(PORTNUM_D,  2, PINCFG_AF_12); // SDMMC_CMD

  sd_mmc.Init();
}

#elif defined(BOARD_DISCOVERY_H747)

#ifndef CORE_CM7
  #warning "define CORE_CM7 !!!!"
#endif

void board_pins_init()
{
  pin_led_count = 4;
  pin_led[0].Assign(PORTNUM_I,  12, true);
  pin_led[1].Assign(PORTNUM_I,  13, true);
  pin_led[2].Assign(PORTNUM_I,  14, true);
  pin_led[3].Assign(PORTNUM_I,  15, true);
  board_pins_init_leds();

  // turn off LCD backlight:
  hwpinctrl.PinSetup(PORTNUM_J, 12, PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7);
  conuart.Init(1); // USART1

  // SDCARD Pins
  hwpinctrl.PinSetup(PORTNUM_C,  8, PINCFG_AF_12); // SDMMC_D0
  hwpinctrl.PinSetup(PORTNUM_C,  9, PINCFG_AF_12); // SDMMC_D1
  hwpinctrl.PinSetup(PORTNUM_C, 10, PINCFG_AF_12); // SDMMC_D2
  hwpinctrl.PinSetup(PORTNUM_C, 11, PINCFG_AF_12); // SDMMC_D3
  hwpinctrl.PinSetup(PORTNUM_C, 12, PINCFG_AF_12); // SDMMC_CK
  hwpinctrl.PinSetup(PORTNUM_D,  2, PINCFG_AF_12); // SDMMC_CMD

  // card detect: I8

  sd_mmc.Init();
}

#elif defined(BOARD_DEV_STM32F407ZE)

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_F,  9, true);
  pin_led[1].Assign(PORTNUM_F, 10, true);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7);  // USART1_RX
  conuart.Init(1);

  // SDCARD Pins
  hwpinctrl.PinSetup(PORTNUM_C,  8, PINCFG_AF_12); // SDMMC_D0
  hwpinctrl.PinSetup(PORTNUM_C,  9, PINCFG_AF_12); // SDMMC_D1
  hwpinctrl.PinSetup(PORTNUM_C, 10, PINCFG_AF_12); // SDMMC_D2
  hwpinctrl.PinSetup(PORTNUM_C, 11, PINCFG_AF_12); // SDMMC_D3
  hwpinctrl.PinSetup(PORTNUM_C, 12, PINCFG_AF_12); // SDMMC_CK
  hwpinctrl.PinSetup(PORTNUM_D,  2, PINCFG_AF_12); // SDMMC_CMD

  sdcard.Init();
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

#elif defined(BOARD_MIN_F401) || defined(BOARD_MIBO48_STM32F303) || defined(BOARD_MIBO64_STM32F405)

TGpioPin       fl_spi_cs_pin(PORTNUM_A, 4, false);
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

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

  #if FL_SPI_USE_DMA

    fl_spi_txdma.Init(2, 5, 3);  // dma2/stream5/ch3
    fl_spi_rxdma.Init(2, 0, 3);  // dma2/stream0/ch3

    fl_spi.DmaAssign(true,  &fl_spi_txdma);
    fl_spi.DmaAssign(false, &fl_spi_rxdma);

  #endif

}

#elif defined(BOARD_ARDUINO_DUE)

TGpioPin       fl_spi_cs_pin(PORTNUM_A, 18, false); // D21/SCL = CS
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();

  // UART - On the Arduino programmer interface
  hwpinctrl.PinSetup(PORTNUM_A, 8, PINCFG_INPUT | PINCFG_AF_0);  // UART_RXD
  hwpinctrl.PinSetup(PORTNUM_A, 9, PINCFG_OUTPUT | PINCFG_AF_0); // UART_TXD
  conuart.Init(0);  // UART

  // the CS must be controlled manually
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  #define USE_USART0  1 // else SPI0

  #if USE_USART0
    // USART0 Setup

    hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_OUTPUT | PINCFG_AF_A);  // D18: USART0 TXD = MOSI
    hwpinctrl.PinSetup(PORTNUM_A, 10, PINCFG_INPUT  | PINCFG_AF_A);  // D19: USART0 RXD = MISO
    hwpinctrl.PinSetup(PORTNUM_A, 17, PINCFG_OUTPUT | PINCFG_AF_B);  // D20: USART0 SCK

    fl_spi.manualcspin = &fl_spi_cs_pin;
    fl_spi.speed = 20000000;
    fl_spi.Init(0x100);  // + 0x100 means use the USARTx instead of SPIx

    #if FL_SPI_USE_DMA
      // the peripheral ids can be found in "Table 22-2." in the ATSAM3X8E Datasheet
      fl_spi_txdma.Init(3, 11);  // perid 11 = USART0_TX
      fl_spi_rxdma.Init(4, 12);  // perid 12 = USART0_RX

      fl_spi.DmaAssign(true,  &fl_spi_txdma);
      fl_spi.DmaAssign(false, &fl_spi_rxdma);
    #endif

  #else
    // SPI0 setup

    hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_OUTPUT | PINCFG_AF_A);  // MOSI
    hwpinctrl.PinSetup(PORTNUM_A, 26, PINCFG_INPUT  | PINCFG_AF_A);  // MISO
    hwpinctrl.PinSetup(PORTNUM_A, 27, PINCFG_OUTPUT | PINCFG_AF_A);  // SCK

    fl_spi.manualcspin = &fl_spi_cs_pin;
    fl_spi.speed = 4000000;
    fl_spi.Init(0);

    #if FL_SPI_USE_DMA
      // the peripheral ids can be found in "Table 22-2." in the ATSAM3X8E Datasheet
      fl_spi_txdma.Init(3, 1);  // perid 1 = SPI0_TX
      fl_spi_rxdma.Init(4, 2);  // perid 2 = SPI0_RX

      fl_spi.DmaAssign(true,  &fl_spi_txdma);
      fl_spi.DmaAssign(false, &fl_spi_rxdma);
    #endif
  #endif
}

#elif defined(BOARD_MIBO64_ATSAM4S)

TGpioPin       fl_spi_cs_pin(PORTNUM_A, 11, false);
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  conuart.Init(0);

  // SPI0 setup

  unsigned pinflags = PINCFG_OUTPUT | PINCFG_AF_A | PINCFG_PULLUP;

  // the CS must be controlled manually
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //hwpinctrl.PinSetup(PORTNUM_A, 11, pinflags);  // CS
  hwpinctrl.PinSetup(PORTNUM_A, 13, pinflags);  // MOSI
  hwpinctrl.PinSetup(PORTNUM_A, 12, pinflags);  // MISO
  hwpinctrl.PinSetup(PORTNUM_A, 14, pinflags);  // SCK

  fl_spi.manualcspin = &fl_spi_cs_pin;
  fl_spi.speed = 30000000;
  fl_spi.Init(0);

  #if FL_SPI_USE_DMA

    fl_spi.PdmaInit(true,  &fl_spi_txdma);
    // alternative: fl_spi_txdma.InitPeriphDma(true,  fl_spi.regs, fl_spi.usartregs);
    fl_spi.PdmaInit(false, &fl_spi_rxdma);
    // alternative: fl_spi_rxdma.InitPeriphDma(false, fl_spi.regs, fl_spi.usartregs);

    //fl_spi.DmaAssign(true,  &fl_spi_txdma);
    //fl_spi.DmaAssign(false, &fl_spi_rxdma);

  #endif
}

#elif defined(BOARD_MIBO64_ATSAME5X)

TGpioPin       fl_spi_cs_pin(PORTNUM_A, 18, false);
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 1, false);
  board_pins_init_leds();

  // SERCOM0
  hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_OUTPUT | PINCFG_AF_3);  // PAD[0] = TX
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_INPUT  | PINCFG_AF_3);  // PAD[1] = RX
  conuart.Init(0);

  #define USE_QSPI 1
  #if USE_QSPI
    // at the ATSAM_V2 the pins and DMA initialization is done internally:
    //  A8  = DATA0
    //  A9  = DATA1
    //  A10 = DATA2
    //  A11 = DATA3
    //  B10 = SCK
    //  B11 = CS
    // You can select the used DMA chennels with the txdmachannel, rxdmachannel
    fl_qspi.txdmachannel = 5;
    fl_qspi.rxdmachannel = 6;
    fl_qspi.multi_line_count = 4;
    fl_qspi.speed = 60000000;
    fl_qspi.Init();

  #else
    // SERCOM3

    unsigned pinflags = PINCFG_AF_D | PINCFG_PULLUP | PINCFG_DRIVE_STRONG;

    hwpinctrl.PinSetup(PORTNUM_A, 16, pinflags);  // SERCOM3_PAD1: SCK
    hwpinctrl.PinSetup(PORTNUM_A, 17, pinflags);  // SERCOM3_PAD0: MOSI
    //hwpinctrl.PinSetup(PORTNUM_A, 18, pinflags);  // SERCOM3_PAD2: CS
    hwpinctrl.PinSetup(PORTNUM_A, 19, pinflags);  // SERCOM3_PAD3: MISO

    fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

    fl_spi.manualcspin = &fl_spi_cs_pin;
    //fl_spi.datasample_late = true;
    fl_spi.speed = 20000000;
    fl_spi.Init(3);

    #if FL_SPI_USE_DMA
      fl_spi_txdma.Init(14, SERCOM3_DMAC_ID_TX);
      fl_spi_rxdma.Init(15, SERCOM3_DMAC_ID_RX);

      fl_spi.DmaAssign(true,  &fl_spi_txdma);
      fl_spi.DmaAssign(false, &fl_spi_rxdma);
    #endif
  #endif
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

  hwpinctrl.PinSetup(PORTNUM_A, 28, PINCFG_AF_2); // MCCDA (CMD)
  hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_AF_3); // MCCK
  hwpinctrl.PinSetup(PORTNUM_A, 30, PINCFG_AF_2); // MCDA0
  hwpinctrl.PinSetup(PORTNUM_A, 31, PINCFG_AF_2); // MCDA1
  hwpinctrl.PinSetup(PORTNUM_A, 26, PINCFG_AF_2); // MCDA2
  hwpinctrl.PinSetup(PORTNUM_A, 27, PINCFG_AF_2); // MCDA3

  hwpinctrl.PinSetup(PORTNUM_C, 16, PINCFG_INPUT | PINCFG_PULLUP); // Card detect input

  sd_mmc.Init();
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

  // FPGA output clock
  hwpinctrl.PinSetup(PORTNUM_A,  6,  PINCFG_OUTPUT | PINCFG_AF_1);  // PCK0 = FPGA.CLK_IN
  PMC->PMC_SCER = (1 << 8); // enable PCK0
  PMC->PMC_PCK[0] = 0
    | (1 << 0)  // CSS(3): 1 = MAIN CLK (12 MHz)
    | (0 << 4)  // PRES(8): divisor - 1
  ;

  // do not clear the FPGA config
  hwpinctrl.PinSetup(PORTNUM_C,  9,  PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);  // FPGA_CFG

  // Keep the ETH in reset
  hwpinctrl.PinSetup(PORTNUM_C, 10,  PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  // SDCARD
  hwpinctrl.PinSetup(PORTNUM_A, 28, PINCFG_AF_2); // MCCDA
  hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_AF_3); // MCCK
  hwpinctrl.PinSetup(PORTNUM_A, 30, PINCFG_AF_2); // MCDA0
  hwpinctrl.PinSetup(PORTNUM_A, 31, PINCFG_AF_2); // MCDA1
  hwpinctrl.PinSetup(PORTNUM_A, 26, PINCFG_AF_2); // MCDA2
  hwpinctrl.PinSetup(PORTNUM_A, 27, PINCFG_AF_2); // MCDA3

  sd_mmc.Init();
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

  // at the ATSAM the pins and DMA initialization is done internally.
  // You can select the used DMA chennels with the txdmachannel, rxdmachannel
  fl_qspi.txdmachannel = 5;
  fl_qspi.rxdmachannel = 6;
  fl_qspi.multi_line_count = 4;
  fl_qspi.speed = 60000000;  // that's 240 MBit/s ...
  fl_qspi.Init();
}

// RP

#elif defined(BOARD_RPI_PICO)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 25, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  conuart.Init(0);

  // because of the transfers are unidirectional the same DMA channel can be used here:
  fl_qspi.txdmachannel = 7;
  fl_qspi.rxdmachannel = 7;
  // for read speeds over 24 MHz dual or quad mode is required.
  // the writes are forced to single line mode (SSS) because the RP does not support SSM mode at write
  fl_qspi.multi_line_count = 4;
  fl_qspi.speed = 66000000;     // this is preatty high speed, but seems to work
  fl_qspi.Init();
}


#else
  #error "Define board_pins_init here"
#endif

