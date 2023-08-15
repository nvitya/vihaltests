/*
 * pioapp_wave.cpp
 *
 *  Created on: Aug 14, 2023
 *      Author: vitya
 */

#include <pioapp_wave.h>

bool TPioAppWave::Init(uint8_t adevnum, uint8_t asmnum, uint8_t apinnum)
{
  initialized = false;

  prg.Init(adevnum, prgoffset);  // offset=0, entry=offset
  prg.Add(0xe001); //  0: set    pins, 1    [0]
  prg.Add(0xe000); //  1: set    pins, 0    [0]
  // wrap is configured automatically between the first and the last line of code
  // but it can be overridden with prg.wrap_start and prg.wrap_end

  if (!sm.Init(adevnum, asmnum))
  {
    return false;
  }

  sm.SetPrg(&prg);

  sm.SetupPinsSet(apinnum, 1);
  sm.SetPinDir(apinnum, 1);

  //sm.SetupPinsSideSet(apinnum, 1);  // also updates the sideset_len

  SetFrequency(frequency);

  initialized = true;
  return true;
}

void TPioAppWave::SetFrequency(unsigned afreq)
{
  frequency = afreq;
  sm.SetClkDiv(SystemCoreClock / 2, frequency);  // clk/2: every edge change requires two clocks
}

void TPioAppWave::Start()
{
  sm.Start();
}
