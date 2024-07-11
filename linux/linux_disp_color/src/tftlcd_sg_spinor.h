/* This file is a part of the VIHAL project: https://github.com/nvitya/vihal
 * Copyright (c) 2021 Viktor Nagy, nvitya
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
 *  file:     tftlcd_sg_spinor.h
 *  brief:    SPI TFT LCD Display driver with the SPINOR interface on SG200x
 *  date:     2024-05-30
 *  authors:  nvitya
*/

#ifndef TFTLCD_SG_SPINOR_H_
#define TFTLCD_SG_SPINOR_H_

#include "platform.h"

#if defined(MCUF_SG2000)

#include "tftlcd.h"
#include "hwpins.h"

class TTftLcd_sg_spinor: public TTftLcd
{
public:
	typedef TTftLcd super;

	// required hw resources:

	TGpioPin   pin_cs;
	TGpioPin   pin_cd;
	TGpioPin 	 pin_reset;  // unassigned

public:
	// interface dependent

	virtual bool InitInterface();
	virtual void ResetPanel();

	virtual void WriteCmd(uint8_t adata);
	virtual void WriteData8(uint8_t adata);
	virtual void WriteData16(uint16_t adata);

	virtual void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,  uint16_t h);
	virtual void FillColor(uint16_t acolor, unsigned acount);

  virtual void BlockFillBegin();
  virtual void BlockFill(uint16_t acolor, unsigned acount);
  virtual void BlockFillEnd();

public:
  spinor_regs_t *  regs = nullptr;
  uint32_t         tran_csr_base = 0;
  uint32_t         ctrl_d8  = 0;
  uint32_t         basespeed = 0;
  uint32_t         speed = 4000000;

  void             NorSpiInit();
  void             SetSpeed(uint32_t aspeed);

  void             SpiSendData8(uint8_t adata);
  void             SpiSendData16(uint16_t adata);

};

#endif

#endif /* TFTLCD_SPINOR_H_ */
