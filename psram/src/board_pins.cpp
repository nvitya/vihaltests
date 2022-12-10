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

// one of these should be initialized:
THwSpi    fl_spi;
THwQspi   fl_qspi;

TGpioPin       fl_spi_cs_pin;
THwDmaChannel  fl_spi_txdma;
THwDmaChannel  fl_spi_rxdma;

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0 // to use elif everywhere

#elif defined(BOARD_MIN_F103)

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

  #define USE_QSPI 1

  #if USE_QSPI
    uint32_t qspipincfg = PINCFG_SPEED_FAST;

    hwpinctrl.PinSetup(PORTNUM_B, 11, qspipincfg | PINCFG_AF_10);   // NCS
    hwpinctrl.PinSetup(PORTNUM_B, 10, qspipincfg | PINCFG_AF_10);   // CLK

    hwpinctrl.PinSetup(PORTNUM_B,  1, qspipincfg | PINCFG_AF_10);   // IO0
    hwpinctrl.PinSetup(PORTNUM_B,  0, qspipincfg | PINCFG_AF_10);   // IO1
    hwpinctrl.PinSetup(PORTNUM_A,  7, qspipincfg | PINCFG_AF_10);   // IO2
    hwpinctrl.PinSetup(PORTNUM_A,  6, qspipincfg | PINCFG_AF_10);   // IO3

    fl_qspi.speed = SystemCoreClock / 2;
    fl_qspi.multi_line_count = 4;
    fl_qspi.idleclk_high = false;
    fl_qspi.datasample_late = false;
    fl_qspi.Init();
  #else
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

    fl_spi_txdma.Init(1, 2, 11);  // 11 = SPI1_TX
    fl_spi_rxdma.Init(1, 3, 10);  // 10 = SPI1_RX

    fl_spi.DmaAssign(true,  &fl_spi_txdma);
    fl_spi.DmaAssign(false, &fl_spi_rxdma);
  #endif
}

#elif defined(BOARD_MIN_F401) || defined(BOARD_MIBO48_STM32F303) || defined(BOARD_MIBO64_STM32F405)

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
  hwpinctrl.PinSetup(PORTNUM_A, 6, PINCFG_AF_5);  // SPI1_MISO
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_AF_5);  // SPI1_MOSI

  fl_spi.manualcspin = &fl_spi_cs_pin;
  fl_spi.speed = 60000000;
  fl_spi.Init(1);

  fl_spi_txdma.Init(2, 5, 3);  // dma2/stream5/ch3
  fl_spi_rxdma.Init(2, 0, 3);  // dma2/stream0/ch3

  fl_spi.DmaAssign(true,  &fl_spi_txdma);
  fl_spi.DmaAssign(false, &fl_spi_rxdma);
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

  // the CS must be controlled manually
  fl_spi_cs_pin.Assign(PORTNUM_A, 18, false); // D21/SCL = CS
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

    // the peripheral ids can be found in "Table 22-2." in the ATSAM3X8E Datasheet
    fl_spi_txdma.Init(3, 11);  // perid 11 = USART0_TX
    fl_spi_rxdma.Init(4, 12);  // perid 12 = USART0_RX

    fl_spi.DmaAssign(true,  &fl_spi_txdma);
    fl_spi.DmaAssign(false, &fl_spi_rxdma);

  #else
    // SPI0 setup

    hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_OUTPUT | PINCFG_AF_A);  // MOSI
    hwpinctrl.PinSetup(PORTNUM_A, 26, PINCFG_INPUT  | PINCFG_AF_A);  // MISO
    hwpinctrl.PinSetup(PORTNUM_A, 27, PINCFG_OUTPUT | PINCFG_AF_A);  // SCK

    fl_spi.manualcspin = &fl_spi_cs_pin;
    fl_spi.speed = 4000000;
    fl_spi.Init(0);

    // the peripheral ids can be found in "Table 22-2." in the ATSAM3X8E Datasheet
    fl_spi_txdma.Init(3, 1);  // perid 1 = SPI0_TX
    fl_spi_rxdma.Init(4, 2);  // perid 2 = SPI0_RX

    fl_spi.DmaAssign(true,  &fl_spi_txdma);
    fl_spi.DmaAssign(false, &fl_spi_rxdma);
  #endif
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

  // SPI0 setup

  unsigned pinflags = PINCFG_OUTPUT | PINCFG_AF_A | PINCFG_PULLUP;

  // the CS must be controlled manually
  fl_spi_cs_pin.Assign(PORTNUM_A, 11, false);
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  //hwpinctrl.PinSetup(PORTNUM_A, 11, pinflags);  // CS
  hwpinctrl.PinSetup(PORTNUM_A, 13, pinflags);  // MOSI
  hwpinctrl.PinSetup(PORTNUM_A, 12, pinflags);  // MISO
  hwpinctrl.PinSetup(PORTNUM_A, 14, pinflags);  // SCK

  fl_spi.manualcspin = &fl_spi_cs_pin;
  fl_spi.speed = 30000000;
  fl_spi.Init(0);

  fl_spi.PdmaInit(true,  &fl_spi_txdma);
  // alternative: fl_spi_txdma.InitPeriphDma(true,  fl_spi.regs, fl_spi.usartregs);
  fl_spi.PdmaInit(false, &fl_spi_rxdma);
  // alternative: fl_spi_rxdma.InitPeriphDma(false, fl_spi.regs, fl_spi.usartregs);

  //fl_spi.DmaAssign(true,  &fl_spi_txdma);
  //fl_spi.DmaAssign(false, &fl_spi_rxdma);
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

    // the QSPI unit in the MCU can not provide proper timing / control in quad mode for this device
    fl_qspi.multi_line_count = 1;
    fl_qspi.speed = SystemCoreClock / 2;
    fl_qspi.Init();

  #else
    // SERCOM3

    unsigned pinflags = PINCFG_AF_D | PINCFG_PULLUP | PINCFG_DRIVE_STRONG;

    hwpinctrl.PinSetup(PORTNUM_A, 16, pinflags);  // SERCOM3_PAD1: SCK
    hwpinctrl.PinSetup(PORTNUM_A, 17, pinflags);  // SERCOM3_PAD0: MOSI
    //hwpinctrl.PinSetup(PORTNUM_A, 18, pinflags);  // SERCOM3_PAD2: CS
    hwpinctrl.PinSetup(PORTNUM_A, 19, pinflags);  // SERCOM3_PAD3: MISO

    fl_spi_cs_pin.Assign(PORTNUM_A, 18, false);
    fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

    fl_spi.manualcspin = &fl_spi_cs_pin;
    //fl_spi.datasample_late = true;
    fl_spi.speed = 20000000;
    fl_spi.Init(3);

    fl_spi_txdma.Init(14, SERCOM3_DMAC_ID_TX);
    fl_spi_rxdma.Init(15, SERCOM3_DMAC_ID_RX);

    fl_spi.DmaAssign(true,  &fl_spi_txdma);
    fl_spi.DmaAssign(false, &fl_spi_rxdma);
  #endif
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

  unsigned pinflags = PINCFG_AF_1 | PINCFG_SPEED_FAST | PINCFG_DRIVE_STRONG;

  hwpinctrl.PinSetup(0, 3, pinflags);  // SPI0_TX
  hwpinctrl.PinSetup(0, 4, pinflags);  // SPI0_RX
  //hwpinctrl.PinSetup(0, 5, pinflags);  // SPI0_CS
  hwpinctrl.PinSetup(0, 6, pinflags);  // SPI0_CLK

  fl_spi_cs_pin.Assign(0, 5, false);
  fl_spi_cs_pin.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  fl_spi.manualcspin = &fl_spi_cs_pin;
  //fl_spi.datasample_late = true;
  fl_spi.speed = 33000000;
  fl_spi.Init(0);

  fl_spi_txdma.Init(2, DREQ_SPI0_TX);
  fl_spi_rxdma.Init(3, DREQ_SPI0_RX);

  fl_spi.DmaAssign(true,  &fl_spi_txdma);
  fl_spi.DmaAssign(false, &fl_spi_rxdma);
}


#else
  #error "Define board_pins_init here"
#endif

