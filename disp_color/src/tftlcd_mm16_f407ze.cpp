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

#ifdef BOARD_DEV_STM32F407ZE

#include <tftlcd_mm16_f407ze.h>
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

bool TTftLcd_mm16_F407ZE::InitInterface()
{
	// stm32f407 FSMC initialization

	// enable FMC:

	RCC->AHB3ENR |= 1; // RCC_AHB3ENR_FSMCEN; // symbol names are different, bit is the same

	unsigned pin_flags = PINCFG_AF_12 | PINCFG_SPEED_FAST | PINCFG_PULLUP;

	hwpinctrl.PinSetup(3, 14, pin_flags); // FSMC_D0: PD14
	hwpinctrl.PinSetup(3, 15, pin_flags); // FSMC_D1: PD15
	hwpinctrl.PinSetup(3,  0, pin_flags); // FSMC_D2: PD0
	hwpinctrl.PinSetup(3,  1, pin_flags); // FSMC_D3: PD1
	hwpinctrl.PinSetup(4,  7, pin_flags); // FSMC_D4: PE7
	hwpinctrl.PinSetup(4,  8, pin_flags); // FSMC_D5: PE8
	hwpinctrl.PinSetup(4,  9, pin_flags); // FSMC_D6: PE9
	hwpinctrl.PinSetup(4, 10, pin_flags); // FSMC_D7: PE10
	hwpinctrl.PinSetup(4, 11, pin_flags); // FSMC_D8: PE11
	hwpinctrl.PinSetup(4, 12, pin_flags); // FSMC_D9: PE12
	hwpinctrl.PinSetup(4, 13, pin_flags); // FSMC_D10: PE13
	hwpinctrl.PinSetup(4, 14, pin_flags); // FSMC_D11: PE14
	hwpinctrl.PinSetup(4, 15, pin_flags); // FSMC_D12: PE15
	hwpinctrl.PinSetup(3,  8, pin_flags); // FSMC_D13: PD8
	hwpinctrl.PinSetup(3,  9, pin_flags); // FSMC_D14: PD9
	hwpinctrl.PinSetup(3, 10, pin_flags); // FSMC_D15: PD10

	hwpinctrl.PinSetup(6, 12, pin_flags); // PG12: FSMC_NE4
	hwpinctrl.PinSetup(3,  4, pin_flags); // PD4: FSMC_NOE
	hwpinctrl.PinSetup(3,  5, pin_flags); // PD5: FSMC_NWE
	//hwpinctrl.PinSetup(3,  6, pin_flags); // PD6: FSMC_NWAIT

	hwpinctrl.PinSetup(5, 12, pin_flags); // FSMC_A6: PF12

	// configure Bank 4
	//-----------------

	// Control register
	FSMC->BCR4 = 0
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

#define SLOW_BUS_TIMING
//#define FASTEST_BUS_TIMING
//#define NODELAY_BUS_TIMING

#ifdef SLOW_BUS_TIMING

 // very safe timing:
	FSMC->BTR4 = 0
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

	FSMC->BTR4 = 0
		| (0 << 28)  // (2 bits) ACCMOD (ignored when EXTMOD = 0)
		| (0 << 24)  // (4 bits) Data latency
		| (0 << 20)  // (4 bits) CLK division (ignored for SRAM)
		| (0 << 16)  // (4 bits) Bus turnaround phase duration
		| (5 <<  8)  // (8 bits) Data phase duration
		| (0 <<  4)  // (4 bits) Address hold phase duration
		| (2 <<  0)  // (4 bits) Address setup phase duration
	;

#endif

#ifdef NODELAY_BUS_TIMING
	// lowest possible delays (with prototype cables):

	FSMC->BTR4 = 0
		| (0 << 28)  // (2 bits) ACCMOD (ignored when EXTMOD = 0)
		| (0 << 24)  // (4 bits) Data latency
		| (0 << 20)  // (4 bits) CLK division (ignored for SRAM)
		| (0 << 16)  // (4 bits) Bus turnaround phase duration
		| (0 <<  8)  // (8 bits) Data phase duration
		| (0 <<  4)  // (4 bits) Address hold phase duration
		| (0 <<  0)  // (4 bits) Address setup phase duration
	;

#endif

	cmdreg  = (volatile uint16_t *)(0x6C000000);  // A6 = low
	datareg = (volatile uint16_t *)(0x6C000080);  // A6 = high

	// Separate Write Timing Register
	//FSMC->BWTR[3] = 0; // (ignored)

	return true;
}

#endif
