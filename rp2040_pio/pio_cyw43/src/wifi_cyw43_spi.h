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

#define CYW43_CH_IOCTL  0
#define CYW43_CH_EVENT  1
#define CYW43_CH_DATA   2

#define CYW43_ERR_RQ    0xEE01  // invalid request

#define CYW43_IRQ_F2_PACKET (1 << 5)

typedef struct
{
  uint16_t  size;
  uint16_t  size_com;
  uint8_t   sequence;
  uint8_t   channel_and_flags;    // bits 0-3: channel: 0 = IOCTL, 1 = EVENT, 2 = DATA
  uint8_t   next_length;          // not used (?), set to 0 at sending
  uint8_t   header_length;
  uint8_t   wireless_flow_control;
  uint8_t   bus_data_credit;
  uint8_t   reserved[2];
//
} TSdpcmHeader;  // 12 Bytes

// NOTE: At data packets (channel = 2) there are + 2 bytes of padding before and
//       +2 Bytes of padding after the TSdpcmBdcHeader

typedef struct
{
  uint8_t flags;
  uint8_t priority;
  uint8_t flags2;
  uint8_t data_offset;
//
} TSdpcmBdcHeader; // 4 Bytes, but +2 padding before and after might be added


typedef struct
{
  uint32_t  cmd;
  uint16_t  txlen;
  uint16_t  rxlen;
  uint16_t  flags;  // bit15..12: IFACE, bit1-2: KIND
  uint16_t  rq_id;
  int32_t   status;
//
} TIoctlHeader;  // 16 Bytes

#define CYW43_WBUF_SIZE  2048

struct TCyw43Request  // 64 Bytes
{
  bool                 completed;
  uint8_t              channel;
  uint16_t             error;

  TCyw43Request *      next;

  uint8_t *            dataptr;
  unsigned             datalen;

  uint8_t *            ansptr;
  unsigned             anslen;  // must be initialized to max length

  uint16_t             rq_id;   // IOCTL request id
  uint16_t             flags;   // some additional IOCTL flags (IFACE + KIND)
  uint32_t             cmd;     // IOCTL command
};

class TWifiCyw43Spi
{
private:
  typedef TWifiCyw43SpiComm  super;

public:

  bool                 initialized = false;
  uint8_t              rqstate = 0;
  uint8_t              wreadstate = 0;
  bool                 use_irq_pin = false;
  uint16_t             wreadlen = 0;

  uint16_t             irq_status = 0;
  uint16_t             irq_mask = 0;
  uint32_t             gspi_status = 0;

  TWifiCyw43SpiComm *  pcomm = nullptr;
  TCyw43Request *      currq = nullptr;

  uint8_t              sdpcm_tx_seq_num = 0;
  uint8_t              wlan_flow_control = 0;
  uint16_t             ioctl_rq_id = 0;

  TSpiFlash *          pspiflash = nullptr;
  uint32_t             fw_storage_addr = 0x1C0000;
  const char *         fw_file_name = "43439A0.bin";

  uint32_t             errcnt_invalid_rxpkt = 0;

  bool        Init(TWifiCyw43SpiComm * acomm, TSpiFlash * aspiflash);
  void        Run(); // process requests, events, and data packets

  bool        InitBackPlane();
  bool        LoadFirmware();
  bool        PrepareBus();

  bool        ResetDeviceCore(uint32_t abaseaddr);

  void        ExecIoctl();

  bool        GpioSetTo(uint32_t gpio_num, uint8_t avalue);
  bool        WriteIoVar(const char * iovar_name, void * params, uint32_t parlen);

  bool        AddRequest(TCyw43Request * arq);
  bool        SendRequest(TCyw43Request * arq);

  void        ProcessRxPacket();

protected:
  void        LoadFirmwareDataFromNvs(uint32_t abpladdr, uint32_t anvsaddr, uint32_t len);
  void        LoadFirmwareDataFromRam(uint32_t abpladdr, const void * srcbuf, uint32_t len);

public:
  TCyw43Request  mrq;                    // internal requests
  uint8_t        mrqdata[256];           // smaller parameter storage for the internal requests
  uint32_t       spiregs[2];
  uint8_t        wbuf[CYW43_WBUF_SIZE];  // local buffer for message assembly and data exchange
};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
