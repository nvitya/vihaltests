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

/* NOTE:
     for direct GPIO pin definitions is simpler to define with port and pin number:

       TGpioPin  pin_mygpio(PORTNUM_C, 13, false);

     but don't forget to initialize it in the setup code:

       pin_mygpio.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
*/

// one of these should be initialized:
THwSpi    fl_spi;
THwQspi   fl_qspi;

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0 // to use elif everywhere

#elif defined(MCUSF_VRV100) //BOARD_VRV100_441)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 0, false);
  board_pins_init_leds();

  conuart.Init(1); // UART1

  fl_spi.speed = 8000000;
  fl_spi.Init(1); // flash
}

#elif defined(BOARD_MIBO48_STM32G473) || defined(BOARD_MIBO48_STM32F303) || defined(BOARD_MIBO64_STM32F405)

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

  // at the ATSAM the pins and DMA initialization is done internally.
  // You can select the used DMA chennels with the txdmachannel, rxdmachannel
  fl_qspi.txdmachannel = 5;
  fl_qspi.rxdmachannel = 6;
  fl_qspi.multi_line_count = 4;
  fl_qspi.speed = 60000000;  // that's 240 MBit/s ...
  fl_qspi.Init();
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

  // PIN and DMA setup is done internally in the Init(), because there are no alternavives
  fl_qspi.multi_line_count = 2;  // in my board the original chip was replaced, to a small dual one
  fl_qspi.speed = 30000000;
  fl_qspi.Init();
}

#else
  #error "Define board_pins_init here"
#endif

