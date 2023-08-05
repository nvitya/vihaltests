/*
 *  file:     board_pins.cpp (uart)
 *  brief:    UART Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "hwsdram.h"
#include "hwlcdctrl.h"
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

#ifdef DISP_SPI

TTftLcd_spi  disp;

void init_spi_display()
{
#if 240 == SPI_DISPLAY_WIDTH
  disp.mirrorx = true;
  disp.Init(LCD_CTRL_ILI9341, 240, 320);
  disp.SetRotation(1);
#else
  //lcd.mirrorx = true;
  disp.Init(LCD_CTRL_ST7735R, 128, 160);
  disp.SetRotation(1);

  //disp.FillScreen(0x0000);
  //disp.FillRect(10, 10, 4, 4, 0xFF00);

#endif
}

#endif

#if 0  // to use elif everywhere

//----------------------------------------------------------------------------------------------------
// SPI DISPLAYS with SMALLER MCUs
//----------------------------------------------------------------------------------------------------

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

  // LCD control
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_OUTPUT | PINCFG_AF_0); // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_OUTPUT | PINCFG_AF_0); // SPI1_MOSI

  disp.pin_reset.Assign(PORTNUM_B, 1, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B1: RESET

  disp.pin_cs.Assign(PORTNUM_B, 2, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cd.Assign(PORTNUM_B, 0, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SPI1
  disp.spi.speed = SystemCoreClock / 4;
  disp.spi.Init(0);
}

void board_display_init()
{
  // special display init for the integrated display:
  disp.invert_color = true;
  disp.shift_x = 26;
  disp.shift_y = 1;
  disp.Init(LCD_CTRL_ST7735R, 80, 160);

  disp.SetRotation(3);
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

  // LCD control
/*
 * Y:    CS      4
 * OR:   RESET   3
 * BL:   CD      2
 * W:    MOSI    1
 * GRY:  SCK     0
 * PUR:  LED
 */

  uint32_t speed_flag = 0; //PINCFG_SPEED_FAST;

  hwpinctrl.PadSetup(PAD_GPIO0, FSPICLK_OUT_IDX, PINCFG_OUTPUT | speed_flag);  // SPI2_SCK
  hwpinctrl.PadSetup(PAD_GPIO1, FSPID_OUT_IDX,   PINCFG_OUTPUT | speed_flag);  // SPI2_MOSI

  disp.pin_reset.Assign(0, 3, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B1: RESET

  disp.pin_cs.Assign(0, 4, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1 | speed_flag);

  disp.pin_cd.Assign(0, 2, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1 | speed_flag);

  // SPI1
  disp.spi.speed = 20000000; // seems to be the maximal working speed
  disp.spi.Init(2);

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

void board_display_init()
{
  init_spi_display();
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

#elif defined(BOARD_MIN_F103)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_PULLUP);  // USART1_RX, no AF here!
  conuart.Init(1);

  // LCD control
  //hwpinctrl.PinSetup(PORTNUM_A, 4, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // SPI1_CS as GPIO
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_OUTPUT | PINCFG_AF_0); // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_OUTPUT | PINCFG_AF_0); // SPI1_MOSI

  disp.pin_reset.Assign(PORTNUM_A, 4, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B0: RESET

  disp.pin_cs.Assign(PORTNUM_A, 2, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cd.Assign(PORTNUM_A, 3, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SPI1
  disp.spi.speed = 36000000; // max speed for this MCU
  disp.spi.Init(1);
}

void board_display_init()
{
  init_spi_display();
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

  // LCD control
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_OUTPUT | PINCFG_AF_5); // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_OUTPUT | PINCFG_AF_5); // SPI1_MOSI

  disp.pin_reset.Assign(PORTNUM_A, 4, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B0: RESET

  disp.pin_cs.Assign(PORTNUM_A, 2, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cd.Assign(PORTNUM_A, 3, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SPI1
  disp.spi.speed = SystemCoreClock / 2; // max speed for this MCU
  disp.spi.Init(1);
}

void board_display_init()
{
  init_spi_display();
}

#elif    defined(BOARD_MIBO48_STM32F303) \
      || defined(BOARD_MIBO64_STM32F405) \
      || defined(BOARD_MIBO48_STM32G473)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 13, false);
  board_pins_init_leds();

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP);  // USART1_RX
  conuart.Init(1);

  // LCD control
  hwpinctrl.PinSetup(PORTNUM_A, 5, PINCFG_OUTPUT | PINCFG_AF_5); // SPI1_SCK
  hwpinctrl.PinSetup(PORTNUM_A, 7, PINCFG_OUTPUT | PINCFG_AF_5); // SPI1_MOSI

  disp.pin_reset.Assign(PORTNUM_B, 0, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B0: RESET

  disp.pin_cs.Assign(PORTNUM_B, 2, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cd.Assign(PORTNUM_B, 1, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SPI1
  disp.spi.speed = SystemCoreClock / 2; // max speed for this MCU
  disp.spi.Init(1);
}

void board_display_init()
{
  init_spi_display();
}

//----------------------------------------------------------------------------------------------------
// EMBEDDED LCD CONROLLER WITH FRAMEBUFFER
//----------------------------------------------------------------------------------------------------

#elif defined(BOARD_DISCOVERY_F746) || defined(BOARD_DISCOVERY_F750)

THwLcdCtrl      lcdctrl;
TFrameBuffer16  disp;

void sdram_init()
{
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

void lcd_init()
{
  uint32_t  tmp;
  uint32_t  pinflags = 0;

  // LCD CONTROLLER PINS

  hwpinctrl.PinSetup(PORTNUM_E,  4, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_G, 12, pinflags | PINCFG_AF_12); //

  hwpinctrl.PinSetup(PORTNUM_I,  9, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_I, 10, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_I, 14, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_I, 15, pinflags | PINCFG_AF_14); //

  hwpinctrl.PinSetup(PORTNUM_J,  0, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  1, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  2, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  3, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  4, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  5, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  6, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  7, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  8, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J,  9, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J, 10, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J, 11, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J, 13, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J, 14, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_J, 15, pinflags | PINCFG_AF_14); //

  hwpinctrl.PinSetup(PORTNUM_K,  0, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_K,  1, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_K,  2, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_K,  4, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_K,  5, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_K,  6, pinflags | PINCFG_AF_14); //
  hwpinctrl.PinSetup(PORTNUM_K,  7, pinflags | PINCFG_AF_14); //

  // LCD GPIO PINS

  hwpinctrl.PinSetup(PORTNUM_I, 12, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // LCD_DISP
  hwpinctrl.PinSetup(PORTNUM_K,  3, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // LCD_BL_CTRL

  // LCD Controller
  lcdctrl.Init(480, 272, (void *)hwsdram.address);

  // Frame Buffer Display
  disp.Init(480, 272, lcdctrl.framebuffer);
  disp.FillScreen(0);
}


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

  sdram_init();

}

void board_display_init()
{
  lcd_init();
}

//----------------------------------------------------------------------------------------------------
// PARALLEL DISPLAYS
//----------------------------------------------------------------------------------------------------

#elif defined(BOARD_DEV_STM32F407ZE)

TTftLcd_mm16_F407ZE  disp;

void board_pins_init()
{
  pin_led_count = 2;
  pin_led[0].Assign(PORTNUM_F,  9, false);
  pin_led[1].Assign(PORTNUM_F, 10, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7);  // USART1_RX
  conuart.Init(1); // USART1

  disp.mirrorx = true;
  disp.Init(LCD_CTRL_UNKNOWN, 240, 320);
  disp.SetRotation(1);
}

#elif defined(BOARD_DEV_STM32F407VG)

TTftLcd_mm16_F407V  disp;

TGpioPin  lcd_reset(PORTNUM_C, 6, false);

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_E, 0, false);
  board_pins_init_leds();

  lcd_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  lcd_reset.Set0();

  delay_ms(10);

  lcd_reset.Set1(); // remove reset

  delay_ms(20);

  // USART1
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_INPUT  | PINCFG_AF_7);  // USART1_RX
  conuart.Init(1);

}

void board_display_init()
{
  disp.mirrorx = true;
  //disp.Init(LCD_CTRL_HX8357B, 480, 320);
  //disp.Init(LCD_CTRL_UNKNOWN, 480, 320);
  disp.Init(LCD_CTRL_HX8357B, 320, 480);
  //disp.SetRotation(0);
}

// ATSAM

#elif defined(BOARD_WIO_TERMINAL)

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 15, false);
  board_pins_init_leds();

  // SERCOM2
  hwpinctrl.PinSetup(PORTNUM_B, 26, PINCFG_OUTPUT | PINCFG_AF_C);  // PAD[0] = TXD
  hwpinctrl.PinSetup(PORTNUM_B, 27, PINCFG_INPUT  | PINCFG_AF_C | PINCFG_PULLUP);  // PAD[1] = RXD
  conuart.Init(2);

/*
    LCD-MISO | B18 - SERCOM7.2
    LCD-MOSI | B19 - SERCOM7.3
    LCD-SCK  | B20 - SERCOM7.1
    LCD-CS   | B21 - SERCOM7.0
    LCD-BLED | C05
    LCD-D/C  | C06
    LCD-RST  | C07
 */


  // LCD control
  hwpinctrl.PinSetup(PORTNUM_B, 20, PINCFG_OUTPUT | PINCFG_AF_D); // SERCOM7[1] = SCK
  hwpinctrl.PinSetup(PORTNUM_B, 19, PINCFG_OUTPUT | PINCFG_AF_D); // SERCOM7[3] = MOSI

  disp.pin_reset.Assign(PORTNUM_C, 7, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B0: RESET

  disp.pin_cs.Assign(PORTNUM_B, 21, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cd.Assign(PORTNUM_C, 6, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // LCD Backlight
  hwpinctrl.PinSetup(PORTNUM_C, 5, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);  // turn the LCD backlight on

  // SPI1
  //disp.spi.speed = 30000000;
  disp.spi.speed = SystemCoreClock / 2; // max speed for this MCU
  disp.spi.data_out_pinout = 2;  // PAD3 = MOSI
  disp.spi.Init(7);
}

void board_display_init()
{
  disp.mirrorx = true;
  disp.Init(LCD_CTRL_ILI9341, 240, 320);
  disp.SetRotation(3);
}

#elif defined(BOARD_ARDUINO_DUE)

/* 16 bit parallel display module designed to arduino mega connected to DUE
 * The pins arent layed out optimal so it is as fast as an SPI display
 *
 * At least optimization -O1 or -Og must be used otherwise it will be too slow
 */

#include "tftlcd_gp16_due.h"

TTftLcd_gp16_due  disp;

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_B, 27, false);
  board_pins_init_leds();

  // UART - On the Arduino programmer interface
  hwpinctrl.PinSetup(0, 8, PINCFG_INPUT | PINCFG_AF_0);  // UART_RXD
  hwpinctrl.PinSetup(0, 9, PINCFG_OUTPUT | PINCFG_AF_0); // UART_TXD
  conuart.Init(0);  // UART

}

void board_display_init()
{
  disp.mirrorx = true;
  disp.Init(LCD_CTRL_HX8357B, 320, 480);
  disp.SetRotation(3);
}

#elif defined(BOARD_VERTIBO_A)

#if VERTIBO_A_LCD_GPIO

  #include "tftlcd_gp16_vertibo_a.h"

  TTftLcd_gp16_vertibo_a  disp;

#else

  #include "tftlcd_mm16_vertibo_a.h"

  TTftLcd_mm16_vertibo_a  disp;

#endif

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_A, 29, false);
  board_pins_init_leds();

  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_0);  // UART0_RX
  hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_0);  // UART0_TX
  conuart.baudrate = 115200;
  conuart.Init(0);
}

void board_display_init()
{
  #if VERTIBO_A_LCD_800x480

    disp.mirrorx = true;
    //disp.Init(LCD_CTRL_ILI9486, 480, 800);
    //disp.Init(LCD_CTRL_ILI9341, 480, 800);
    disp.Init(LCD_CTRL_HX8357B, 480, 800);
    //disp.Init(LCD_CTRL_UNKNOWN, 480, 800);
    disp.SetRotation(0);

  #else

    disp.mirrorx = true;
    //disp.Init(LCD_CTRL_ILI9486, 320, 480);
    disp.Init(LCD_CTRL_HX8357B, 320, 480);
    //lcd.Init(LCD_CTRL_UNKNOWN, 320, 480);
    disp.SetRotation(1);

  #endif
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

  // LCD control
  hwpinctrl.PinSetup(0, 14, PINCFG_OUTPUT | PINCFG_AF_1); // SPI1_SCK
  hwpinctrl.PinSetup(0, 15, PINCFG_OUTPUT | PINCFG_AF_1); // SPI1_MOSI

  disp.pin_reset.Assign(0, 13, false);
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // B0: RESET

  disp.pin_cd.Assign(0, 12, false);
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cs.Assign(0, 11, false);
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SPI1
  disp.spi.speed = SystemCoreClock / 4;
  disp.spi.Init(1);
}

void board_display_init()
{
  init_spi_display();
}

#else
  #error "Define board_pins_init here"
#endif

