/*
 * pioapp_spi.h
 *
 *  Created on: Aug 14, 2023
 *      Author: vitya
 */

#ifndef SRC_PIOAPP_SPI_H_
#define SRC_PIOAPP_SPI_H_

#include "hwrppio.h"

class TPioAppSpi : public THwRpPioApp
{
public:
  unsigned      frequency = 1000000;  // 1 MHz by default
  uint8_t       databits = 8;
  uint8_t       pin_sck  = 0xFF;
  uint8_t       pin_mosi = 0xFF;  // 0xFF = SCK + 1
  uint8_t       pin_miso = 0xFF;  // 0xFF = SCK + 2

  uint32_t      smbit = 1;

public:
  THwRpPioSm    sm;
  THwRpPioPrg   prg;

  bool          Init(uint8_t adevnum, uint8_t asmnum, uint8_t apin_sck = 0xFF);
  void          SetFrequency(unsigned afreq);
  void          Start();

  inline bool   TrySend8(uint8_t   adata) { return sm.TrySendMsb8(adata); }
  inline bool   TryRecv8(uint8_t * adata) { return sm.TryRecvMsb8(adata); }
};

#endif /* SRC_PIOAPP_SPI_H_ */
