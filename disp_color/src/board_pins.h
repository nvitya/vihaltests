/*
 *  file:     board_pins.h (spiflash)
 *  brief:    SPI Flash Test Board pins
 *  version:  1.00
 *  date:     2021-10-29
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"

#define MAX_LEDS  4

extern TGpioPin   pin_led[MAX_LEDS];
extern unsigned   pin_led_count;

extern THwUart    conuart;  // console uart

void board_pins_init();

#if 0

//----------------------------------------------------------------------------------------------------
// SPI DISPLAYS with SMALLER MCUs
//----------------------------------------------------------------------------------------------------

#elif    defined(BOARD_MIN_F103) \
      || defined(BOARD_MIN_F401) \
      || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO48_STM32F303) \
      || defined(BOARD_MIBO64_STM32F405) \
      || defined(BOARD_MIBO48_STM32G473)

#define DISP_SPI

#include "tftlcd_spi.h"

extern TTftLcd_spi  disp;

//----------------------------------------------------------------------------------------------------
// EMBEDDED LCD CONROLLER WITH FRAMEBUFFER
//----------------------------------------------------------------------------------------------------

#elif defined(BOARD_DISCOVERY_F746) || defined(BOARD_DISCOVERY_F750)

#include "framebuffer16.h"

extern TFrameBuffer16  disp;

//----------------------------------------------------------------------------------------------------
// PARALLEL DISPLAYS
//----------------------------------------------------------------------------------------------------

#elif defined(BOARD_DEV_STM32F407ZE)

#include "tftlcd_mm16_f407ze.h"

extern TTftLcd_mm16_F407ZE  disp;

#elif defined(BOARD_DEV_STM32F407VG)

#include "tftlcd_mm16_f407v.h"

extern TTftLcd_mm16_F407V  disp;

#elif defined(BOARD_ARDUINO_DUE)

#include "tftlcd_gp16_due.h"

extern TTftLcd_gp16_due  disp;

#elif defined(BOARD_VERTIBO_A)


  #if VERTIBO_A_LCD_GPIO

    #include "tftlcd_gp16_vertibo_a.h"

    extern TTftLcd_gp16_vertibo_a  disp;

  #else

    #include "tftlcd_mm16_vertibo_a.h"

    extern TTftLcd_mm16_vertibo_a  disp;

  #endif

#else
  #error "Board not defined."
#endif

#ifdef DISP_SPI

void init_spi_display();

#endif

#endif /* SRC_BOARD_PINS_H_ */
