/*
 *  file:     board_pins.cpp (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "platform.h"
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
THwSpi     fl_spi;
THwQspi    fl_qspi;
TSpiFlash  spiflash;

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0 // to use elif everywhere

#elif defined(MCUF_VRV100) //BOARD_VRV100_441)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 0, false);
  board_pins_init_leds();

  conuart.Init(1); // UART1

  fl_spi.speed = 2000000;
  fl_spi.Init(1); // flash

  spiflash.spi = &fl_spi;
  spiflash.has4kerase = true;
  spiflash.Init();
}

#elif defined(BOARD_NODEMCU_ESP32C3)

void board_pins_init()
{
  pin_led_count = 3;
  pin_led[0].Assign(0, 3, false);
  pin_led[1].Assign(0, 4, false);
  pin_led[2].Assign(0, 5, false);
  board_pins_init_leds();

  //hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT);
  hwpinctrl.PadSetup(PAD_U0TXD, U0TXD_OUT_IDX, PINCFG_OUTPUT | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  hwpinctrl.PadSetup(PAD_U0RXD, U0RXD_IN_IDX,  PINCFG_INPUT  | PINCFG_AF_0);  // with AF_0 there is a direct routing mode
  conuart.Init(0);

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
}

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

  fl_spi.speed = 16000000;
  fl_spi.Init(3);

  spiflash.spi = &fl_spi;
  spiflash.has4kerase = true;
  spiflash.Init();
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
  fl_qspi.speed = 32000000;
  fl_qspi.Init();

  spiflash.qspi = &fl_qspi;
  spiflash.has4kerase = true;
  spiflash.Init();
}

#elif defined(BOARD_DISCOVERY_F750)

/* WARNING: for debugging with openocd add this to "Run/Restart commands":

      monitor gdb_breakpoint_override hard

  Without this the instruction stepping did not work for me.

*/

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

  // QSPI

  uint32_t qspipincfg = 0; //PINCFG_SPEED_MEDIUM | PINCFG_DRIVE_STRONG;
  hwpinctrl.PinSetup(PORTNUM_B,  6, qspipincfg | PINCFG_AF_10);  // NCS
  hwpinctrl.PinSetup(PORTNUM_B,  2, qspipincfg | PINCFG_AF_9);   // CLK
  hwpinctrl.PinSetup(PORTNUM_D, 11, qspipincfg | PINCFG_AF_9);   // IO0
  hwpinctrl.PinSetup(PORTNUM_D, 12, qspipincfg | PINCFG_AF_9);   // IO1
  hwpinctrl.PinSetup(PORTNUM_E,  2, qspipincfg | PINCFG_AF_9);   // IO2
  hwpinctrl.PinSetup(PORTNUM_D, 13, qspipincfg | PINCFG_AF_9);   // IO3

  fl_qspi.multi_line_count = 4;  // there are some problems with quad here
  fl_qspi.speed = 50000000;
  fl_qspi.Init();

  spiflash.qspi = &fl_qspi;
  spiflash.has4kerase = true;
  spiflash.Init();
}


#else
  #error "Define board_pins_init here"
#endif

