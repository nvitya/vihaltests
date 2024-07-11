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
 *  file:     tftlcd_spinor.cpp
 *  brief:    SPI TFT LCD Display driver with the SPINOR interface on SG200x
 *  date:     2024-05-30
 *  authors:  nvitya
*/

#include "platform.h"

#if defined(MCUF_SG2000)

#include <tftlcd_sg_spinor.h>
#include "clockcnt.h"
#include "sg_utils.h"

bool TTftLcd_sg_spinor::InitInterface()
{
	// should be overridden !
  NorSpiInit();
	return false;
}

void TTftLcd_sg_spinor::NorSpiInit()
{
  //uint32_t tmp;

  regs = (spinor_regs_t *)map_hw_addr(SPINOR_BASE_ADDR, sizeof(spinor_regs_t), (void * *)&regs);

  regs->SPI_CTRL = (1 << 21); // software reset
  if (regs->SPI_CTRL) { }
  regs->DMMR_CTRL = 0; // disable memory mapped mode (probably not necessary)
  if (regs->DMMR_CTRL) { }

  basespeed = 150000000; // ???

  uint32_t spi_ctrl = (0
    | (0  <<  0)  // SCK_DIV(11):
    | (0  << 11)  // CPHA:
    | (0  << 12)  // CPOL: 0 = CLK is low when idle, 1 = CLK is high when idle
    | (1  << 14)  // HOLD_O
    | (1  << 15)  // WP_O
    | (0  << 16)  // FRAME_LEN(4): 0 = 16-bit, 8 = 8-bit, 15 = 15-bit
    | (0  << 20)  // LSB_FIRST: 0 = MSB first, 1 = LSB first
    | (0  << 21)  // SRST: reset
  );

  ctrl_d8 = (spi_ctrl
    | (8  << 16)  // FRAME_LEN(4): 0 = 16-bit, 8 = 8-bit, 15 = 15-bit
  );
  regs->SPI_CTRL = ctrl_d8;

  regs->CE_CTRL = 0;

  regs->DLY_CTRL = (0
    | (0  <<  0)  // FRAME_INTERVAL(4)
    | (0  <<  8)  // CET(4) (no cs delay)
    | (0  << 12)  // SMP_EN_DLY(2)
    | (0  << 14)  // RX_PIPE_CTRL
  );

  regs->INT_EN = 0;

  regs->FF_PT = 0; // clears the FIFO

  tran_csr_base = (0
    | (2  <<  0)  // TRAN_MODE(2): 0 = no Tx + no Rx, 1 = Rx only, 2 = Tx only, 3 = Tx+Rx (no DMA!)
    | (0  <<  3)  // FAST_MODE: 1 = fast mode (???)
    | (0  <<  4)  // BUS_WIDTH(2): 0 = 1-bit, 1 = 2-bit, 2 = 4-bit
    | (0  <<  6)  // DMA_EN: 1 = enable DMA
    | (0  <<  7)  // MISO_CKED: MISO_I pin level value ?
    | (0  <<  8)  // ADDR_BN(3): address byte number
    | (0  << 11)  // WITH_CMD: 1 = send command
    | (0  << 12)  // FF_TRG_LVL(2): fifo trigger level 0 = 1 byte, 1 = 2 bytes, 2 = 4 bytes, 3 = 8 bytes
    | (1  << 15)  // GO_BUSY
    | (0  << 16)  // DUMMY_CYC(4): dummy cycle count
    | (0  << 20)  // BYTE4EN: 4 bytes address cycle enable in dmmr_mode
    | (0  << 21)  // BYTE4CMD: 4 bytes address cmd enable in dmmr_mode
  );

  regs->TRAN_NUM = 0;

  SetSpeed(speed);
}

void TTftLcd_sg_spinor::SetSpeed(uint32_t aspeed)
{
  speed = aspeed;

  uint32_t clkdiv;
  clkdiv = basespeed / speed;

  ctrl_d8 &= ~(0xFFF);
  ctrl_d8 |= (clkdiv - 1);
  regs->SPI_CTRL = ctrl_d8;
}

void TTftLcd_sg_spinor::SpiSendData8(uint8_t adata)
{
  regs->TRAN_NUM = 1;
  regs->TRAN_CSR = tran_csr_base;
  *(uint8_t *)&regs->FF_PORT = adata; // push the tx data into the FIFO

  while (regs->TRAN_CSR & (1 << 15))  // still busy?
  {
    // wait
  }
}

void TTftLcd_sg_spinor::SpiSendData16(uint16_t adata)
{
  regs->TRAN_NUM = 2;
  regs->TRAN_CSR = tran_csr_base;

  *(uint8_t *)&regs->FF_PORT = (adata >> 8); // push the tx data into the FIFO
  *(uint8_t *)&regs->FF_PORT = adata; // push the tx data into the FIFO

  while (regs->TRAN_CSR & (1 << 15))  // still busy?
  {
    // wait
  }
}

void TTftLcd_sg_spinor::ResetPanel()
{
	if (!pin_reset.Assigned())
	{
		return;
	}

	// issue hardware reset
	pin_reset.Set1();
	pin_cs.Set0();

	delay_ms(150);

	pin_reset.Set0();

	delay_ms(150);

	pin_reset.Set1();

	delay_ms(10);

	pin_cs.Set1();
}

void TTftLcd_sg_spinor::WriteCmd(uint8_t adata)
{
	pin_cd.Set0();
	pin_cs.Set0();
	SpiSendData8(adata);
	pin_cs.Set1();
}

void TTftLcd_sg_spinor::WriteData8(uint8_t adata)
{
	pin_cd.Set1();
	pin_cs.Set0();
	SpiSendData8(adata);
	pin_cs.Set1();
}

void TTftLcd_sg_spinor::WriteData16(uint16_t adata)
{
	pin_cd.Set1();
	pin_cs.Set0();
	SpiSendData16(adata);
	pin_cs.Set1();
}

void TTftLcd_sg_spinor::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h)
{
  if (rotation & 1)
  {
    x0 += shift_y;
    y0 += shift_x;
  }
  else
  {
    x0 += shift_x;
    y0 += shift_y;
  }

	uint16_t x1 = x0 + w - 1;
	uint16_t y1 = y0 + h - 1;


	WriteCmd(0x2A);
	WriteData8(x0 >> 8);
	WriteData8(x0);
	WriteData8(x1 >> 8);
	WriteData8(x1);

	WriteCmd(0x2b);
	WriteData8(y0 >> 8);
	WriteData8(y0);
	WriteData8(y1 >> 8);
	WriteData8(y1);

	WriteCmd(0x2c);
}

void TTftLcd_sg_spinor::FillColor(uint16_t acolor, unsigned acount)
{
	pin_cd.Set1();
	pin_cs.Set0();
  while (acount > 0)
  {
  	SpiSendData16(acolor);
  	--acount;
  }
	pin_cs.Set1();
}

void TTftLcd_sg_spinor::BlockFillBegin()
{
  pin_cd.Set1();
  pin_cs.Set0();
}

void TTftLcd_sg_spinor::BlockFill(uint16_t acolor, unsigned acount)
{
  while (acount > 0)
  {
    SpiSendData16(acolor);
    --acount;
  }
}

void TTftLcd_sg_spinor::BlockFillEnd()
{
  pin_cs.Set1();
}

#endif
