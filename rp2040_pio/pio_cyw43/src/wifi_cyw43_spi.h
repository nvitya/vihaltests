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

#define CYW43_COUNTRY_WORLDWIDE         "XX"

#define CYW43_COUNTRY_AUSTRALIA         "AU"
#define CYW43_COUNTRY_AUSTRIA           "AT"
#define CYW43_COUNTRY_BELGIUM           "BE"
#define CYW43_COUNTRY_BRAZIL            "BR"
#define CYW43_COUNTRY_CANADA            "CA"
#define CYW43_COUNTRY_CHILE             "CL"
#define CYW43_COUNTRY_CHINA             "CN"
#define CYW43_COUNTRY_COLOMBIA          "CO"
#define CYW43_COUNTRY_CZECH_REPUBLIC    "CZ"
#define CYW43_COUNTRY_DENMARK           "DK"
#define CYW43_COUNTRY_ESTONIA           "EE"
#define CYW43_COUNTRY_FINLAND           "FI"
#define CYW43_COUNTRY_FRANCE            "FR"
#define CYW43_COUNTRY_GERMANY           "DE"
#define CYW43_COUNTRY_GREECE            "GR"
#define CYW43_COUNTRY_HONG_KONG         "HK"
#define CYW43_COUNTRY_HUNGARY           "HU"
#define CYW43_COUNTRY_ICELAND           "IS"
#define CYW43_COUNTRY_INDIA             "IN"
#define CYW43_COUNTRY_ISRAEL            "IL"
#define CYW43_COUNTRY_ITALY             "IT"
#define CYW43_COUNTRY_JAPAN             "JP"
#define CYW43_COUNTRY_KENYA             "KE"
#define CYW43_COUNTRY_LATVIA            "LV"
#define CYW43_COUNTRY_LIECHTENSTEIN     "LI"
#define CYW43_COUNTRY_LITHUANIA         "LT"
#define CYW43_COUNTRY_LUXEMBOURG        "LU"
#define CYW43_COUNTRY_MALAYSIA          "MY"
#define CYW43_COUNTRY_MALTA             "MT"
#define CYW43_COUNTRY_MEXICO            "MX"
#define CYW43_COUNTRY_NETHERLANDS       "NL"
#define CYW43_COUNTRY_NEW_ZEALAND       "NZ"
#define CYW43_COUNTRY_NIGERIA           "NG"
#define CYW43_COUNTRY_NORWAY            "NO"
#define CYW43_COUNTRY_PERU              "PE"
#define CYW43_COUNTRY_PHILIPPINES       "PH"
#define CYW43_COUNTRY_POLAND            "PL"
#define CYW43_COUNTRY_PORTUGAL          "PT"
#define CYW43_COUNTRY_SINGAPORE         "SG"
#define CYW43_COUNTRY_SLOVAKIA          "SK"
#define CYW43_COUNTRY_SLOVENIA          "SI"
#define CYW43_COUNTRY_SOUTH_AFRICA      "ZA"
#define CYW43_COUNTRY_SOUTH_KOREA       "KR"
#define CYW43_COUNTRY_SPAIN             "ES"
#define CYW43_COUNTRY_SWEDEN            "SE"
#define CYW43_COUNTRY_SWITZERLAND       "CH"
#define CYW43_COUNTRY_TAIWAN            "TW"
#define CYW43_COUNTRY_THAILAND          "TH"
#define CYW43_COUNTRY_TURKEY            "TR"
#define CYW43_COUNTRY_UK                "GB"
#define CYW43_COUNTRY_USA               "US"


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
  uint8_t data_offset;  // in words (?)
//
} TBdcHeader; // 4 Bytes, but +2 padding before and after might be added


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

  uint8_t              macaddr[6];

  TSpiFlash *          pspiflash = nullptr;

  uint32_t             fw_storage_addr = 0x1C0000;
  const char *         country_code    = CYW43_COUNTRY_WORLDWIDE;  // might some channels not available
  const char *         fw_file_name    = "43439A0.bin";
  const char *         clm_file_name   = "43439A0_clm.bin";

  uint32_t             errcnt_invalid_rxpkt = 0;
  uint32_t             errcnt_irq_status = 0;

  bool        Init(TWifiCyw43SpiComm * acomm, TSpiFlash * aspiflash);
  void        Run(); // process requests, events, and data packets

  bool        InitBackPlane();
  bool        LoadFirmware();
  bool        PrepareBus();

  bool        ResetDeviceCore(uint32_t abaseaddr);

  bool        GpioSetTo(uint32_t gpio_num, uint8_t avalue);
  bool        WriteIoVar(const char * iovar_name, void * params, uint32_t parlen);
  bool        ReadIoVar(const char * iovar_name, void * dstbuf, uint32_t len);
  bool        IoctlSet(uint32_t acmd, void * params, uint32_t parlen);

  bool        WifiOn();

  bool        WriteIoVarU32(const char * iovar_name, uint32_t avalue);

  bool        AddRequest(TCyw43Request * arq);
  bool        SendRequest(TCyw43Request * arq);

  void        ProcessRxPacket();

protected:
  void        LoadFirmwareDataFromNvs(uint32_t abpladdr, uint32_t anvsaddr, uint32_t len);
  void        LoadFirmwareDataFromRam(uint32_t abpladdr, const void * srcbuf, uint32_t len);
  bool        LoadClmData();
  bool        FindVrofsFile(const char * afname, uint32_t * rnvsaddr, uint32_t * rlen);

public:
  TCyw43Request  mrq;                    // internal requests
  uint8_t        mrqdata[256];           // smaller parameter storage for the internal requests
  uint32_t       spiregs[2];
  // local buffer for message assembly and data exchange:
  uint8_t        wbuf[CYW43_WBUF_SIZE] __attribute__((aligned(4)));
};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
