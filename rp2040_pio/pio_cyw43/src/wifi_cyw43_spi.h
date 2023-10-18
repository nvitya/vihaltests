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

typedef struct
{
  uint16_t  size;
  uint16_t  size_com;
  uint8_t   sequence;
  uint8_t   channel_and_flags;
  uint8_t   next_length;
  uint8_t   header_length;
  uint8_t   wireless_flow_control;
  uint8_t   bus_data_credit;
  uint8_t   reserved[2];
//
} TSdpcmHeader;  // 12 bytes

typedef struct
{
  uint32_t  cmd;
  uint32_t  len;    // lower 16 is output len; upper 16 is input len
  uint32_t  flags;
  uint32_t  status;
//
} TIoctlHeader;  // 16 bytes


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
  bool      PrepareBus();

  bool      ResetDeviceCore(uint32_t abaseaddr);

protected:
  uint8_t *   fwbuf = nullptr;

  void        LoadFirmwareDataFromNvs(uint32_t abpladdr, uint32_t anvsaddr, uint32_t len);
  void        LoadFirmwareDataFromRam(uint32_t abpladdr, const void * srcbuf, uint32_t len);

};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
