/*
 *  file:     board_pins.cpp (uart)
 *  brief:    UART Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "hwi2c.h"
#include "hwpwm.h"

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

uint8_t cdisp_chars[DISPLAY_WIDTH * DISPLAY_HEIGHT];
uint8_t cdisp_changemap[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

#if defined(DISP_16X2_I2C)

TCharLcd_i2c   disp;

void init_display()
{
  disp.pi2c = &i2c;
  disp.Init(CHLCD_CTRL_HD44780, DISPLAY_WIDTH, DISPLAY_HEIGHT, &cdisp_chars[0], &cdisp_changemap[0]);
}

#elif defined(DISP_16X2_BB4)

TCharLcd_bb4   disp;
THwPwmChannel  conpwm;  // contrast PWM

void init_display()
{
  conpwm.Enable();
  conpwm.SetOnClocks((conpwm.periodclocks * 30) / 100);

  disp.Init(CHLCD_CTRL_ST7066U, DISPLAY_WIDTH, DISPLAY_HEIGHT, &cdisp_chars[0], &cdisp_changemap[0]);
}

#else

  #error "unhandled interface type"

#endif

#if 0  // to use elif everywhere

//----------------------------------------------------------------------------------------------------
// SPI DISPLAYS with SMALLER MCUs
//----------------------------------------------------------------------------------------------------

#elif defined(BOARD_MIN_F103)  \
      || defined(BOARD_MIBO48_STM32F303) \
      || defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411)

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
  // Opendrain allows 5V I2C signal driving, because the pins are 5V tolerant, no level shifter required this way
  // some modules have built-in pullups

  hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SDA
  i2c.Init(1); // I2C1

  #if 1
    i2c_txdma.Init(1, 6, 0);  // dma1,ch6 = I2C1 TX
    i2c_rxdma.Init(1, 7, 0);  // dma1,ch5 = I2C1 RX

    i2c.DmaAssign(true,  &i2c_txdma);
    i2c.DmaAssign(false, &i2c_rxdma);
  #endif

  #if defined(DISP_16X2_BB4)

    disp.pin_en.Assign(PORTNUM_B, 0, false);
    disp.pin_rs.Assign(PORTNUM_B, 1, false);
    disp.pin_rw.Assign(PORTNUM_B, 2, false);

    disp.pin_data[0].Assign(PORTNUM_A, 4, false);
    disp.pin_data[1].Assign(PORTNUM_A, 5, false);
    disp.pin_data[2].Assign(PORTNUM_A, 6, false);
    disp.pin_data[3].Assign(PORTNUM_A, 7, false);

    hwpinctrl.PinSetup(PORTNUM_B, 3, PINCFG_AF_1); // select TIM2_CH2

    conpwm.frequency = 10000;
    conpwm.Init(2, 2, 0); // Timer2/Ch2/output0
    conpwm.Enable();
    conpwm.SetOnClocks((conpwm.periodclocks * 30) / 100);

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

  #if 0
    i2c_txdma.Init(1, 6, 17);  // dmamux17 = I2C1 TX
    i2c_rxdma.Init(1, 5, 16);  // dmamux16 = I2C1 RX

    i2c.DmaAssign(true,  &i2c_txdma);
    i2c.DmaAssign(false, &i2c_rxdma);
  #endif

  #if defined(DISP_16X2_BB4)

    disp.pin_en.Assign(PORTNUM_B, 0, false);
    disp.pin_rs.Assign(PORTNUM_B, 1, false);
    disp.pin_rw.Assign(PORTNUM_B, 2, false);

    disp.pin_data[0].Assign(PORTNUM_A, 4, false);
    disp.pin_data[1].Assign(PORTNUM_A, 5, false);
    disp.pin_data[2].Assign(PORTNUM_A, 6, false);
    disp.pin_data[3].Assign(PORTNUM_A, 7, false);

    hwpinctrl.PinSetup(PORTNUM_B, 3, PINCFG_AF_1); // select TIM2_CH2
    conpwm.frequency = 10000;
    conpwm.Init(2, 2, 0); // Timer2/Ch2/output0

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
  i2c.Init(0);

  #if 1
    i2c_txdma.Init(2, DREQ_I2C0_TX);
    i2c_rxdma.Init(3, DREQ_I2C0_RX);

    i2c.DmaAssign(true,  &i2c_txdma);
    i2c.DmaAssign(false, &i2c_rxdma);
  #endif


  #if defined(DISP_16X2_BB4)

    disp.pin_en.Assign(0, 18, false);
    disp.pin_rs.Assign(0, 19, false);
    disp.pin_rw.Assign(0, 20, false);

    disp.pin_data[0].Assign(0, 10, false);
    disp.pin_data[1].Assign(0, 11, false);
    disp.pin_data[2].Assign(0, 12, false);
    disp.pin_data[3].Assign(0, 13, false);

    hwpinctrl.PinSetup(0, 13,  PINCFG_OUTPUT | PINCFG_AF_4);  // PWM6_B
    conpwm.frequency = 10000;
    conpwm.Init(0, 6, 1);
    conpwm.Enable();
    conpwm.SetOnClocks((conpwm.periodclocks * 30) / 100);

  #endif

  init_display();
}

#else
  #error "Define board_pins_init here"
#endif

