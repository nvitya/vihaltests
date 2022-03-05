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
 *  file:     tftlcd_gp16_vertibo_a.h
 *  brief:    Vertibo-A 16 bit parallel display connection test with GPIO
 *  version:  1.00
 *  date:     2018-08-22
 *  authors:  nvitya
*/

#ifndef SRC_TFTLCD_GP16_VERTIBO_A_H_
#define SRC_TFTLCD_GP16_VERTIBO_A_H_

#include "platform.h"
#include "tftlcd_gp16.h"

class TTftLcd_gp16_vertibo_a: public TTftLcd_gp16
{
public:
	virtual bool InitInterface();
};

#endif /* SRC_TFTLCD_GP16_VERTIBO_A_H_ */
