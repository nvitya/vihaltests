/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM Tests project: https://github.com/nvitya/nvcmtests
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     tftlcd_mm16_f407ze.cpp
 *  brief:    Memory mapped TFT LCD interface for STM32F407ZE
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#if defined(BOARD_DEV_STM32F407VG)

#include <tftlcd_mm16_f407v.h>
#include "hwpins.h"

// the original STM32Fxx definition for the FSMC is unuseable

typedef struct
{
  __IO uint32_t BCR1;
  __IO uint32_t BTR1;
  __IO uint32_t BCR2;
  __IO uint32_t BTR2;
  __IO uint32_t BCR3;
  __IO uint32_t BTR3;
  __IO uint32_t BCR4;
  __IO uint32_t BTR4;
  __IO uint32_t _reserved[57];
  __IO uint32_t BWTR1;
  __IO uint32_t _reserved2;
  __IO uint32_t BWTR2;
  __IO uint32_t _reserved3;
  __IO uint32_t BWTR3;
  __IO uint32_t _reserved4;
  __IO uint32_t BWTR4;
//
} T_FSMC_REGS;

#ifdef FMC_R_BASE
  #define FMC_REGS_ADDR  FMC_R_BASE
#endif

#ifdef FSMC_R_BASE
  #define FMC_REGS_ADDR  FSMC_R_BASE
#endif

#define FSMC ((T_FSMC_REGS *) FMC_REGS_ADDR)

bool TTftLcd_mm16_F407V::InitInterface()
{
	// stm32f407 FSMC initialization

	// enable FMC:

	RCC->AHB3ENR |= 1; // RCC_AHB3ENR_FSMCEN; // symbol names are different, bit is the same

	unsigned pin_flags = PINCFG_AF_12 | PINCFG_SPEED_FAST | PINCFG_PULLUP;

	hwpinctrl.PinSetup(PORTNUM_D, 14, pin_flags); // FSMC_D0: PD14
	hwpinctrl.PinSetup(PORTNUM_D, 15, pin_flags); // FSMC_D1: PD15
	hwpinctrl.PinSetup(PORTNUM_D,  0, pin_flags); // FSMC_D2: PD0
	hwpinctrl.PinSetup(PORTNUM_D,  1, pin_flags); // FSMC_D3: PD1

	hwpinctrl.PinSetup(PORTNUM_E,  7, pin_flags); // FSMC_D4: PE7
	hwpinctrl.PinSetup(PORTNUM_E,  8, pin_flags); // FSMC_D5: PE8
	hwpinctrl.PinSetup(PORTNUM_E,  9, pin_flags); // FSMC_D6: PE9
	hwpinctrl.PinSetup(PORTNUM_E, 10, pin_flags); // FSMC_D7: PE10

	hwpinctrl.PinSetup(PORTNUM_E, 11, pin_flags); // FSMC_D8: PE11
	hwpinctrl.PinSetup(PORTNUM_E, 12, pin_flags); // FSMC_D9: PE12
	hwpinctrl.PinSetup(PORTNUM_E, 13, pin_flags); // FSMC_D10: PE13
	hwpinctrl.PinSetup(PORTNUM_E, 14, pin_flags); // FSMC_D11: PE14
	hwpinctrl.PinSetup(PORTNUM_E, 15, pin_flags); // FSMC_D12: PE15

	hwpinctrl.PinSetup(PORTNUM_D,  8, pin_flags); // FSMC_D13: PD8
	hwpinctrl.PinSetup(PORTNUM_D,  9, pin_flags); // FSMC_D14: PD9
	hwpinctrl.PinSetup(PORTNUM_D, 10, pin_flags); // FSMC_D15: PD10

	hwpinctrl.PinSetup(PORTNUM_D,  7, pin_flags); // PD7:  FSMC_NE1 = LCD_CS
  hwpinctrl.PinSetup(PORTNUM_D,  5, pin_flags); // PD5:  FSMC_NWE = LCD_WR
  hwpinctrl.PinSetup(PORTNUM_D, 13, pin_flags); // PD13: FSMC_A18 = LCD_RS

	//hwpinctrl.PinSetup(PORTNUM_D,  4, pin_flags); // PD4: FSMC_NOE = ???


	// configure Bank 1
	//-----------------

	// Control register
	FSMC->BCR1 = 0
		| (0 << 15)  // no async wait
		| (0 << 14)  // EXTMOD: 0 = read and write timings are same, BWTR register ignored
		| (1 << 12)  // Enable Write
		| (0 <<  6)  // Flash access enable ???
		| (1 <<  4)  // 16 bit wide
		| (0 <<  2)  // SRAM
		| (0 <<  1)  // no address / data multiplexing
		| (1 <<  0)  // Enable it
	;

	// Timing Register

//#define SLOW_BUS_TIMING
#define FASTEST_BUS_TIMING
//#define NODELAY_BUS_TIMING

#ifdef SLOW_BUS_TIMING

 // very safe timing:
	FSMC->BTR1 = 0
			| ( 0 << 28)  // (2 bits) ACCMOD (ignored when EXTMOD = 0)
			| (15 << 24)  // (4 bits) Data latency
			| ( 1 << 20)  // (4 bits) CLK division (ignored for SRAM)
			| (15 << 16)  // (4 bits) Bus turnaround phase duration
			| (63 <<  8)  // (8 bits) Data phase duration
			| (15 <<  4)  // (4 bits) Address hold phase duration
			| (15 <<  0)  // (4 bits) Address setup phase duration
	;

#endif

#ifdef FASTEST_BUS_TIMING
	// lowest possible delays (with prototype cables):

	FSMC->BTR1 = 0
		| (0 << 28)  // (2 bits) ACCMOD (ignored when EXTMOD = 0)
		| (1 << 24)  // (4 bits) Data latency
		| (0 << 20)  // (4 bits) CLK division (ignored for SRAM)
		| (1 << 16)  // (4 bits) Bus turnaround phase duration
		| (5 <<  8)  // (8 bits) Data phase duration
		| (1 <<  4)  // (4 bits) Address hold phase duration
		| (1 <<  0)  // (4 bits) Address setup phase duration
	;

#endif

#ifdef NODELAY_BUS_TIMING
	// lowest possible delays (with prototype cables):

	FSMC->BTR1 = 0
		| (0 << 28)  // (2 bits) ACCMOD (ignored when EXTMOD = 0)
		| (0 << 24)  // (4 bits) Data latency
		| (0 << 20)  // (4 bits) CLK division (ignored for SRAM)
		| (0 << 16)  // (4 bits) Bus turnaround phase duration
		| (0 <<  8)  // (8 bits) Data phase duration
		| (0 <<  4)  // (4 bits) Address hold phase duration
		| (0 <<  0)  // (4 bits) Address setup phase duration
	;

#endif

	cmdreg  = (volatile uint16_t *)(0x60000000);  // A18 = low
	datareg = (volatile uint16_t *)(0x60FFFFF0);  // A18 = high

	// Separate Write Timing Register
	//FSMC->BWTR[3] = 0; // (ignored)


#if 0 // test


  while (1)
  {
    __NOP();
    *cmdreg = 0;
    *cmdreg = 0xFFFF;
    *datareg = 0;
    *datareg = 0xFFFF;

    //*cmdreg = 0;
    //*datareg = 0;
    __NOP();
  }

#endif

	return true;
}

#endif
