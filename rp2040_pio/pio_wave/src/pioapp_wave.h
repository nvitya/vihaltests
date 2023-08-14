/*
 * pioapp_wave.h
 *
 *  Created on: Aug 14, 2023
 *      Author: vitya
 */

#ifndef SRC_PIOAPP_WAVE_H_
#define SRC_PIOAPP_WAVE_H_

#include "hwrppio.h"

class TPioAppWave : public THwRpPioApp
{
public:
  unsigned      frequency = 1000000;  // 1 MHz by default

public:
  THwRpPioSm    sm;
  THwRpPioPrg   prg;

  bool          Init(uint8_t adevnum, uint8_t asmnum, uint8_t apinnum);
  void          SetFrequency(unsigned afreq);
  void          Start();
};

extern TPioAppWave  piowave;

#endif /* SRC_PIOAPP_WAVE_H_ */
