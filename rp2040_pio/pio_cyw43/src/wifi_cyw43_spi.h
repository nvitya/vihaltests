/*
 * wifi_cyw43_spi.h
 *
 *  Created on: Oct 10, 2023
 *      Author: vitya
 */

#ifndef SRC_WIFI_CYW43_SPI_H_
#define SRC_WIFI_CYW43_SPI_H_

#include "wifi_cyw43_spi_comm.h"
#include "spiflash.h"

class TWifiCyw43Spi
{
private:
  typedef TWifiCyw43SpiComm  super;

public:

  bool                 initialized = false;
  TWifiCyw43SpiComm *  pcomm = nullptr;

  TSpiFlash *          pspiflash = nullptr;
  uint32_t             fw_storage_addr = 0x1C0000;
  const char *         fw_file_name = "43439A0.bin";

  bool      Init(TWifiCyw43SpiComm * acomm, TSpiFlash * aspiflash);

  bool      InitBackPlane();

  bool      LoadFirmware();

  bool      ResetDeviceCore(uint32_t abaseaddr);

protected:
  uint8_t *   fwbuf = nullptr;

};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
