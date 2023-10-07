/*
 * pioapp_spi.cpp
 *
 *  Created on: Aug 14, 2023
 *      Author: vitya
 */

#include <pioapp_cyw43_spi.h>
#include "clockcnt.h"
#include "hwpins.h"

#define BUS_FUNCTION        (0)
#define BACKPLANE_FUNCTION  (1)
#define WLAN_FUNCTION       (2)

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

bool TPioAppCyw43Spi::Init(uint8_t adevnum, uint8_t asmnum, uint8_t apin_sck)
{
  initialized = false;

  if (0xFF != apin_sck)  pin_sck  = apin_sck;
  if (pin_data > 31)     pin_data = pin_sck + 1;

  //pin_data = 16;

  if (!pin_cs.Assigned())    pin_cs.Assign(0, 25, false);
  if (!pin_wlon.Assigned())  pin_wlon.Assign(0, 23, false);

  pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);

  // the reset must be done with data pin low, otherwise the communication won't work,
  // (probably SDIO mode will be selected ?)
  hwpinctrl.PinSetup(0,  pin_data, PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);  // this will be later overridden

  pin_wlon.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  delay_ms(20);
  pin_wlon.Set1();
  delay_ms(50);

  prg.Init(adevnum, prgoffset);  // offset=0, entry=offset

  // Pin assignments:
  //  - SCK is side-set pin 0
  //  - MOSI/MISO is OUT/IN pin 0
  // x: preloaded to output bit count - 1
  // y: preloaded to input bit count - 1

  prg.Add(0x6001); //  0: out    pins, 1         side 0
  prg.Add(0x1040); //  1: jmp    x--, 0          side 1
  prg.Add(0xe080); //  2: set    pindirs, 0      side 0
  prg.Add(0xb042); //  3: nop                    side 1
  prg.Add(0xa042); //  4: nop                    side 0
  prg.Add(0x5001); //  5: in     pins, 1         side 1
  prg.Add(0x0085); //  6: jmp    y--, 5          side 0

  if (!sm.Init(adevnum, asmnum))
  {
    return false;
  }

  smbit = (1 << asmnum);

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

  initialized = true;
  return true;
}

void TPioAppCyw43Spi::SetFrequency(unsigned afreq)
{
  frequency = afreq;
  sm.SetClkDiv(SystemCoreClock / 2, frequency);  // a full SPI clock cycle requires 2 instructions
}

void TPioAppCyw43Spi::Start()
{
  sm.Start();
}

void TPioAppCyw43Spi::StartTransfer(uint32_t * txbuf, uint32_t txwords, uint32_t * rxbuf, uint32_t rxwords)
{
  uint32_t txremaining = txwords;
  uint32_t rxremaining = rxwords;

  sm.SetPinDir(pin_data, 1);

  pin_cs.Set0();
  sm.PreloadY((rxwords << 5) - 1, 32);
  sm.PreloadX((txwords << 5) - 1, 32);

  sm.Start();

  // push the TX data first
  while (txremaining)
  {
    if (sm.TrySend32(*txbuf))
    {
      ++txbuf;
      --txremaining;
    }
  }

  // get the RX data then
  while (rxremaining)
  {
    if (sm.TryRecv32(rxbuf))
    {
      ++rxbuf;
      --rxremaining;
    }
  }

  sm.Stop();
  pin_cs.Set1();
}
