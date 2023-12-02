/*
 * soft_spi.cpp
 *
 *  Created on: Dec 2, 2023
 *      Author: vitya
 */

#include <soft_spi.h>
#include "clockcnt.h"

bool TSoftSpi::Init()
{
  initialized = false;

  if (!pin_clk or !pin_miso or !pin_mosi)
  {
    return false;
  }

  initialized = true;
  return true;
}

void TSoftSpi::Transfer(uint32_t acmd, uint32_t aaddr, uint32_t aflags,
    uint32_t alen, uint8_t * asrc, uint8_t * adst)
{
  if (!initialized) return;

  // prepare the header

  hcnt = 0;

  if (aflags & SPITR_CMD_MASK) // send command ?
  {
    data_cmd = acmd;
    unsigned cmdlen = (aflags & 0x7);
    if (aflags & SPITR_CMD)
    {
      cmdlen = default_cmd_len;
    }

    header[hcnt++] = (acmd & 0xFF);
    if (cmdlen > 1) header[hcnt++] = ((acmd >>  8) & 0xFF);
    if (cmdlen > 2) header[hcnt++] = ((acmd >> 16) & 0xFF);
    if (cmdlen > 3) header[hcnt++] = ((acmd >> 24) & 0xFF);
  }

  if (aflags & SPITR_ADDR_MASK) // send address ?
  {
    data_addr = aaddr;
    unsigned addrlen = ((aflags >> 4) & 7);
    if (aflags & SPITR_ADDR) // use default size?
    {
      addrlen = default_addr_len;
    }
    if (bigendian_addr)
    {
      data_addr = (__builtin_bswap32(data_addr) >> (8 * (4 - addrlen)));
    }

    header[hcnt++] = (data_addr & 0xFF);
    if (addrlen > 1) header[hcnt++] = ((data_addr >>  8) & 0xFF);
    if (addrlen > 2) header[hcnt++] = ((data_addr >> 16) & 0xFF);
    if (addrlen > 3) header[hcnt++] = ((data_addr >> 24) & 0xFF);
  }

  if (aflags & SPITR_EXTRA_MASK) // send extra ?
  {
    data_extra = aaddr;
    unsigned extralen = ((aflags >> 8) & 7);
    if (aflags & SPITR_EXTRA) // use default size?
    {
      extralen = default_extra_len;
    }

    header[hcnt++] = (data_extra & 0xFF);
    if (extralen > 1) header[hcnt++] = ((data_extra >>  8) & 0xFF);
    if (extralen > 2) header[hcnt++] = ((data_extra >> 16) & 0xFF);
    if (extralen > 3) header[hcnt++] = ((data_extra >> 24) & 0xFF);
  }

  BeginTransfer();

  // send the header
  if (hcnt)
  {
    SendAndRecv(hcnt, &header[0], &header_rx[0]);
  }

  // send the body
  if (alen)
  {
    SendAndRecv(alen, asrc, adst);
  }

  EndTransfer();
}

void TSoftSpi::BeginTransfer()
{
  half_clock_ticks = SystemCoreClock / (speed << 1);

  pin_clk->Set0();
  if ((cs_number < SOFT_SPI_MAX_CS) && pin_cs[cs_number])
  {
    pin_cs[cs_number]->Set0();
    delay_clocks(half_clock_ticks);
  }
}

void TSoftSpi::EndTransfer()
{
  if ((cs_number < SOFT_SPI_MAX_CS) && pin_cs[cs_number])
  {
    delay_clocks(half_clock_ticks);
    pin_cs[cs_number]->Set1();
  }
  pin_clk->Set0();
}

void TSoftSpi::SendAndRecv(uint32_t alen, uint8_t * asrc, uint8_t * adst)
{
  unsigned t0, t1;

  uint8_t txdata;
  uint8_t rxdata;

  while (alen)
  {
    txdata = (asrc ? *asrc++ : 0);
    rxdata = 0;
    for (unsigned bitcnt = 0; bitcnt < 8; ++bitcnt)
    {
      // set the TX data bit
      pin_mosi->SetTo((txdata >> (7 - bitcnt)) & 1);
      // hold half clock time
      t0 = CLOCKCNT;
      while (CLOCKCNT - t0 < half_clock_ticks) { }
      // rise the clock
      pin_clk->Set1();
      // hold half clock time
      t0 = CLOCKCNT;
      while (CLOCKCNT - t0 < half_clock_ticks) { }
      // sample the input
      rxdata |= (pin_miso->Value() << (7 - bitcnt));
      pin_clk->Set0();
    }

    if (adst)
    {
      *adst++ = rxdata;
    }

    --alen;
  }
}
