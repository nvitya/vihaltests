/*
 * wifi_cyw43_spi.cpp
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

/* The CYW43 SPI protocol is described in the CYS43439 Datasheet */

#include <wifi_cyw43_spi.h>
#include "clockcnt.h"
#include "traces.h"

/*
Command Word:
  Bit    31:  0 = read, 1 = write
  Bit    30:  1 = increment
  Bit 29-28:  function, 0 = BUS, 1 = BACKPLANE, 2 = WLAN
  Bit 27-11:  address
  bit  10-0:  byte count

*/

inline uint32_t make_cmd(bool write, bool inc, uint32_t fn, uint32_t addr, uint32_t sz)
{
  return (0
    | (write << 31)
    | (inc   << 30)
    | (fn    << 28)
    | ((addr & 0x1ffff) << 11)
    | (sz    <<  0)
  );
}

bool TWifiCyw43Spi::Init(TPioAppCyw43Spi * apioapp)
{
  initialized = false;
  pcomm = apioapp;

  if (!InitBaseComm())
  {
    return false;
  }

  if (!InitBackPlane())
  {
    return false;
  }

  initialized = true;
  return true;
}

bool TWifiCyw43Spi::InitBaseComm()
{
  pcomm->ResetModule();

  unsigned t0 = CLOCKCNT;
  uint32_t v;
  unsigned trycnt = 0;

  // 1. Read the SPI READ TEST register to check the communication
  // (16-bit swapped commands for now)
  while (true)
  {
    v = ReadCmdU32(0xA0044000);
    if (0xBEADFEED == v)
    {
      break;
    }

    ++trycnt;
    if (trycnt > 10)
    {
      TRACE("CYW43: SPI Response error = 0x%08X\r\n", v);
      return false;
    }
    delay_ms(1);
  }

  // 2. Switch to 32-bit Little Endian Mode (WARNING: different from the PICO-SDK !)
  WriteCmdU32(0x0004C000, 0x04B10002);

  // check if the requested byte order working properly
  if (ReadCmdU32(0x4000A004) != 0xFEEDBEAD)
  {
    TRACE("CYW43: byte order error\r\n");
    return false;
  }

  WriteSpiReg(0x18, 0x12345678, 4);
  v = ReadSpiReg(0x18, 4);
  if (v != 0x12345678)
  {
    TRACE("CYW43: register write test error: 0x%08X\r\n", v);
    return false;
  }

  //WriteReg(0x18 + 2, 0x0000BBAA, 2);
  //v = ReadReg(0x18, 4);

  return true;
}

bool TWifiCyw43Spi::InitBackPlane()
{
  uint32_t tmp;
  uint32_t trycnt;

  WriteSpiReg(0x1C + 1, CYWBPL_READ_PAD_WORDS << 2, 1);  // F1 response delay bytes

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
  WriteSpiReg(0x04, tmp, 2);  // clear the selected interrupt flags

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
  WriteSpiReg(0x06, tmp, 2);  // enable the selected interrupts

  // cyw43_write_reg_u8(self, BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, SBSDIO_ALP_AVAIL_REQ);
  WriteBplReg(0x1000E, 0x08, 1);  // set the request for ALP
  trycnt = 0;
  while (true)
  {
    tmp = ReadBplReg(0x1000E, 1);  // read back the CHIP_CLOCK_CSR
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

  WriteBplReg(0x1000E, 0, 1);  // clear the request for ALP

  TRACE("ALP is set.\r\n");

  return true;
}

uint32_t TWifiCyw43Spi::ReadCmdU32(uint32_t acmd)
{
  txbuf[0] = acmd;
  rxbuf[0] = 0;
  pcomm->SpiTransfer(&txbuf[0],  1, &rxbuf[0], 1);
  return rxbuf[0];
}

void TWifiCyw43Spi::WriteCmdU32(uint32_t acmd, uint32_t avalue)
{
  txbuf[0] = acmd;
  txbuf[1] = avalue;
  pcomm->SpiTransfer(&txbuf[0],  2, &rxbuf[0], 0);
}

void TWifiCyw43Spi::WriteSpiReg(uint32_t addr, uint32_t value, uint32_t len)
{
  txbuf[0] = (0
    | (len    <<  0)  // write size: 1..4
    | (addr   << 11)  // 15 bit address + 2 bit function
    | (0      << 28)  // 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // 1 = address increment
    | (1      << 31)  // 1 = write
  );
  txbuf[1] = value;
  pcomm->SpiTransfer(&txbuf[0],  2, &rxbuf[0], 0);
}

uint32_t TWifiCyw43Spi::ReadSpiReg(uint32_t addr, uint32_t len)
{
  txbuf[0] = (0
    | (len    <<  0)  // read size: 1..4
    | (addr   << 11)  // 15 bit address + 2 bit function
    | (0      << 28)  // 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // 1 = address increment
    | (0      << 31)  // 0 = read
  );
  pcomm->SpiTransfer(&txbuf[0],  1, &rxbuf[0], 1);
  return rxbuf[0];
}

void TWifiCyw43Spi::WriteBplReg(uint32_t addr, uint32_t value, uint32_t len)
{
  txbuf[0] = (0
    | (len    <<  0)  // write size: 1..4
    | (addr   << 11)  // 15 bit address + 2 bit function
    | (1      << 28)  // 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // 1 = address increment
    | (1      << 31)  // 1 = write
  );
  txbuf[1] = value;
  pcomm->SpiTransfer(&txbuf[0],  2, &rxbuf[0], 0);
}

uint32_t TWifiCyw43Spi::ReadBplReg(uint32_t addr, uint32_t len)
{
  txbuf[0] = (0
    | (len    <<  0)  // read size: 1..4
    | (addr   << 11)  // 15 bit address + 2 bit function
    | (1      << 28)  // 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // 1 = address increment
    | (0      << 31)  // 0 = read
  );
  pcomm->SpiTransfer(&txbuf[0],  1, &rxbuf[0], CYWBPL_READ_PAD_WORDS + 1);
  return rxbuf[CYWBPL_READ_PAD_WORDS];
}
