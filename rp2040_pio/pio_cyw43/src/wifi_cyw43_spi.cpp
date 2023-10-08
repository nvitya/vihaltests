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
  Bit31:     0 = read, 1 = write
  Bit30:     1 = increment
  Bit28-29:  function, 0 = BUS, 1 = BACKPLANE, 2 = WLAN
  Bit11-27:  address
  bit0-10:   byte count

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
    v = ReadU32(0xA0044000);
    if (0xBEADFEED == v)
    {
      break;
    }

    ++trycnt;
    if (trycnt > 10)
    {
      TRACE("SPI TEST REG Value = 0x%08X\r\n", v);
      return false;
    }
    delay_ms(1);
  }

  // 2. Switch to 32-bit Little Endian Mode (WARNING: different from the PICO-SDK !)
  WriteU32(0x0004C000, 0x04B10002);

  if (ReadU32(0x4000A004) != 0xFEEDBEAD)
  {
    return false;
  }

  WriteU32(0xC000C004, 0x12345678);

  v = ReadU32(0x4000C004);

  WriteU32(make_cmd(true, true, 0, 0x18 + 2, 2), 0x0000BBAA);

  v = ReadU32(0x4000C004);

  return true;
}

uint32_t TWifiCyw43Spi::ReadU32(uint32_t acmd)
{
  txbuf[0] = acmd;
  rxbuf[0] = 0;
  pcomm->SpiTransfer(&txbuf[0],  1, &rxbuf[0], 1);
  return rxbuf[0];
}

void TWifiCyw43Spi::WriteU32(uint32_t acmd, uint32_t avalue)
{
  txbuf[0] = acmd;
  txbuf[1] = avalue;
  pcomm->SpiTransfer(&txbuf[0],  2, &rxbuf[0], 0);
}
