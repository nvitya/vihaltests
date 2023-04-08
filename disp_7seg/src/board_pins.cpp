/*
 *  file:     board_pins.cpp (uart)
 *  brief:    UART Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "hwuart.h"
#include "ledandkey.h"

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

#if 0 == DISP_SEL

  TLedAndKey  disp;

#elif 1 == DISP_SEL

  TTimeDisp_tm1637  disp;

#elif 2 == DISP_SEL

  TMax7219  disp;

#elif 3 == DISP_SEL

  Td7s_595  disp;

#else
  #error "Unhandled display type"
#endif


#if 0  // to use elif everywhere

#elif    defined(BOARD_MIN_F103) \
      || defined(BOARD_MIBO48_STM32F303) \
      || defined(BOARD_MIN_F401) \
      || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  #if defined(BOARD_MIBO48_STM32G473)
    // Turn off the USB-C power delivery pull down functionality on B6:
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR3 |= PWR_CR3_UCPD_DBDIS;
  #endif

  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  #if 0 == DISP_SEL
    disp.controller.stb_pin.Assign(PORTNUM_B, 5, false);
    disp.controller.clk_pin.Assign(PORTNUM_B, 6, false);
    disp.controller.dio_pin.Assign(PORTNUM_B, 7, false);
    disp.Init();
  #elif 1 == DISP_SEL
    disp.controller.clk_pin.Assign(PORTNUM_B, 6, false);
    disp.controller.dio_pin.Assign(PORTNUM_B, 7, false);
    disp.Init();
  #elif 2 == DISP_SEL
    disp.dio_pin.Assign(PORTNUM_B, 5, false);
    disp.cs_pin.Assign( PORTNUM_B, 6, false);
    disp.clk_pin.Assign(PORTNUM_B, 7, false);
    disp.Init();
  #elif 3 == DISP_SEL
    disp.pin_sclk.Assign(PORTNUM_B, 5, false);
    disp.pin_rclk.Assign(PORTNUM_B, 6, false);
    disp.pin_din.Assign( PORTNUM_B, 7, false);
    disp.Init();
  #else
    #error "Unhandled display type"
  #endif
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

  #if 0 == DISP_SEL
    disp.controller.stb_pin.Assign(0, 18, false);
    disp.controller.clk_pin.Assign(0, 19, false);
    disp.controller.dio_pin.Assign(0, 20, false);
    disp.Init();
  #elif 1 == DISP_SEL
    disp.controller.clk_pin.Assign(0, 18, false);
    disp.controller.dio_pin.Assign(0, 19, false);
    disp.Init();
  #elif 2 == DISP_SEL
    disp.dio_pin.Assign(0, 18, false);
    disp.cs_pin.Assign( 0, 19, false);
    disp.clk_pin.Assign(0, 20, false);
    disp.Init();
  #elif 3 == DISP_SEL
    disp.pin_sclk.Assign(0, 18, false);
    disp.pin_rclk.Assign(0, 19, false);
    disp.pin_din.Assign( 0, 20, false);
    disp.Init();
  #else
    #error "Unhandled display type"
  #endif
}

// ESP

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

  #if 0 == DISP_SEL
    disp.controller.stb_pin.Assign(0, 0, false);
    disp.controller.clk_pin.Assign(0, 1, false);
    disp.controller.dio_pin.Assign(0, 2, false);
    disp.Init();
  #elif 1 == DISP_SEL
    disp.controller.clk_pin.Assign(0, 0, false);
    disp.controller.dio_pin.Assign(0, 1, false);
    disp.Init();
  #elif 2 == DISP_SEL
    disp.dio_pin.Assign(0, 0, false);
    disp.cs_pin.Assign( 0, 1, false);
    disp.clk_pin.Assign(0, 2, false);
    disp.Init();
  #elif 3 == DISP_SEL
    disp.pin_sclk.Assign(0, 0, false);
    disp.pin_rclk.Assign(0, 1, false);
    disp.pin_din.Assign( 0, 2, false);
    disp.Init();
  #else
    #error "Unhandled display type"
  #endif

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
}

#else
  #error "Define board_pins_init here"
#endif

