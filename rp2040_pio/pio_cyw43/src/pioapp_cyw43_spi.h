/*
 * pioapp_spi.h
 *
 *  Created on: Aug 14, 2023
 *      Author: vitya
 */

#ifndef SRC_PIOAPP_CYW43_SPI_H_
#define SRC_PIOAPP_CYW43_SPI_H_

#include "hwpins.h"
#include "hwrppio.h"

class TPioAppCyw43Spi : public THwRpPioApp
{
public:
  unsigned      frequency = 4000000;  // 4 MHz by default

  uint8_t       pin_sck  = 29;   // PICO-W default pin
  uint8_t       pin_data = 24;   // PICO-W default pin
  //uint8_t       pin_cs   = 25;   // PICO-W default pin, software controlled

  uint32_t      smbit = 1;

  TGpioPin      pin_cs;    // GPIO-25 on PICO-W
  TGpioPin      pin_wlon;  // GPIO-23 on PICO-W

public:
  THwRpPioSm    sm;
  THwRpPioPrg   prg;

  bool          Init(uint8_t adevnum, uint8_t asmnum, uint8_t apin_sck = 0xFF);
  void          SetFrequency(unsigned afreq);
  void          Start();

  void          StartTransfer(uint32_t * txbuf, uint32_t txwords, uint32_t * rxbuf, uint32_t rxwords);

  inline bool   TrySend8(uint8_t   adata) { return sm.TrySendMsb8(adata); }
  inline bool   TryRecv8(uint8_t * adata) { return sm.TryRecvMsb8(adata); }
};

#endif /* SRC_PIOAPP_SPI_H_ */
