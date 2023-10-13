/*
 * wifi_cyw43_spi.cpp
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

/* The CYW43 SPI protocol is described in the CYS43439 Datasheet */

#include "string.h"
#include <wifi_cyw43_spi_comm.h>
#include "clockcnt.h"
#include "traces.h"

void TWifiCyw43SpiComm::ResetModule()
{
  // the reset must be done with data pin low, otherwise the communication won't work,
  // (probably SDIO mode will be selected ?)
  hwpinctrl.PinSetup(0,  pin_data, PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  pin_wlon.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  delay_ms(20);
  pin_wlon.Set1();
  delay_ms(50);

  sm.SetupPioPins(pin_data, 1);  // change back data to PIO alternate function
}

bool TWifiCyw43SpiComm::Init(uint8_t adevnum, uint8_t asmnum)
{
  initialized = false;

  if (!pin_cs.Assigned())    pin_cs.Assign(0, 25, false);
  if (!pin_wlon.Assigned())  pin_wlon.Assign(0, 23, false);

  pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  prg.Init(adevnum, prgoffset);  // offset=0, entry=offset

  // Pin assignments:
  //  - SCK is side-set pin 0
  //  - MOSI/MISO is OUT/IN pin 0
  // x: preloaded to output bit count - 1
  // y: preloaded to input bit count - 1

  prg.Add(0x6001); //  0: out    pins, 1         side 0
  prg.Add(0x1040); //  1: jmp    x--, 0          side 1
  prg.Add(0xe080); //  2: set    pindirs, 0      side 0
  //prg.Add(0x1067); //  3: jmp    !y, 7           side 1  ; this is required for 33 MHz operation !?!
  prg.Add(0x0067); //  3: jmp    !y, 7           side 0
  prg.Add(0xa042); //  4: nop                    side 0
  prg.Add(0x5001); //  5: in     pins, 1         side 1
  prg.Add(0x0085); //  6: jmp    y--, 5          side 0
  prg.Add(0xc020); //  7: irq    wait 0          side 0

  if (!sm.Init(adevnum, asmnum))
  {
    return false;
  }

  sm.SetPrg(&prg);

  sm.SetPinDir(pin_sck,  1);
  sm.SetPinDir(pin_data, 1);

  sm.SetupPinsOut(     pin_data, 1);
  sm.SetupPinsIn(      pin_data, 1);
  sm.SetupPinsSet(     pin_data, 1);
  sm.SetupPinsSideSet( pin_sck,  1);

  sm.SetOutShift(false, true, 32);
  sm.SetInShift( false, true, 32);

  SetFrequency(frequency);

  // ResetModule();

  if (!InitBaseComm())
  {
    return false;
  }

  initialized = true;
  return true;
}

void TWifiCyw43SpiComm::SetFrequency(unsigned afreq)
{
  frequency = afreq;
  sm.SetClkDiv(SystemCoreClock / 2, frequency);  // a full SPI clock cycle requires 2 instructions
}

#if 0
void TWifiCyw43SpiComm::SpiTransfer(uint32_t * txbuf, uint32_t txwords, uint32_t * rxbuf, uint32_t rxwords)
{
  sm.IrqClear(1); // clear IRQ-0

  sm.regs->instr = pio_encode_set(pio_pindirs, 1); // set pindir=1

  pin_cs.Set0();

  sm.PreloadY((rxwords ? (rxwords << 5) - 1 : 0), 32);
  sm.PreloadX((txwords << 5) - 1, 32);

  *sm.tx_lsb = *txbuf;  // fast load the first word

  sm.Start();

  ++txbuf;
  --txwords;

  while (txwords)  // push the remaining TX data
  {
    if (sm.TrySend32(*txbuf))
    {
      ++txbuf;
      --txwords;
    }
  }

  // get the RX data then
  while (rxwords)
  {
    if (sm.TryRecv32(rxbuf))
    {
      ++rxbuf;
      --rxwords;
    }
  }

  while (0 == (sm.dregs->irq & 1))
  {
    // wait
  }

  sm.Stop();
  pin_cs.Set1();
}
#endif

void TWifiCyw43SpiComm::SpiTransfer(uint32_t cmd, bool istx, uint32_t * buf, uint32_t wordcnt)
{
  sm.IrqClear(1); // clear IRQ-0

  sm.regs->instr = pio_encode_set(pio_pindirs, 1); // set pindir=1

  pin_cs.Set0();

  sm.PreloadX((istx ? ((wordcnt + 1) << 5) - 1 : 31                ), 32); // tx bit count - 1 + 32 bit command
  sm.PreloadY((istx ? 0                        : (wordcnt << 5) - 1), 32); // rx bit count - 1 or 0

  *sm.tx_lsb = cmd;  // load the command

  sm.Start();

  if (istx)
  {
    while (wordcnt)  // push the remaining TX data
    {
      if (sm.TrySend32(*buf))
      {
        ++buf;
        --wordcnt;
      }
    }
  }
  else  // RX
  {
    while (wordcnt)
    {
      if (sm.TryRecv32(buf))
      {
        ++buf;
        --wordcnt;
      }
    }
  }

  while (0 == (sm.dregs->irq & 1))
  {
    // wait
  }

  sm.Stop();
  pin_cs.Set1();
}


bool TWifiCyw43SpiComm::InitBaseComm()
{
  ResetModule();

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

uint32_t TWifiCyw43SpiComm::ReadCmdU32(uint32_t acmd)
{
  SpiTransfer(acmd, false, &rwbuf[0], 1);
  return rwbuf[0];
}

void TWifiCyw43SpiComm::WriteCmdU32(uint32_t acmd, uint32_t avalue)
{
  rwbuf[0] = avalue;
  SpiTransfer(acmd, true, &rwbuf[0],  1);
}

void TWifiCyw43SpiComm::WriteSpiReg(uint32_t addr, uint32_t value, uint32_t len)
{
  uint32_t cmd = (0
    | (len    <<  0)  // write size: 1..4
    | (addr   << 11)  // 15 bit address + 2 bit function
    | (0      << 28)  // 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // 1 = address increment
    | (1      << 31)  // 1 = write
  );
  rwbuf[0] = value;
  SpiTransfer(cmd, true, &rwbuf[0],  1);
}

uint32_t TWifiCyw43SpiComm::ReadSpiReg(uint32_t addr, uint32_t len)
{
  uint32_t cmd = (0
    | (len    <<  0)  // read size: 1..4
    | ((addr & 0x1FFFF) << 11)  // ADDR(17): 17 bit address
    | (0      << 28)  // FUNC(2): 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // INC: 1 = address increment
    | (0      << 31)  // R/W: 0 = read
  );
  SpiTransfer(cmd, false, &rwbuf[0], 1);
  return rwbuf[0] & ((1u << (len * 8))-1);  // mask the result according the requested length
}

void TWifiCyw43SpiComm::WriteBplReg(uint32_t addr, uint32_t value, uint32_t len)
{
  uint32_t cmd = (0
    | (len    <<  0)  // write size: 1..4
    | ((addr & 0x1FFFF) << 11)  // 17 bit address
    | (1      << 28)  // FUNC(2): 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // INC: 1 = address increment
    | (1      << 31)  // R/W: 1 = write
  );
  rwbuf[0] = value;
  SpiTransfer(cmd, true, &rwbuf[0],  1);
}

uint32_t TWifiCyw43SpiComm::ReadBplReg(uint32_t addr, uint32_t len)
{
  uint32_t cmd = (0
    | (len    <<  0)  // read size: 1..4
    | ((addr & 0x1FFFF) << 11)  // ADDR(17): 17 bit address
    | (1      << 28)  // FUNC(2): 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // INC: 1 = address increment
    | (0      << 31)  // R/W: 0 = read
  );
  SpiTransfer(cmd, false, &rwbuf[0], CYWBPL_READ_PAD_WORDS + 1);
  return rwbuf[CYWBPL_READ_PAD_WORDS] & ((1u << (len * 8))-1);
}

void TWifiCyw43SpiComm::SetBackplaneWindow(uint32_t addr)
{
  addr = (addr & 0xFFFF8000);

#if 0 // two step version

  if ((addr & 0xFF000000) != (cur_backplane_window & 0xFF000000)) // this usually never changes
  {
    WriteBplReg(0x1000C, addr >> 24, 1);  // SDIO_BACKPLANE_ADDRESS_HIGH
  }
  if ((addr & 0x00FFFF00) != (cur_backplane_window & 0x00FFFF00))
  {
    WriteBplReg(0x1000A, addr >> 8, 2);   // SDIO_BACKPLANE_ADDRESS_LOW + MID
  }

#else // one step version with 3-byte write

  if ((addr & 0xFFFFFF00) != (cur_backplane_window & 0xFFFFFF00))
  {
    WriteBplReg(0x1000A, addr >> 8, 3);   // SDIO_BACKPLANE_ADDRESS_LOW + MID + HIGH
  }

#endif

  cur_backplane_window = addr;
}

uint32_t TWifiCyw43SpiComm::ReadBplAddr(uint32_t addr, uint32_t len)
{
  SetBackplaneWindow(addr);
  return ReadBplReg(addr | 0x8000, len);  // 0x8000 = 2_4B_FLAG
}

void TWifiCyw43SpiComm::WriteBplAddr(uint32_t addr, uint32_t value, uint32_t len)
{
  SetBackplaneWindow(addr);
  WriteBplReg(addr | 0x8000, value, len);  // 0x8000 = 2_4B_FLAG
}

void TWifiCyw43SpiComm::WriteBplAddrBlock(uint32_t addr, uint8_t * buf, uint32_t bytelen)
{
  SetBackplaneWindow(addr);

  addr = ((addr & 0x7FFF) | 0x8000);

  uint32_t cmd = (0
    | (bytelen    <<  0)  // write size: 1..64
    | ((addr & 0x1FFFF) << 11)  // 17 bit address
    | (1      << 28)  // FUNC(2): 0 = SPI Bus, 1 = BackPlane, 2 = WiFi
    | (1      << 30)  // INC: 1 = address increment
    | (1      << 31)  // R/W: 1 = write
  );
  memcpy(&rwbuf[1], buf, bytelen);
  SpiTransfer(cmd, true, &rwbuf[0],  bytelen >> 2);
}

uint32_t TWifiCyw43SpiComm::ReadArmCoreReg(uint32_t addr, uint32_t len)
{
  addr += CYW_BPL_ADDR_WLAN_ARMCM3 + CYW_BPL_WRAPPER_REG_OFFS;
  SetBackplaneWindow(addr);
  return ReadBplReg(addr | 0x8000, len);  // 0x8000 = 2_4B_FLAG
}

void TWifiCyw43SpiComm::WriteArmCoreReg(uint32_t addr, uint32_t value, uint32_t len)
{
  addr += CYW_BPL_ADDR_WLAN_ARMCM3 + CYW_BPL_WRAPPER_REG_OFFS;
  SetBackplaneWindow(addr);
  WriteBplReg(addr | 0x8000, value, len);  // 0x8000 = 2_4B_FLAG
}


uint32_t TWifiCyw43SpiComm::ReadSocRamReg(uint32_t addr, uint32_t len)
{
  addr += CYW_BPL_ADDR_SOCSRAM + CYW_BPL_WRAPPER_REG_OFFS;
  SetBackplaneWindow(addr);
  return ReadBplReg(addr | 0x8000, len);  // 0x8000 = 2_4B_FLAG
}

void TWifiCyw43SpiComm::WriteSocRamReg(uint32_t addr, uint32_t value, uint32_t len)
{
  addr += CYW_BPL_ADDR_SOCSRAM + CYW_BPL_WRAPPER_REG_OFFS;
  SetBackplaneWindow(addr);
  WriteBplReg(addr | 0x8000, value, len);  // 0x8000 = 2_4B_FLAG
}
