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
 *  file:     tftlcd_mm16_vertibo_a.cpp
 *  brief:    Memory mapped TFT LCD interface for VERTIBO-A
 *  version:  1.00
 *  date:     2018-08-21
 *  authors:  nvitya
 *  notes:
 *    When driving the LCD with a flatband cable then you might need
 *    a 10 pF filter on the WR pin at the LCD side.
 *    (The quickly changing data signals can induce a false WR pulse)
*/

#ifdef BOARD_VERTIBO_A

#include <tftlcd_mm16_vertibo_a.h>
#include "hwpins.h"
#include "traces.h"
#include "clockcnt.h"


bool TTftLcd_mm16_vertibo_a::InitInterface()
{
	// ATSAM-E70 initialization

	// RESET
	pin_reset.Assign(PORTNUM_A, 1, false);
	pin_reset.Setup(PINCFG_OUTPUT);
	pin_reset.Set0();
	delay_us(100000);
	pin_reset.Set1();
	delay_us(100000);

	// enable FMC:

	TRACE("Initializing FPGA parallel bus\r\n");

	uint32_t pincfgbase = PINCFG_DRIVE_STRONG;

	hwpinctrl.PinSetup(PORTNUM_C, 0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D0
	hwpinctrl.PinSetup(PORTNUM_C, 1, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D1
	hwpinctrl.PinSetup(PORTNUM_C, 2, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D2
	hwpinctrl.PinSetup(PORTNUM_C, 3, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D3
	hwpinctrl.PinSetup(PORTNUM_C, 4, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D4
	hwpinctrl.PinSetup(PORTNUM_C, 5, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D5
	hwpinctrl.PinSetup(PORTNUM_C, 6, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D6
	hwpinctrl.PinSetup(PORTNUM_C, 7, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D7

	hwpinctrl.PinSetup(PORTNUM_E, 0, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D8
	hwpinctrl.PinSetup(PORTNUM_E, 1, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D9
	hwpinctrl.PinSetup(PORTNUM_E, 2, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D10
	hwpinctrl.PinSetup(PORTNUM_E, 3, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D11
	hwpinctrl.PinSetup(PORTNUM_E, 4, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D12
	hwpinctrl.PinSetup(PORTNUM_E, 5, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D13
	hwpinctrl.PinSetup(PORTNUM_A, 15, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D14
	hwpinctrl.PinSetup(PORTNUM_A, 16, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // D15

	hwpinctrl.PinSetup(PORTNUM_C, 18, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A0/NBS0
	hwpinctrl.PinSetup(PORTNUM_C, 19, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A1
	hwpinctrl.PinSetup(PORTNUM_C, 20, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A2
	hwpinctrl.PinSetup(PORTNUM_C, 21, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A3
	hwpinctrl.PinSetup(PORTNUM_C, 22, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A4
	hwpinctrl.PinSetup(PORTNUM_C, 23, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A5
	hwpinctrl.PinSetup(PORTNUM_C, 24, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A6
	hwpinctrl.PinSetup(PORTNUM_C, 25, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A7
	hwpinctrl.PinSetup(PORTNUM_C, 26, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A8
	hwpinctrl.PinSetup(PORTNUM_C, 27, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A9
	hwpinctrl.PinSetup(PORTNUM_C, 28, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A10
	hwpinctrl.PinSetup(PORTNUM_C, 29, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A11
	hwpinctrl.PinSetup(PORTNUM_C, 30, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // A12

	hwpinctrl.PinSetup(PORTNUM_C, 12, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // NCS3
	hwpinctrl.PinSetup(PORTNUM_C, 11, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // NRD
	hwpinctrl.PinSetup(PORTNUM_C,  8, pincfgbase | PINCFG_OUTPUT | PINCFG_AF_0);  // NWR0/NWE

	// ENABLE SMC clock:
	PMC->PMC_PCER0 = (1 << 9); // SMC PID=9

/*
The timing must be specified separately for the CS and WR (RD) signals,
with the following parameters:
 s : setup time
 p : pulse time
 h : hold time
The parameters are given in MCLK clocks. The WR pulse cycly should match the CS cycle!


CS:   ~~~~\__________/~~~~~
					 pppppppppp
			sssss          hhhhhh

WR:   ~~~~~~~\____/~~~~~~~~
							pppp
			ssssssss    hhhhhhhhh
*/

#if 0
	// 20 cm flying wires

	uint32_t cs_setup_hold  = 0;
	uint32_t cs_pulse  = 5;
	uint32_t wr_pulse  = 3;
	uint32_t wr_pulse_offset  = cs_pulse - wr_pulse - 1; // align to the end

#else
	// 30 cm flatband cable

	uint32_t cs_setup_hold  = 2;
	uint32_t cs_pulse  = 5;
	uint32_t wr_pulse  = 3;
	uint32_t wr_pulse_offset  = cs_pulse - wr_pulse - 1; // align to the end

#endif

	uint32_t cycle     = cs_pulse + 2 * cs_setup_hold;

	// register values:

	uint32_t ncs_wr_setup = cs_setup_hold;
	uint32_t nwe_setup = cs_setup_hold + wr_pulse_offset;

	uint32_t nwe_pulse = wr_pulse;
	uint32_t ncs_wr_pulse = cs_pulse;

	uint32_t nwe_cycle = cycle;

	SMC->SMC_CS_NUMBER[3].SMC_SETUP = 0
	  | (nwe_setup      <<  0)  // NWE_SETUP(6):
	  | (ncs_wr_setup   <<  8)  // NCS_WR_SETUP(6):
	  | (nwe_setup      << 16)  // NRD_SETUP(6):
	  | (ncs_wr_setup   << 24)  // NCS_RD_SETUP(6):
	;

	SMC->SMC_CS_NUMBER[3].SMC_PULSE = 0
	  | (nwe_pulse      <<  0)  // NWE_PULSE(7):
	  | (ncs_wr_pulse   <<  8)  // NCS_WR_PULSE(7):
	  | (nwe_pulse      << 16)  // NRD_PULSE(7):
	  | (ncs_wr_pulse   << 24)  // NCS_RD_PULSE(7):
	;

	SMC->SMC_CS_NUMBER[3].SMC_CYCLE = 0
	  | (nwe_cycle <<  0)  // NWE_CYCLE(9):
	  | (nwe_cycle << 16)  // NRD_PULSE(9):
	;


	SMC->SMC_CS_NUMBER[3].SMC_MODE = 0
		| (0 <<  0)  // READ_MODE:
		| (0 <<  1)  // WRITE_MODE:
		| (0 <<  4)  // EXNW_MODE(2): external NWAIT mode, 0 = disabled
		| (1 <<  8)  // BAT: Byte Access type
		| (1 << 12)  // DBW: Data Bus Width, 1 = 16 bit
		| (0 << 16)  // TDF_CYCLES(4)
		| (0 << 20)  // TDF_MODE:
		| (0 << 24)  // PMEN: page mode enable
		| (0 << 28)  // PS(2): page size
	;

	cmdreg16  = (volatile uint16_t *)(0x63000000);  // A12 = low
	cmdreg32  = (volatile uint32_t *)(0x63000000);  // A12 = low
	datareg16 = (volatile uint16_t *)(0x6300F000);  // A12 = high
	datareg32 = (volatile uint32_t *)(0x6300F000);  // A12 = high

#if 0

	TRACE("Testing PDB...\r\n");
  volatile uint32_t * creg = (volatile uint32_t *)(0x63000000);

	uint16_t d = 1;

	while (1)
	{
		*cmdreg = 0;
//		*datareg = 0xFFFF;

#if 0
		if (*datareg)
		{
			// ignore result
		}
#endif

		delay_us(100);
	}

#endif

	return true;
}

#endif
