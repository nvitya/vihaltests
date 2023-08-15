/*
 * pioapp_spi.cpp
 *
 *  Created on: Aug 14, 2023
 *      Author: vitya
 */

#include <pioapp_spi.h>

bool TPioAppSpi::Init(uint8_t adevnum, uint8_t asmnum, uint8_t apin_sck)
{
  initialized = false;

  if (0xFF != apin_sck)  pin_sck  = apin_sck;
  if (pin_mosi > 31)     pin_mosi = pin_sck + 1;
  if (pin_miso > 31)     pin_miso = pin_sck + 2;

  prg.Init(adevnum, prgoffset);  // offset=0, entry=offset

  // program for CPHA0
  // Pin assignments:
  //  - SCK is side-set pin 0
  //  - MOSI is OUT pin 0
  //  - MISO is IN pin 0
  prg.Add(0x6101); //  out    pins, 1         side 0 [1]
  prg.Add(0x5101); //  in     pins, 1         side 1 [1]
  // wrap is configured automatically between the first and the last line of code
  // but it can be overridden with prg.wrap_start and prg.wrap_end

  if (!sm.Init(adevnum, asmnum))
  {
    return false;
  }

  smbit = (1 << asmnum);

  sm.SetPrg(&prg);

  sm.SetPinDir(pin_sck,  1);
  sm.SetPinDir(pin_mosi, 1);
  sm.SetPinDir(pin_miso, 0);

  sm.SetupPinsOut(pin_mosi, 1);
  sm.SetupPinsIn(pin_miso, 1);
  sm.SetupPinsSideSet(pin_sck,  1);

  sm.SetOutShift(false, true, databits);
  sm.SetInShift( false, true, databits);

  SetFrequency(frequency);

  initialized = true;
  return true;
}

void TPioAppSpi::SetFrequency(unsigned afreq)
{
  frequency = afreq;
  sm.SetClkDiv(SystemCoreClock / 4, frequency);  // a full SPI clock cycle requires 4 instructions
}

void TPioAppSpi::Start()
{
  sm.Start();
}
