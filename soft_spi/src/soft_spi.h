/*
 * soft_spi.h
 *
 *  Created on: Dec 2, 2023
 *      Author: vitya
 */

#ifndef SRC_SOFT_SPI_H_
#define SRC_SOFT_SPI_H_

#include "platform.h"
#include "hwpins.h"
#include "hwspi.h"

#ifndef SOFT_SPI_MAX_CS
  #define SOFT_SPI_MAX_CS  4
#endif

class TSoftSpi
{
public:
  TGpioPin *     pin_clk  = nullptr;
  TGpioPin *     pin_mosi = nullptr;
  TGpioPin *     pin_miso = nullptr;
  TGpioPin *     pin_cs[SOFT_SPI_MAX_CS] = {0};

  bool           initialized = false;

public:
  uint8_t        cs_number = 0;          // selects the CS line
  bool           bigendian_addr = true;  // the "address" block will be sent in big-endian byte order
  unsigned       speed = 1000000;        // default speed = 1MHz
  uint8_t        databits = 8;           // frame length
  bool           lsb_first = false;
  bool           idleclk_high = false;
  bool           datasample_late = false;

  uint32_t       data_cmd = 0;
  uint32_t       data_addr = 0;
  uint32_t       data_extra = 0;

  uint8_t        default_cmd_len = 1;
  uint8_t        default_addr_len = 3;
  uint8_t        default_extra_len = 1;

  uint8_t        header_rx[16];

public:
  bool           Init();
  void           Transfer(uint32_t acmd, uint32_t aaddr, uint32_t aflags,
                          uint32_t alen, uint8_t * asrc, uint8_t * adst);

  void           BeginTransfer();
  void           EndTransfer();

  void           SendAndRecv(uint32_t alen, uint8_t * asrc, uint8_t * adst);

protected:
  unsigned       half_clock_ticks = 0;
  uint8_t *      rxdata = nullptr;
  uint8_t *      txdata = nullptr;
  uint32_t       data_remaining = 0;
  uint8_t        hcnt = 0;
  uint8_t        hidx = 0;
  uint8_t        header[16];

};

#endif /* SRC_SOFT_SPI_H_ */
