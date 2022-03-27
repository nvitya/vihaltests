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
 *  file:     tftlcd_mm16_f407v.h
 *  brief:    Memory mapped TFT LCD interface for STM32F407ZE
 *  date:     2022-03-27
 *  authors:  nvitya
*/

#ifndef SRC_TFTLCD_MM16_F407V_H_
#define SRC_TFTLCD_MM16_F407V_H_

#include "platform.h"
#include "tftlcd_mm16.h"

class TTftLcd_mm16_F407V : public TTftLcd_mm16
{
public:
	virtual bool InitInterface();
};

#endif
