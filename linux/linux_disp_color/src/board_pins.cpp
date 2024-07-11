/*
 *  file:     board_pins.cpp
 *  brief:    Application Board pins
 *  date:     2024-07-03
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "traces.h"

unsigned  pin_led_count = 1;
TGpioPin  pin_led[MAX_LEDS] = { TGpioPin(), TGpioPin(), TGpioPin(), TGpioPin() };

void board_pins_init_leds()
{
  for (unsigned n = 0; n < pin_led_count; ++n)
  {
    pin_led[n].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  }
}

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// RV64G
//-------------------------------------------------------------------------------

#elif defined(BOARD_MILKV_DUO)

TTftLcd_sg_spinor  disp;

void board_display_init()
{
  //lcd.mirrorx = true;
  disp.Init(LCD_CTRL_ST7735R, 128, 160);
  disp.SetRotation(1);
}

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(PORTNUM_C, 24, true);  // onboard led: PAD_AUD_OUTR (pin 1 on the QFN-68) = XGPIOC[24]
  board_pins_init_leds();

  SystemCoreClock = 1000000000;

  // LCD control

  disp.pin_cs.Assign(PORTNUM_A, 24, false);    // PAD_SPINOR_CS_X
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  disp.pin_cd.Assign(PORTNUM_A, 23, false);    // PAD_SPINOR_MISO
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  disp.pin_reset.Assign(PORTNUM_A, 27, false); // PAD_SPINOR_WP_X
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  hwpinctrl.PadFuncSetup(PAD_SPINOR_MOSI,  FMUX_SPINOR_MOSI__SPINOR_MOSI, 0);
  hwpinctrl.PadFuncSetup(PAD_SPINOR_SCK,   FMUX_SPINOR_SCK__SPINOR_SCK, 0);

  disp.speed = 16000000;
  disp.Init(LCD_CTRL_ST7735R, 128, 160);
  disp.SetRotation(1);
}

//-------------------------------------------------------------------------------
// ARM Cortex-A
//-------------------------------------------------------------------------------

#elif defined(BOARD_BEAGLEB_BLACK)

TTftLcd_spi  disp;

void board_display_init()
{
  //lcd.mirrorx = true;
  disp.Init(LCD_CTRL_ST7735R, 128, 160);
  disp.SetRotation(1);
}

void board_pins_init()
{
  pin_led_count = 4;
  pin_led[0].Assign(1, 21, false);
  pin_led[1].Assign(1, 22, false);
  pin_led[2].Assign(1, 23, false);
  pin_led[3].Assign(1, 24, false);
  board_pins_init_leds();

  // WARNING: PIN mode setup does not work here !
  //   "For writing to the control module registers, the MPU will need to be in privileged mode of operation
  //   and writes will not work from user mode."
  // for pin mode setup use the "config-pin" utility !

  // LCD control
  hwpinctrl.PinSetup(0, 2, PINCFG_OUTPUT | PINCFG_AF_0);    // P9_22: GPIO0_2 = SPI0_SCK
  hwpinctrl.PinSetup(0, 3, PINCFG_OUTPUT | PINCFG_AF_0);    // P9_21: GPIO0_3 = SPI0_MOSI

  disp.pin_cs.Assign(0, 5, false);                          // P9_17: GPIO0_5
  disp.pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_cd.Assign(1, 16, false);                         // P9_15: GPIO1_16
  disp.pin_cd.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  disp.pin_reset.Assign(0, 31, false);                      // P9_13: GPIO0_31
  disp.pin_reset.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // SPI0
  disp.spi.tx_only = true;
  //disp.spi.datasample_late = true;
  disp.spi.speed = 8000000;
  disp.spi.Init(0);
}

#else
  #error "Define board_pins_init here"
#endif
