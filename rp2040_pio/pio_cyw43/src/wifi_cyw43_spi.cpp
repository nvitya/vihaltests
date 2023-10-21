/*
 * wifi_cyw43_spi.cpp
 *
 *  Created on: Oct 10, 2023
 *      Author: vitya
 */

/* Copyright header of the cyw43-driver
 * ----------------------------------------------------------------------------
 * This file is part of the cyw43-driver
 *
 * Copyright (C) 2019-2022 George Robotics Pty Ltd
 *
 * >>> 3-clause BSD License Text <<<
 * ----------------------------------------------------------------------------
*/

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "traces.h"
#include "clockcnt.h"
#include "vrofs.h"

#include "wifi_cyw43_spi.h"

#include "wifi_cyw43439_nvram.h"  // wifi_nvram_4343[]

bool TWifiCyw43Spi::Init(TWifiCyw43SpiComm * acomm, TSpiFlash * aspiflash)
{
  initialized = false;

  pcomm = acomm;
  pspiflash = aspiflash;

  mrq.completed = true;
  rqstate = 0;
  use_irq_pin = false;

  if (!InitBackPlane())
  {
    return false;
  }

  if (!LoadFirmware())
  {
    return false;
  }

  if (!PrepareBus())
  {

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
  pcomm->WriteSpiReg(0x04, tmp, 1);  // clear the selected interrupt flags

  irq_mask = (0
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
  pcomm->WriteSpiReg(0x06, irq_mask, 2);  // enable the selected interrupts

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
  uint32_t tmp;
  uint32_t trycnt;

  TRACE("CYW43: Loading Firmware...\r\n");

  uint32_t nvsaddr;
  uint32_t fwlen;

  if (!FindVrofsFile(fw_file_name, &nvsaddr, &fwlen))
  {
    return false;
  }

  LoadFirmwareDataFromNvs(0x00000000, nvsaddr, fwlen);

  // the NVDATA is not so big and in text format, so it is embedded in the code
  uint32_t padded_size = ((sizeof(wifi_nvram_4343) + 63) & 0xFC0); // pad round up to 64 bytes
  TRACE("CYW43: Loading NVRAM data: %u Bytes\r\n", padded_size);
  LoadFirmwareDataFromRam(CYW_BPL_ARM_RAM_SIZE - 4 - padded_size, &wifi_nvram_4343[0], padded_size);

  uint32_t size_code = (padded_size >> 2);  // divide by 4 to represent words
  size_code = ((size_code << 16) | size_code) ^ 0xFFFF0000;  // upper 16 bit = inverted size code
  pcomm->WriteBplAddr(CYW_BPL_ARM_RAM_SIZE - 4, size_code, 4);

  TRACE("CYW43: Starting the ARM core\r\n");

  ResetDeviceCore(CYW_BPL_ADDR_WLAN_ARMCM3);  // start the ARM core

  tmp = pcomm->ReadArmCoreReg(0x408, 1);  // IOCTL
  if (1 != (tmp & 3))
  {
    TRACE("  ARM Core is not up!\r\n");
    // return false;
  }

  tmp = pcomm->ReadArmCoreReg(0x800, 1);  // reset control
  if (tmp & 1)
  {
    TRACE("  ARM Core is in reset!\r\n");
    // return false;
  }

  // switch back to the common addresses
  pcomm->SetBackplaneWindow(CYW_BPL_ADDR_COMMON);

  // wait until HT clock is available; takes about 29ms
  trycnt = 0;
  while (true)
  {
    tmp = pcomm->ReadBplReg(0x1000E, 1); // SDIO_CHIP_CLOCK_CSR)
    if (tmp & 0x80) // SBSDIO_HT_AVAIL
    {
      break;
    }

    ++trycnt;
    if (trycnt > 500)
    {
      TRACE("CYW43: error waiting for HT!\r\n");
      return false;
    }

    delay_ms(1);
  }

  TRACE("ARM Core Preparation is finished.\r\n");

  // Set up the interrupt mask and enable interrupts
  pcomm->WriteSdioReg(0x24, 0x000000F0, 4);  // SDIO_INT_HOST_MASK = I_HMB_SW_MASK

  // switch back to the common addresses, for normal ReadBplRegs()
  pcomm->SetBackplaneWindow(CYW_BPL_ADDR_COMMON);

  // Lower F2 Watermark to avoid DMA Hang in F2 when SD Clock is stopped.
  pcomm->WriteBplReg(0x10008, 32, 1);

  trycnt = 0;
  while (true)
  {
    tmp = pcomm->ReadSpiReg(0x0008, 4); // SPI_STATUS_REGISTER
    if (tmp & 0x20) // STATUS_F2_RX_READY
    {
      break;
    }

    ++trycnt;
    if (trycnt > 500)
    {
      TRACE("CYW43: F2 is not ready !\r\n");
      return false;
    }

    delay_ms(1);
  }

  TRACE("F2 is ready.\r\n");

  return true;
}

bool TWifiCyw43Spi::FindVrofsFile(const char * afname, uint32_t * rnvsaddr, uint32_t * rlen)
{
  uint32_t tmp;
  uint32_t trycnt;

  // the wbuf is big enough to hold the whole directory listing
  pspiflash->StartReadMem(fw_storage_addr, &wbuf[0], sizeof(wbuf));
  pspiflash->WaitForComplete();

  TVrofsMainHead * pmh = (TVrofsMainHead *)&wbuf[0];
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

  //TRACE("VROFS found.\r\n");

  // search for the firmware name

  uint8_t * iptr8 = (uint8_t *)(pmh + 1);
  uint8_t * iend8 = iptr8 + pmh->index_block_bytes;

  TVrofsIndexRec * pirec; // warning the TVrofsIndexRec is usually bigger than the actual size of the index record
  while (true)
  {
    if (iptr8 >= iend8)
    {
      TRACE("  FW file \"%s\" was not found in the VROFS.\r\n", afname);
      return false;
    }
    pirec = (TVrofsIndexRec *)iptr8;
    if ((strlen(afname) == pirec->path_len) && (strncmp(pirec->path, afname, pirec->path_len) == 0))
    {
      // found it!
      break;
    }
    iptr8 += pmh->index_rec_bytes;
  }

  TVrofsMainHead mh   = *pmh;   // copy the main header, because the buffer will be reloaded
  TVrofsIndexRec irec = *pirec; // copy the index rec, because the buffer will be reloaded

  TRACE("  File \"%s\" was found, size = %u\r\n", irec.path, irec.data_bytes);

  // calculate the spi flash address of the beginning of the actual data:
  *rnvsaddr = fw_storage_addr + mh.main_head_bytes + mh.index_block_bytes + irec.offset;
  *rlen = irec.data_bytes;

  return true;
}

bool TWifiCyw43Spi::PrepareBus()
{
  uint32_t tmp;
  uint32_t trycnt;

  TRACE("Preparing Bus\r\n");

  // Setting KSO = Keep SDIO On:

  tmp = pcomm->ReadBplReg(0x1001E, 1);  // SDIO_WAKEUP_CTRL
  tmp |= (1 << 1); // SBSDIO_WCTRL_WAKE_TILL_HT_AVAIL;
  pcomm->WriteBplReg(0x1001E, tmp, 1); // SDIO_WAKEUP_CTRL
  pcomm->WriteSpiReg(0xF0, 0x08, 1);  // SDIOD_CCCR_BRCM_CARDCAP = SDIOD_CCCR_BRCM_CARDCAP_CMD_NODEC
  pcomm->WriteBplReg(0x1000E, 0x02, 1); // SDIO_CHIP_CLOCK_CSR = SBSDIO_FORCE_HT

  tmp = pcomm->ReadBplReg(0x1001F, 1); // SDIO_SLEEP_CSR
  if (0 == (tmp & 1))  // is the SBSDIO_SLPCSR_KEEP_SDIO_ON not set ?
  {
    tmp |= 1; // then set the SBSDIO_SLPCSR_KEEP_SDIO_ON
    pcomm->WriteBplReg(0x1001F, tmp, 1); // SDIO_SLEEP_CSR
  }

  // Put SPI interface block to sleep
  pcomm->WriteBplReg(0x1000F, 0x0F, 1); // SDIO_PULL_UP = 0x0F

  // CLEAR PAD PULLS
  pcomm->WriteBplReg(0x1000F, 0x00, 1); // SDIO_PULL_UP = 0x00
  tmp = pcomm->ReadBplReg(0x1000F,  1);

  // We always seem to start with a data unavailable error - so clear it now
  tmp = pcomm->ReadSpiReg(0x0004, 2);  // SPI_INTERRUPT_REGISTER
  if (tmp & 1)  // DATA_UNAVAILABLE is set ?
  {
    pcomm->WriteSpiReg(0x0004, tmp, 2); // clear the interrupt flags (all of them)
  }

  // ensure that the KSO is on:
  trycnt = 0;
  while (true)
  {
    tmp = pcomm->ReadBplReg(0x1001F, 1); // SDIO_SLEEP_CSR
    if ((tmp != 0xFF) && (tmp & 1))
    {
      break;
    }

    ++trycnt;
    if (trycnt > 64)
    {
      TRACE("CYW43: KSO set failed !\r\n");
      return false;
    }

    pcomm->WriteBplReg(0x1001F, 1, 1); // try again.
    delay_ms(1);
  }

  // Now, communicate mainly with packets on the F2 / WIFI buffer

  // Operate the state machine for 20 ms, to read the two (?) initial async events
  unsigned t0 = CLOCKCNT;
  unsigned wait_clocks = 10 * (SystemCoreClock / 1000);
  while (CLOCKCNT - t0 < wait_clocks)
  {
    Run();
  }

  if (!LoadClmData())
  {
    return false;
  }

  WriteIoVarU32("bus:txglom", 0); // tx glomming off
  WriteIoVarU32("apsta", 1);      // apsta on

  // get the MAC address
  if (!ReadIoVar("cur_etheraddr", &macaddr[0], 6))
  {
    TRACE("Error getting the MAC address!\r\n");
    return false;
  }

  TRACE("MAC address = %02X:%02X:%02X:%02X:%02X:%02X\r\n",
      macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

  TRACE("CYW43 bus prepare finished.\r\n");
  return true;
}

struct TClmLoadParams
{
  uint16_t  flag;
  uint16_t  param2;  // unknown meaning
  uint32_t  len;
  uint32_t  param4;  // unknown meaning
};

bool TWifiCyw43Spi::LoadClmData()
{
  TRACE("Loading the CLM data\r\n");

  uint32_t nvsaddr;
  uint32_t fwlen;
  if (!FindVrofsFile(clm_file_name, &nvsaddr, &fwlen))
  {
    return false;
  }

  // use the end of the wbuf[] as NVS load buffer

  const uint32_t  dload_max_chunk = 512;
  uint8_t *  nvsbuf = &wbuf[sizeof(wbuf) - dload_max_chunk];
  uint32_t   bufsize = dload_max_chunk;

  // assembly the IOCTL request, put the IOCTL parameters before

  uint32_t   vnamelen = 8;  // "clmload\x00" = 8 byte
  uint32_t   paramsize = sizeof(TClmLoadParams) + vnamelen;
  uint8_t *  paramstart = nvsbuf - paramsize;
  memcpy(paramstart, "clmload\x00", vnamelen);
  TClmLoadParams * phead = (TClmLoadParams *)(nvsbuf - sizeof(TClmLoadParams));

  uint32_t   remaining = fwlen;
  uint32_t   chunksize = 0;
  uint32_t   offset = 0;

  while (remaining)
  {
    // load the next chunk of the data
    chunksize = remaining;
    if (chunksize > dload_max_chunk)  chunksize = dload_max_chunk;

    pspiflash->StartReadMem(nvsaddr, nvsbuf, chunksize);
    pspiflash->WaitForComplete();

    phead->flag = (1 << 12);  // DLOAD_HANDLER_VER
    if (0 == offset)             phead->flag |= 2; // DL_BEGIN
    if (remaining == chunksize)  phead->flag |= 4; // DL_END
    phead->param2 = 2;
    phead->len  = chunksize;
    phead->param4 = 0;

    // the data follows this header

    // assembly the IOCTL request

    mrq.channel = CYW43_CH_IOCTL;
    mrq.dataptr = paramstart;
    mrq.datalen = paramsize + chunksize;

    // don't expect data response here ?
    mrq.anslen = 0;
    mrq.ansptr = nullptr;

    ++ioctl_rq_id;
    mrq.rq_id = ioctl_rq_id;
    mrq.flags = (0
      | (0  << 12)  // IFACE(4): 0 = STA, 1 = AP, 2 = P2P
      | (2  <<  0)  // KIND: 0 = GET, 2 = SET
    );
    mrq.cmd = 263;  // 263 = WLC_SET_VAR

    if (!AddRequest(&mrq))
    {
      return false;
    }

    while (!mrq.completed)
    {
      Run();
    }

    remaining    -= chunksize;
    offset       += chunksize;
    nvsaddr      += chunksize;
  }

  TRACE("  %u Bytes loaded.\r\n", fwlen);

  // check the load status

  uint32_t load_status;

  if (!ReadIoVar("clmload_status", &load_status, sizeof(load_status)))
  {
    TRACE("  error getting load status!\r\n");
    return false;
  }

  if (0 != load_status)
  {
    TRACE("  CLM load error = %u\r\n", load_status);
    return false;
  }

  return true;
}

bool TWifiCyw43Spi::GpioSetTo(uint32_t gpio_num, uint8_t avalue)
{
  uint32_t params[2];
  params[0] = (1 << gpio_num);
  params[1] = (avalue ? 1 << gpio_num : 0);
  return WriteIoVar("gpioout", &params[0], 8);
}

bool TWifiCyw43Spi::WriteIoVarU32(const char * iovar_name, uint32_t avalue)
{
  uint32_t params[1];
  params[0] = avalue;
  return WriteIoVar(iovar_name, &params[0], 4);
}

bool TWifiCyw43Spi::WriteIoVar(const char * iovar_name, void * params, uint32_t parlen)
{
  // finish already running transaction
  while (!mrq.completed)
  {
    Run();
  }

  // prepare the payload into the mrqdata buffer

  uint8_t *       pdatabegin = &mrqdata[0];
  uint8_t *       pdata = pdatabegin;

  int vnamelen = strlen(iovar_name) + 1; // include the closing zero too

  if (vnamelen + parlen > sizeof(mrqdata))
  {
    TRACE("CYW43 WriteIoVar: rq data too big\r\n");
    return false;
  }

  memcpy(pdata, iovar_name, vnamelen);
  pdata += vnamelen;
  memcpy(pdata, params, parlen);
  pdata += parlen;

  mrq.channel = CYW43_CH_IOCTL;
  mrq.dataptr = pdatabegin;
  mrq.datalen = pdata - pdatabegin;

  // don't expect data response here
  mrq.anslen = 0;
  mrq.ansptr = nullptr;

  ++ioctl_rq_id;
  mrq.rq_id = ioctl_rq_id;
  mrq.flags = (0
    | (0  << 12)  // IFACE(4): 0 = STA, 1 = AP, 2 = P2P
    | (2  <<  0)  // KIND: 0 = GET, 2 = SET
  );
  mrq.cmd = 263;  // 263 = WLC_SET_VAR

  if (!AddRequest(&mrq))
  {
    return false;
  }

  while (!mrq.completed)
  {
    Run();
  }

  return (mrq.error == 0);
}

bool TWifiCyw43Spi::ReadIoVar(const char * iovar_name, void * dstbuf, uint32_t len)
{
  // finish already running transaction
  while (!mrq.completed)
  {
    Run();
  }

  // prepare the payload into the mrqdata buffer

  uint8_t *       pdatabegin = &mrqdata[0];
  uint8_t *       pdata = pdatabegin;

  int vnamelen = strlen(iovar_name) + 1; // include the closing zero too

  if (vnamelen > sizeof(mrqdata))
  {
    TRACE("CYW43 ReadIoVar: rq data too big\r\n");
    return false;
  }

  memcpy(pdata, iovar_name, vnamelen);
  pdata += vnamelen;
  memset(pdata, 0, len);
  pdata += len;

  mrq.channel = CYW43_CH_IOCTL;
  mrq.dataptr = pdatabegin;
  mrq.datalen = pdata - pdatabegin;

  mrq.anslen = len;
  mrq.ansptr = (uint8_t *)dstbuf;

  ++ioctl_rq_id;
  mrq.rq_id = ioctl_rq_id;
  mrq.flags = (0
    | (0  << 12)  // IFACE(4): 0 = STA, 1 = AP, 2 = P2P
    | (0  <<  0)  // KIND: 0 = GET, 2 = SET
  );
  mrq.cmd = 262;  // 262 = WLC_GET_VAR

  if (!AddRequest(&mrq))
  {
    return false;
  }

  while (!mrq.completed)
  {
    Run();
  }

  return (mrq.error == 0);
}


bool TWifiCyw43Spi::AddRequest(TCyw43Request * arq)
{
  if (currq)
  {
    // search the last rq + check if already added
    TCyw43Request * rq = currq;
    while (rq->next)
    {
      if (rq == arq)
      {
        return false; // already added
      }
      rq = rq->next;
    }
    rq->next = arq;
  }
  else
  {
    currq = arq; // set as first
  }

  arq->completed = false;
  arq->error = 0;
  arq->next = nullptr;
  return true;
}

void TWifiCyw43Spi::Run()
{
  if (!pcomm->SpiTransferFinished())
  {
    return; // the SPI line is busy
  }

  if (1 == wreadstate)  // packet read finished, process the packet
  {
    ProcessRxPacket();
    wreadstate = 0;
  }

  if (currq && (0 == rqstate))
  {
    if (SendRequest(currq))
    {
      rqstate = 1; // go to wait response state
      return; // the SPI line is busy
    }
    else // error
    {
      currq->completed = true;
      currq->error = CYW43_ERR_RQ;
    }
  }

  if ((0 == wreadstate) && (!use_irq_pin || (1 == pcomm->pin_irq.Value())))  // check for new incoming packets
  {
    // read the following three registers with one transaction:
    //   0x04 u16: SPI interrupts
    //   0x06 u16: SPI IRQ Mask (not used here)
    //   0x08 u32: SPI status (packet available, packet length)

    uint32_t cmd = (0
      | (8    <<  0)  // read size: 8 bytes
      | (0x04 << 11)  // ADDR(17): 17 bit address
      | (0    << 28)  // FUNC(2): 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
      | (1    << 30)  // INC: 1 = address increment
      | (0    << 31)  // R/W: 0 = read
    );
    pcomm->SpiTransfer(cmd, false, &spiregs[0], 2);  // blocking read, but this is short

    gspi_status = spiregs[1];
    irq_status = (spiregs[0] & irq_mask);

    uint16_t err_mask = (0
      | (1 <<  1) // F2_F3_FIFO_RD_UNDERFLOW
      | (1 <<  2) // F2_F3_FIFO_WR_OVERFLOW
      | (1 <<  3) // COMMAND_ERROR
      | (1 <<  4) // DATA_ERROR
      | (1 <<  7) // F1_OVERFLOW
    );

    if (irq_status & err_mask)
    {
      ++errcnt_irq_status;
      TRACE("CYW43: IRQ ERROR FLAGS = 0x04X\r\n", irq_status);
      pcomm->WriteSpiReg(0x04, irq_status & err_mask, 2);  // clear the IRQ flags
      irq_status &= ~err_mask;
    }

    if ((gspi_status != 0xFFFFFFFF) && (gspi_status & (1 << 8))) // packet available ?
    {
      wreadlen = (gspi_status >> 9) & 0x7FF;
      if ((wreadlen == 0) || (wreadlen > 1544) || (gspi_status & 2))
      {
        TRACE("CYW43: Dropping invalid frame (u)\r\n", wreadlen);
        pcomm->WriteBplReg(0x1000D, 1, 1);  // drops the frame ?
      }
      else
      {
        pcomm->StartReadWlanBlock(0, &wbuf[0], wreadlen);
        wreadstate = 1;
        return; // SPI line busy
      }
    }
    else // no data packet available
    {
      if (irq_status & CYW43_IRQ_F2_PACKET) // this probably never happen
      {
        use_irq_pin = false;  // something wrong, go back to register polling
      }
      else // packet reads automatically clear the F2 PACKET flag
      {
        use_irq_pin = true; // use the IRQ pin only when all the interrupts are successfully cleared
      }
    }
  }

  // handle finished transactions and timeouts

  if (currq)
  {
    if (currq->completed)
    {
      currq = currq->next;
      rqstate = 0;
    }
    else if (rqstate > 0)
    {
      // handle response timeouts
    }
  }
}

bool TWifiCyw43Spi::SendRequest(TCyw43Request * arq)
{
  uint32_t   tmp;
  uint8_t *  pdata;
  uint8_t *  pdatabegin;

  // prepare the SDPCM Header

  TSdpcmHeader *  psdh  = (TSdpcmHeader *)&wbuf[0];

  //psdh->size     = (pdata - &wbuf[0]);    // the total packet size
  //psdh->size_com = (psdh->size ^ 0xffff); // inverted size
  // (size will be set later)
  psdh->sequence = sdpcm_tx_seq_num;
  psdh->channel_and_flags = arq->channel;
  psdh->next_length = 0;
  psdh->header_length = sizeof(TSdpcmHeader);
  if (CYW43_CH_DATA == arq->channel)
  {
    psdh->header_length += 2;  // speciality, for the right alignment +2 bytes added to the data header too
  }
  psdh->wireless_flow_control = 0;
  psdh->bus_data_credit = 0;
  psdh->reserved[0] = 0;
  psdh->reserved[1] = 0;

  pdata = &wbuf[psdh->header_length]; // beginning of the header

  if (CYW43_CH_IOCTL == arq->channel)
  {
    TIoctlHeader *  pich  = (TIoctlHeader *)pdata;

    pich->cmd = arq->cmd;
    pich->txlen = arq->datalen;
    pich->rxlen = 0;
    pich->flags = arq->flags;
    pich->rq_id = arq->rq_id;
    pich->status = 0;

    pdata += sizeof(TIoctlHeader);
    pdatabegin = pdata;
  }
  else if (CYW43_CH_DATA == arq->channel)
  {
    TBdcHeader *  pdh = (TBdcHeader *)pdata;
    pdata += sizeof(TBdcHeader) + 2;  // extra padding
    pdatabegin = pdata;
  }
  else // unsupported request
  {
    arq->error = CYW43_ERR_RQ;
    arq->completed = true;
    return false;
  }

  // copy the payload into the buffer

  if (arq->datalen)
  {
    memcpy(pdata, arq->dataptr, arq->datalen);
    pdata += arq->datalen;
  }

  // update the SDPCM size field
  psdh->size     = (pdata - &wbuf[0]);    // the total packet size
  psdh->size_com = (psdh->size ^ 0xffff); // inverted size

  ++sdpcm_tx_seq_num;

  // Start sending the request

  pcomm->StartWriteWlanBlock(0, &wbuf[0], psdh->size);  // the buffer must be word aligned !
  return true;
}

void TWifiCyw43Spi::ProcessRxPacket()
{
  //uint32_t   tmp;
  uint8_t *  pdata;
  uint8_t *  pdatabegin;

  TSdpcmHeader *  psdh  = (TSdpcmHeader *)&wbuf[0];

  if ( (psdh->size_com != (psdh->size ^ 0xFFFF))
       || (psdh->size < sizeof(TSdpcmHeader))
       || (psdh->header_length < sizeof(TSdpcmHeader))
     )
  {
    ++errcnt_invalid_rxpkt;
    TRACE("CYW43: Invalid RX packet\r\n");
    return;
  }

  if (wlan_flow_control != psdh->wireless_flow_control)
  {
    wlan_flow_control = psdh->wireless_flow_control;
    TRACE("CYW43: update WLAN flow control: %02X\r\n", wlan_flow_control);
  }

  if (psdh->size == sizeof(TSdpcmHeader))
  {
    return; // flow control packet with no data
  }

  pdata = &wbuf[psdh->header_length];

  uint8_t ch = (psdh->channel_and_flags & 0xF);

  if (CYW43_CH_IOCTL == ch)
  {
    if (psdh->size < sizeof(TSdpcmHeader) + sizeof(TIoctlHeader))
    {
      ++errcnt_invalid_rxpkt;
      TRACE("CYW43: Invalid IOCTL response\r\n");
      return;
    }

    if (!currq || (currq->channel != CYW43_CH_IOCTL))
    {
      ++errcnt_invalid_rxpkt;
      TRACE("CYW43: Unexpected IOCTL response\r\n");
      return;
    }

    TIoctlHeader *  pich  = (TIoctlHeader *)pdata;
    if (pich->rq_id != currq->rq_id)
    {
      ++errcnt_invalid_rxpkt;
      TRACE("CYW43: IOCTL response ID mismatch\r\n");
      return;
    }

    pdata += sizeof(TIoctlHeader);
    pdatabegin = pdata;

    uint32_t len = psdh->size - (pdatabegin - &wbuf[0]);

    //TRACE("CYW43: IOCTL response len=%u, status=%i\r\n", len, pich->status);

    if (currq->anslen)
    {
      // the payload is at the end
      memcpy(currq->ansptr, &wbuf[psdh->size - currq->anslen], currq->anslen);
    }

    currq->completed = true;
    currq->error = pich->status;  // maybe an error code ?
  }
  else if (CYW43_CH_DATA == ch)
  {
    TRACE("CYW43: unprocessed DATA, len=%u\r\n", psdh->size);
  }
  else if (CYW43_CH_EVENT == ch)
  {
    if (psdh->size < sizeof(TSdpcmHeader) + sizeof(TBdcHeader))
    {
      ++errcnt_invalid_rxpkt;
      TRACE("CYW43: Invalid EVENT header\r\n");
      return;
    }

    TBdcHeader *  pbdch = (TBdcHeader *)pdata;
    pdata += sizeof(TBdcHeader) + (pbdch->data_offset << 2);
    pdatabegin = pdata;

    uint32_t len = psdh->size - (pdatabegin - &wbuf[0]);

    TRACE("CYW43: unprocessed EVENT, len=%u, type=%04X\r\n", len, pdata[12] + pdata[13] << 8);
  }
  else
  {
    ++errcnt_invalid_rxpkt;
    TRACE("CYW43: Invalid channel in rx packet\r\n");
    return;
  }
}

void TWifiCyw43Spi::LoadFirmwareDataFromRam(uint32_t abpladdr, const void * srcbuf, uint32_t len)
{
  // slice the fwbuf into two
  uint8_t *  bufptr = (uint8_t *)srcbuf;
  uint32_t   bpladdr = abpladdr;
  uint32_t   remaining = len;
  uint32_t   chunksize = 0;

  while (remaining)
  {
    // transfer to the CHIP...
    chunksize = remaining;
    if (chunksize > 64)  chunksize = 64;

    pcomm->StartWriteBplAddrBlock(bpladdr, (uint32_t *)bufptr, chunksize);  // use the DMA now
    pcomm->SpiTransferWaitFinish();

    remaining    -= chunksize;
    bufptr       += chunksize;
    bpladdr      += chunksize;
  }

  //TRACE("  %u Bytes loaded.\r\n", len);
}

#if 0

// simple interleaved load

void TWifiCyw43Spi::LoadFirmwareDataFromNvs(uint32_t abpladdr, uint32_t anvsaddr, uint32_t len)
{
  // slice the fwbuf into two
  uint32_t   bufsize = CYW43_FW_BUF_SIZE;
  int        bufremaining = 0;
  uint8_t *  bufptr = fwbuf;

  uint32_t   nvsaddr = anvsaddr;
  uint32_t   bpladdr = abpladdr;
  uint32_t   remaining = len;
  uint32_t   chunksize = 0;

  while (remaining)
  {
    if (bufremaining <= 0)
    {
      bufptr = fwbuf;
      if (bufsize > remaining)  bufsize = remaining;

      // load the next chunk of the data
      pspiflash->StartReadMem(nvsaddr, fwbuf, bufsize);
      pspiflash->WaitForComplete();

      bufremaining = bufsize;
      nvsaddr += bufsize;
    }

    while (bufremaining)
    {
      // transfer to the CHIP...
      chunksize = bufremaining;
      if (chunksize > 64)  chunksize = 64;

      //cyw43_download_resource(self, 0x00000000, CYW43_WIFI_FW_LEN, 0, fw_data);
      pcomm->StartWriteBplAddrBlock(bpladdr, (uint32_t *)bufptr, chunksize);  // use the DMA now
      pcomm->SpiTransferWaitFinish();

      bufremaining -= chunksize;
      remaining    -= chunksize;

      bufptr       += chunksize;
      bpladdr      += chunksize;
    }
  }

  TRACE("  %u Bytes loaded.\r\n", len);
}

#else

// overlapped Flash Load and Spi Write

void TWifiCyw43Spi::LoadFirmwareDataFromNvs(uint32_t abpladdr, uint32_t anvsaddr, uint32_t len)
{
  // slice the fwbuf into two
  uint32_t   loadchunk = 64;  // must be fix 64
  uint8_t *  bufptr = &wbuf[0];

  uint32_t   nvsaddr = anvsaddr;
  uint32_t   bpladdr = abpladdr;
  uint32_t   remaining = len;
  uint32_t   nvsremaining = len;

  // preload the first 64
  pspiflash->StartReadMem(nvsaddr, bufptr, loadchunk);
  pspiflash->WaitForComplete();
  nvsaddr      += loadchunk;
  nvsremaining -= loadchunk;

  uint32_t   nvschunk = 0;
  uint32_t   chunksize = 0; // start with zero !
  uint8_t    spiidx = 0; // points to the buffer that can be loded into the CYW

  while (true)
  {
    if (pcomm->SpiTransferFinished() && pspiflash->completed)  // the SPI flash is usually faster
    {
      if (remaining == 0)
      {
        break; // finished ?
      }

      // start a new transfer
      chunksize = remaining;
      if (chunksize > loadchunk)  chunksize = loadchunk;
      pcomm->StartWriteBplAddrBlock(bpladdr, (uint32_t *)bufptr, chunksize);  // use the DMA now
      remaining    -= chunksize;
      bpladdr      += chunksize;

      //bufptr       += chunksize;

      //pcomm->SpiTransferWaitFinish();

      spiidx ^= 1; // flip the buffer for the next transfer, this is where the next chunk will be loaded
      bufptr = &wbuf[spiidx * loadchunk];

      if (nvsremaining) // start the next chunk load from SPI Flash
      {
        nvschunk = nvsremaining;
        if (nvschunk > loadchunk)  nvschunk = loadchunk;

        pspiflash->StartReadMem(nvsaddr, bufptr, nvschunk);

        nvsremaining -= nvschunk;
        nvsaddr      += nvschunk;
      }
    }

    pspiflash->Run();  // operate the SPI flash read in the background
  }

  TRACE("  %u Bytes loaded.\r\n", len);
}

#endif
