/*
 * wifi_cyw43_spi.cpp
 *
 *  Created on: Oct 10, 2023
 *      Author: vitya
 */

#include <wifi_cyw43_spi.h>
#include "traces.h"
#include "clockcnt.h"

bool TWifiCyw43Spi::Init(TWifiCyw43SpiComm * acomm)
{
  initialized = false;

  pcomm = acomm;

  if (!InitBackPlane())
  {
    return false;
  }

  initialized = true;
  return true;
}

bool TWifiCyw43Spi::InitBackPlane()
{
  uint32_t tmp;
  uint32_t trycnt;

  pcomm->WriteSpiReg(0x1C + 1, CYWBPL_READ_PAD_WORDS << 2, 1);  // F1 response delay bytes

  tmp = (0
    | (1 <<  0) // DATA_UNAVAILABLE
    | (0 <<  1) // F2_F3_FIFO_RD_UNDERFLOW
    | (0 <<  2) // F2_F3_FIFO_WR_OVERFLOW
    | (1 <<  3) // COMMAND_ERROR
    | (1 <<  4) // DATA_ERROR
    | (0 <<  5) // F2_PACKET_AVAILABLE
    | (0 <<  6) // F3_PACKET_AVAILABLE
    | (1 <<  7) // F1_OVERFLOW
    | (0 <<  8) // GSPI_PACKET_AVAILABLE
    | (0 <<  9) // MISC_INTR1
    | (0 << 10) // MISC_INTR2
    | (0 << 11) // MISC_INTR3
    | (0 << 12) // MISC_INTR4
    | (0 << 13) // F1_INTR
    | (0 << 14) // F2_INTR
    | (0 << 15) // F3_INTR
  );
  pcomm->WriteSpiReg(0x04, tmp, 2);  // clear the selected interrupt flags

  tmp = (0
    | (0 <<  0) // DATA_UNAVAILABLE
    | (1 <<  1) // F2_F3_FIFO_RD_UNDERFLOW
    | (1 <<  2) // F2_F3_FIFO_WR_OVERFLOW
    | (1 <<  3) // COMMAND_ERROR
    | (1 <<  4) // DATA_ERROR
    | (1 <<  5) // F2_PACKET_AVAILABLE
    | (0 <<  6) // F3_PACKET_AVAILABLE
    | (1 <<  7) // F1_OVERFLOW
    | (0 <<  8) // GSPI_PACKET_AVAILABLE
    | (0 <<  9) // MISC_INTR1
    | (0 << 10) // MISC_INTR2
    | (0 << 11) // MISC_INTR3
    | (0 << 12) // MISC_INTR4
    | (0 << 13) // F1_INTR
    | (0 << 14) // F2_INTR
    | (0 << 15) // F3_INTR
  );
  pcomm->WriteSpiReg(0x06, tmp, 2);  // enable the selected interrupts

  // cyw43_write_reg_u8(self, BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, SBSDIO_ALP_AVAIL_REQ);
  pcomm->WriteBplReg(0x1000E, 0x08, 1);  // set the request for ALP
  trycnt = 0;
  while (true)
  {
    tmp = pcomm->ReadBplReg(0x1000E, 1);  // read back the CHIP_CLOCK_CSR
    if (tmp & 0x40)
    {
      break; // ALP avail is set
    }
    ++trycnt;
    if (trycnt > 10)
    {
      TRACE("CYW43: waiting for ALP to be set, CSR=0x%08X\r\n", tmp);
      return false;
    }
    delay_ms(1);
  }

  pcomm->WriteBplReg(0x1000E, 0, 1);  // clear the request for ALP

  //TRACE("ALP is set.\r\n");

  // check reset states
  tmp = pcomm->ReadArmCoreReg(0x800, 1);  // reset control
  if (0 == (tmp & 1))
  {
    TRACE("  ARM Core is not in reset!\r\n");
    return false;
  }

  tmp = pcomm->ReadSocRamReg(0x800, 1);  // reset control
  if (0 == (tmp & 1))
  {
    TRACE("  SOC-RAM is not in reset!\r\n");
    return false;
  }

  TRACE("CYW43: reset states are ok.\r\n");

  ResetDeviceCore(CYW_BPL_ADDR_SOCSRAM);

  // this is 4343x specific stuff: Disable remap for SRAM_3
  pcomm->WriteSocRamReg(0x10, 3, 4);  // SOCSRAM_BANKX_INDEX = 3
  pcomm->WriteSocRamReg(0x44, 0, 4);  // SOCSRAM_BANKX_PDA = 0

  // download the firmware...
  TRACE("Here comes the firmware download...\r\n");

  return true;
}

bool TWifiCyw43Spi::ResetDeviceCore(uint32_t abaseaddr)
{
  uint32_t tmp;
  uint32_t base = abaseaddr + CYW_BPL_WRAPPER_REG_OFFS;

  pcomm->WriteBplAddr(base + 0x408, 3, 1);  // SICF_FGC | SICF_CLOCK_EN
  tmp = pcomm->ReadBplAddr(base + 0x408, 1);
  pcomm->WriteBplAddr(base + 0x800, 0, 1);  // remove reset ?
  delay_ms(1);
  pcomm->WriteBplAddr(base + 0x408, 1, 1); // only SICF_CLOCK_EN
  tmp = pcomm->ReadBplAddr(base + 0x408, 1);
  delay_ms(1);

  return true;
}
