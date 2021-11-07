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
 *  file:     i2c_eeprom_app.cpp
 *  brief:    I2C EEPROM Emulation Class
 *  version:  1.00
 *  date:     2019-10-13
 *  authors:  nvitya
*/

#include <i2c_eeprom_app.h>
#include "traces.h"

bool TI2cEepromApp::Init(uint8_t aaddress, uint8_t aaddrmask)
{
	address     = aaddress;
	addressmask = aaddrmask;

	if (!InitHw(devnum))
	{
		return false;
	}

	return true;
}

void TI2cEepromApp::OnAddressRw(uint8_t aaddress)  // IRQ context !
{
	if (istx)  // istx = true: Slave -> Master
	{
		waitmemaddr = false;
	}
	else
	{
		waitmemaddr = true;
	}
	//TRACE("TI2cEepromApp:OnAddress(%02X)\r\n", aaddress);
}

void TI2cEepromApp::OnByteReceived(uint8_t adata) // IRQ context !
{
	if (waitmemaddr)
	{
		memaddr = adata;
		waitmemaddr = false;
	}
	else
	{
		data[memaddr++] = adata;
	}
}

uint8_t TI2cEepromApp::OnTransmitRequest() // IRQ context !
{
	uint8_t r = data[memaddr++];
	return r;
}

