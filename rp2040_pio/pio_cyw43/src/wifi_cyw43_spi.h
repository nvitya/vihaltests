/*
 * wifi_cyw43_spi.h
 *
 *  Created on: Oct 10, 2023
 *      Author: vitya
 */

#ifndef SRC_WIFI_CYW43_SPI_H_
#define SRC_WIFI_CYW43_SPI_H_

#include "wifi_cyw43_spi_comm.h"

class TWifiCyw43Spi
{
private:
  typedef TWifiCyw43SpiComm  super;

public:

  bool                 initialized = false;
  TWifiCyw43SpiComm *  pcomm = nullptr;

  bool      Init(TWifiCyw43SpiComm * acomm);

  bool      InitBackPlane();

  bool      ResetDeviceCore(uint32_t abaseaddr);

};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
