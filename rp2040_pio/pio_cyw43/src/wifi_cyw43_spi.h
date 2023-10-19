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

#define CYW43_WBUF_SIZE  2048

class TWifiCyw43Spi
{
private:
  typedef TWifiCyw43SpiComm  super;

public:

  bool                 initialized = false;
  TWifiCyw43SpiComm *  pcomm = nullptr;

  uint8_t              sdpcm_tx_seq_num = 0;
  uint16_t             sdpcm_ioctl_rq_id = 0;

  TSpiFlash *          pspiflash = nullptr;
  uint32_t             fw_storage_addr = 0x1C0000;
  const char *         fw_file_name = "43439A0.bin";

  bool        Init(TWifiCyw43SpiComm * acomm, TSpiFlash * aspiflash);

  bool        InitBackPlane();
  bool        LoadFirmware();
  bool        PrepareBus();

  bool        ResetDeviceCore(uint32_t abaseaddr);

  void        ExecIoctl();

  bool        GpioSetTo(uint32_t gpio_num, uint8_t avalue);
  bool        WriteIoVar(const char * iovar_name, void * params, uint32_t parlen);

protected:
  void        LoadFirmwareDataFromNvs(uint32_t abpladdr, uint32_t anvsaddr, uint32_t len);
  void        LoadFirmwareDataFromRam(uint32_t abpladdr, const void * srcbuf, uint32_t len);

public:
  uint8_t     wbuf[CYW43_WBUF_SIZE];  // local buffer for message assembly and data exchange
};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
