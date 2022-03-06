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
/*
 *  file:     tftlcd_gp16_vertibo_a.cpp
 *  brief:    Vertibo-A 16 bit parallel display connection test with GPIO
 *  version:  1.00
 *  date:     2018-08-22
 *  authors:  nvitya
*/


#include "tftlcd_gp16_vertibo_a.h"
#include "hwpins.h"

bool TTftLcd_gp16_vertibo_a::InitInterface()
{
	pin_cd.Assign(PORTNUM_C, 30, false);  // A12
	pin_wr.Assign(PORTNUM_C, 8, false);   // NWR0/NWE
	pin_cs.Assign(PORTNUM_C, 12, false);  // NCS3
	pin_reset.Assign(PORTNUM_A, 1, false);

	//pin_cs.Assign(PORTNUM_C, 14, false);  // NCS0 (FPGA)
	//pin_reset.Assign(PORTNUM_D, 15, false); // NWR1 (FPGABUF)

	pin_d[0].Assign(PORTNUM_C, 0, false);
	pin_d[1].Assign(PORTNUM_C, 1, false);
	pin_d[2].Assign(PORTNUM_C, 2, false);
	pin_d[3].Assign(PORTNUM_C, 3, false);
	pin_d[4].Assign(PORTNUM_C, 4, false);
	pin_d[5].Assign(PORTNUM_C, 5, false);
	pin_d[6].Assign(PORTNUM_C, 6, false);
	pin_d[7].Assign(PORTNUM_C, 7, false);

	pin_d[8].Assign(PORTNUM_E,   0, false);
	pin_d[9].Assign(PORTNUM_E,   1, false);
	pin_d[10].Assign(PORTNUM_E,  2, false);
	pin_d[11].Assign(PORTNUM_E,  3, false);
	pin_d[12].Assign(PORTNUM_E,  4, false);
	pin_d[13].Assign(PORTNUM_E,  5, false);
	pin_d[14].Assign(PORTNUM_A, 15, false);
	pin_d[15].Assign(PORTNUM_A, 16, false);

	unsigned n;
	unsigned pin_flags = PINCFG_OUTPUT | PINCFG_SPEED_FAST | PINCFG_GPIO_INIT_1;

	pin_cs.Setup(pin_flags);
	pin_wr.Setup(pin_flags);
	pin_cd.Setup(pin_flags);
	pin_reset.Setup(pin_flags);

	for (n = 0; n < 16; ++n)
	{
		pin_d[n].Setup(pin_flags);
	}

	return true;
}

