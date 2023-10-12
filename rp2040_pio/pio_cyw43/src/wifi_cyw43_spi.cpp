/*
 * wifi_cyw43_spi.cpp
 *
 *  Created on: Oct 10, 2023
 *      Author: vitya
 */

#include <wifi_cyw43_spi.h>
#include "stdlib.h"
#include "string.h"
#include "traces.h"
#include "clockcnt.h"
#include "vrofs.h"

#define CYW43_FW_BUF_SIZE  4096  // allocated on heap

bool TWifiCyw43Spi::Init(TWifiCyw43SpiComm * acomm, TSpiFlash * aspiflash)
{
  initialized = false;

  pcomm = acomm;
  pspiflash = aspiflash;

  if (!InitBackPlane())
  {
    return false;
  }

  if (!LoadFirmware())
  {
    if (fwbuf)
    {
      free(fwbuf);
      fwbuf = nullptr;
    }
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

bool TWifiCyw43Spi::LoadFirmware()
{
  TRACE("CYW43: Loading Firmware...\r\n");

  fwbuf = (uint8_t *)malloc(CYW43_FW_BUF_SIZE);  // will be freed outside on error

  pspiflash->StartReadMem(fw_storage_addr, fwbuf, CYW43_FW_BUF_SIZE);
  pspiflash->WaitForComplete();

  TVrofsMainHead * pmh = (TVrofsMainHead *)fwbuf;
  // head consistency check
  if (memcmp(pmh->vrofsid, VROFS_ID_10, 8) != 0)
  {
    TRACE("VROFS Firmware Storage was not in SPI Flash at 0x%06X\r\n", fw_storage_addr);
    TRACE("HINT: Use the picotool to upload the cyw43439_fw.vrofs.bin file into the SPI Flash.\r\n");
    return false;
  }

  if (pmh->main_head_bytes != sizeof(TVrofsMainHead))
  {
    TRACE("Invalid VROFS main head.\r\n");
    return false;
  }

  TRACE("VROFS found.\r\n");

  // search for the firmware name

  uint8_t * iptr8 = (uint8_t *)(pmh + 1);
  uint8_t * iend8 = iptr8 + pmh->index_block_bytes;

  TVrofsIndexRec * pirec; // warning the TVrofsIndexRec is usually bigger than the actual size of the index record
  while (true)
  {
    if (iptr8 >= iend8)
    {
      TRACE("  FW file \"%s\" was not found in the VROFS.\r\n", fw_file_name);
      return false;
    }
    pirec = (TVrofsIndexRec *)iptr8;
    if ((strlen(fw_file_name) == pirec->path_len) && (strncmp(pirec->path, fw_file_name, pirec->path_len) == 0))
    {
      // found it!
      break;
    }
    iptr8 += pmh->index_rec_bytes;
  }

  TVrofsMainHead mh   = *pmh;   // copy the main header, because the buffer will be reloaded
  TVrofsIndexRec irec = *pirec; // copy the index rec, because the buffer will be reloaded

  TRACE("FW File \"%s\" was found, size = %u\r\n", irec.path, irec.data_bytes);

  uint32_t nvsaddr = fw_storage_addr + mh.main_head_bytes + mh.index_block_bytes + irec.offset; // beginning of the data
  uint32_t nvsend = nvsaddr + irec.data_bytes;

  uint32_t bpladdr = 0;
  uint32_t remaining = irec.data_bytes;

  // load the first chunk of the data
  pspiflash->StartReadMem(nvsaddr, fwbuf, CYW43_FW_BUF_SIZE);
  pspiflash->WaitForComplete();

  TRACE("First chunk of the FW data was loaded.\r\n");

  // transfer to the CHIP...
  //cyw43_download_resource(self, 0x00000000, CYW43_WIFI_FW_LEN, 0, fw_data);
  pcomm->WriteBplAddrBlock(bpladdr, fwbuf, 64);


  free(fwbuf);
  fwbuf = nullptr;
  return true;
}
