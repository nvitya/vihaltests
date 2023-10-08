/*
 * wifi_cyw43_spi.h
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

#ifndef SRC_WIFI_CYW43_SPI_H_
#define SRC_WIFI_CYW43_SPI_H_

#include "pioapp_cyw43_spi.h"

// these can be or-ed to the addresses to select the address range:
#define CYWFN_BUS   (0 << 17)   // SPI BUS
#define CYWFN_BKPL  (1 << 17)   // BACKPLANE
#define CYWFN_WLAN  (2 << 17)   // WLAN

#define CYWBPL_READ_PAD_WORDS              4

#define CYWIRQ_DATA_UNAVAILABLE                0x0001  // Requested data not available; Clear by writing a "1"
#define CYWIRQ_F2_F3_FIFO_RD_UNDERFLOW         0x0002
#define CYWIRQ_F2_F3_FIFO_WR_OVERFLOW          0x0004
#define CYWIRQ_COMMAND_ERROR                   0x0008  // Cleared by writing 1
#define CYWIRQ_DATA_ERROR                      0x0010  // Cleared by writing 1
#define CYWIRQ_F2_PACKET_AVAILABLE             0x0020
#define CYWIRQ_F3_PACKET_AVAILABLE             0x0040
#define CYWIRQ_F1_OVERFLOW                     0x0080  // Due to last write. Bkplane has pending write requests
#define CYWIRQ_GSPI_PACKET_AVAILABLE           0x0100
#define CYWIRQ_MISC_INTR1                      0x0200
#define CYWIRQ_MISC_INTR2                      0x0400
#define CYWIRQ_MISC_INTR3                      0x0800
#define CYWIRQ_MISC_INTR4                      0x1000
#define CYWIRQ_F1_INTR                         0x2000
#define CYWIRQ_F2_INTR                         0x4000
#define CYWIRQ_F3_INTR                         0x8000

class TWifiCyw43Spi
{
public:
  bool                initialized = false;
  TPioAppCyw43Spi *   pcomm = nullptr;

  bool      Init(TPioAppCyw43Spi * apioapp);

public:
  bool      InitBaseComm();
  bool      InitBackPlane();

public:
  uint32_t  ReadCmdU32(uint32_t acmd);
  void      WriteCmdU32(uint32_t acmd, uint32_t avalue);

  void      WriteSpiReg(uint32_t addr, uint32_t value, uint32_t len);
  uint32_t  ReadSpiReg(uint32_t addr, uint32_t len);

  void      WriteBplReg(uint32_t addr, uint32_t value, uint32_t len);
  uint32_t  ReadBplReg(uint32_t addr, uint32_t len);

public:
  uint32_t  txbuf[16];
  uint32_t  rxbuf[16];
};

#endif /* SRC_WIFI_CYW43_SPI_H_ */
