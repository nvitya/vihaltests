/*
 *  file:     board_pins.cpp (uart)
 *  brief:    UART Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "hwi2c.h"
#include "oleddisp_i2c.h"
#include "monolcd_spi.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 1;

THwI2c          i2c;
THwDmaChannel   i2c_txdma;
THwDmaChannel   i2c_rxdma;

THwSpi          spi;
THwDmaChannel   spi_txdma;

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

uint8_t disp_buf[DISPLAY_WIDTH*DISPLAY_HEIGHT >> 3];

#ifdef DISP_I2C_OLED

TOledDisp_i2c   disp;

void init_display()
{
  disp.pi2c = &i2c;
  //disp.i2caddress = 0x3D;
  disp.rotation = 0;
  disp.Init(OLED_CTRL_SSD1306, DISPLAY_WIDTH, DISPLAY_HEIGHT, &disp_buf[0]);
}

#elif defined(DISP_SPI_MONO)

TMonoLcd_spi    disp;


void init_display()
{
  disp.pspi = &spi;
  disp.rotation = DISPLAY_ROTATION;
  #ifdef DISPLAY_CONTRAST
    disp.contrast = DISPLAY_CONTRAST;
  #endif
  disp.Init(DISPLAY_CTRL, DISPLAY_WIDTH, DISPLAY_HEIGHT, &disp_buf[0]);
}

#elif defined(DISP_BITBANG)

TMonoLcd_bb     disp;

void init_display()
{
  disp.rotation = DISPLAY_ROTATION;
  #ifdef DISPLAY_CONTRAST
    disp.contrast = DISPLAY_CONTRAST;
  #endif
  disp.Init(DISPLAY_CTRL, DISPLAY_WIDTH, DISPLAY_HEIGHT, &disp_buf[0]);
}

#else

  #error "unhandled interface type"

#endif

#if 0  // to use elif everywhere

//----------------------------------------------------------------------------------------------------
// SPI DISPLAYS with SMALLER MCUs
//----------------------------------------------------------------------------------------------------

#elif    defined(BOARD_MIN_F103) \
      || defined(BOARD_MIBO48_STM32F303)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // I2C1
  // open drain mode have to be used, otherwise it won't work
  // External pull-ups are required !
  hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_AF_4 | PINCFG_OPENDRAIN | PINCFG_SPEED_FAST); // I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_AF_4 | PINCFG_OPENDRAIN | PINCFG_SPEED_FAST); // I2C1_SDA
  i2c.speed = 100000;
  i2c.Init(1); // I2C1

  #if 1
    i2c_txdma.Init(1, 6, 0);  // dma1,ch6 = I2C1 TX
    i2c_rxdma.Init(1, 7, 0);  // dma1,ch5 = I2C1 RX

    i2c.DmaAssign(true,  &i2c_txdma);
    i2c.DmaAssign(false, &i2c_rxdma);
  #endif

  init_display();
}

#elif    defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  #ifdef DISP_I2C_OLED
    // I2C1
    // open drain mode have to be used, otherwise it won't work
    hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SCL
    hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SDA
    i2c.speed = 100000; // 100 kHz
    i2c.Init(1); // I2C1

    #if 1
      i2c_txdma.Init(1, 6, 1);  // dma1,stream6,ch1 = I2C1 TX
      i2c_rxdma.Init(1, 5, 1);  // dma1,stream5,ch1 = I2C1 RX

      i2c.DmaAssign(true,  &i2c_txdma);
      i2c.DmaAssign(false, &i2c_rxdma);
    #endif

  #elif defined(DISP_SPI_MONO)

    // LCD control
    disp.pin_cs.Assign(PORTNUM_A, 0, false);
    disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

    disp.pin_reset.Assign(PORTNUM_A, 1, false);
    disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

    disp.pin_cd.Assign(PORTNUM_A, 2, false);  // aka. "RS"
    disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

    // SPI1
    hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_OUTPUT | PINCFG_AF_5); // SPI1_SCK
    hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_OUTPUT | PINCFG_AF_5); // SPI1_MOSI

    spi.idleclk_high = false;
    spi.datasample_late = false;
    spi.speed = 4000000; // 8 MHz
    spi.Init(1);

    spi_txdma.Init(2, 5, 3); // SPI1_TX, alternative 2, 3, 3
    spi.DmaAssign(true, &spi_txdma);

  #elif defined(DISP_BITBANG)

    disp.pin_clk.Assign(PORTNUM_A, 5, false);
    disp.pin_clk.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

    disp.pin_din.Assign(PORTNUM_A, 7, false);
    disp.pin_din.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

    disp.pin_ce.Assign(PORTNUM_A, 0, false);
    disp.pin_ce.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

    disp.pin_reset.Assign(PORTNUM_A, 1, false);
    disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B0: RESET

  #else
    #error "unhandled display type"
  #endif

  init_display();
}

#elif defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART2
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A,  3,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(2);

  // I2C1
  // open drain mode have to be used, otherwise it won't work
  // WARNING: 1. B6 is not connected to I2C anymore
  //          2. B8 is shared to BOOT0 pin, so when it is pulled up the device does not start from flash
  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SDA
  hwpinctrl.PinSetup(PORTNUM_A, 15, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SCL
  i2c.speed = 100000; // 100 kHz
  i2c.Init(1); // I2C1

#if 1
  i2c_txdma.Init(1, 6, 17);  // dmamux17 = I2C1 TX
  i2c_rxdma.Init(1, 5, 16);  // dmamux16 = I2C1 RX

  i2c.DmaAssign(true,  &i2c_txdma);
  i2c.DmaAssign(false, &i2c_rxdma);
#endif

  init_display();
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

  // I2C0
  hwpinctrl.PinSetup(0, 16, PINCFG_AF_3 | PINCFG_PULLUP); // I2C0_SDA
  hwpinctrl.PinSetup(0, 17, PINCFG_AF_3 | PINCFG_PULLUP); // I2C0_SCL
  //i2c.speed = 400000;
  i2c.speed = 1000000;
  i2c.Init(0);

  #if 0
    i2c_txdma.Init(2, DREQ_I2C0_TX);
    i2c_rxdma.Init(3, DREQ_I2C0_RX);

    i2c.DmaAssign(true,  &i2c_txdma);
    i2c.DmaAssign(false, &i2c_rxdma);
  #endif

  init_display();
}

#elif defined(BOARD_WEMOS_C3MINI)

#if SPI_SELF_FLASHING
  THwQspi    fl_qspi;
  TSpiFlash  spiflash;
#endif

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(0, 7, false);
  board_pins_init_leds();

  // for now, external UART adapter is required

  //hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);

  #if SPI_SELF_FLASHING
    // the normal SPI flash (SPI1) is coupled to the "SPIxxx" pins
    hwpinctrl.PadSetup(PAD_GPIO12, SPIWP_OUT_IDX,   PINCFG_OUTPUT | PINCFG_AF_0);         // CS0,  AF_0 = direct routing
    hwpinctrl.PadSetup(PAD_GPIO13, SPIHD_OUT_IDX,   PINCFG_OUTPUT | PINCFG_AF_0);         // CS0,  AF_0 = direct routing
    hwpinctrl.PadSetup(PAD_GPIO14, SPICS0_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_0);         // CS0,  AF_0 = direct routing
    hwpinctrl.PadSetup(PAD_GPIO17, SPIQ_IN_IDX,     PINCFG_INPUT  | PINCFG_AF_0);         // MISO, AF_0 = direct routing
    hwpinctrl.PadSetup(PAD_GPIO16, SPID_OUT_IDX,    PINCFG_OUTPUT | PINCFG_AF_0);         // MOSI, AF_0 = direct routing
    hwpinctrl.PadSetup(PAD_GPIO15, SPICLK_OUT_IDX,  PINCFG_OUTPUT | PINCFG_AF_0);         // CLK,  AF_0 = direct routing

    fl_qspi.speed = 20000000;
    fl_qspi.multi_line_count = 4;
    fl_qspi.Init();

    spiflash.qspi = &fl_qspi;
    spiflash.has4kerase = true;
    spiflash.Init();
  #endif

  // I2C0
  hwpinctrl.PadSetup(PAD_GPIO1, I2CEXT0_SDA_OUT_IDX, PINCFG_OUTPUT | PINCFG_OPENDRAIN);  // SDA
  hwpinctrl.PadInput(PAD_GPIO1, I2CEXT0_SDA_IN_IDX);
  hwpinctrl.PadSetup(PAD_GPIO0, I2CEXT0_SCL_OUT_IDX, PINCFG_OUTPUT | PINCFG_OPENDRAIN);  // SCL
  hwpinctrl.PadInput(PAD_GPIO0, I2CEXT0_SCL_IN_IDX);
  //i2c.speed = 1000000;  // 1 MBit/s works as well
  i2c.speed =  400000;  // 400 kBit/s
  i2c.Init(0);

  // no DMA support for I2C by Espressif, but it has at least a 32 byte fifo

  init_display();
}



#else
  #error "Define board_pins_init here"
#endif

