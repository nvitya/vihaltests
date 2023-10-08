/*
 * wifi_cyw43_spi.h
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

#ifndef SRC_WIFI_CYW43_SPI_H_
#define SRC_WIFI_CYW43_SPI_H_

#include "pioapp_cyw43_spi.h"

class TWifiCyw43Spi
{
public:
  bool                initialized = false;
  TPioAppCyw43Spi *   pcomm = nullptr;

  bool      Init(TPioAppCyw43Spi * apioapp);

public:
  bool      InitBaseComm();

public:
  uint32_t  ReadU32(uint32_t acmd);
  void      WriteU32(uint32_t acmd, uint32_t avalue);

public:
  uint32_t  txbuf[16];
  uint32_t  rxbuf[16];
};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
