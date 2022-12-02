/* -----------------------------------------------------------------------------
 * This file is a part of the VIHAL TEST project: https://github.com/nvitya/vihaltests
 * Copyright (c) 2021 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     espwifi_uart.h
 *  brief:    UART (AT) WiFi Driver for Espressif Modules
 *  date:     2022-11-27
 *  authors:  nvitya
*/

#ifndef SRC_ESPWIFI_UART_H_
#define SRC_ESPWIFI_UART_H_

#include "stdint.h"
#include "platform.h"
#include "hwuart.h"
#include "hwdma.h"
#include "hwpins.h"
#include "net_ip4.h"

#define UARTCOMM_RXBUF_SIZE       256  // circular buffer
#define UARTCOMM_TXBUF_SIZE       256  // one tx response must fit into it, allocated two times

#define UARTCOMM_MAX_RX_MSG_LEN  2048  // maximal length of a parsed message

class TEspAtUdpSocket;

class TEspWifiUart
{
protected:
  uint8_t             state = 0;

  uint16_t            rxmsglen = 0;
  uint16_t            rxdmapos = 0;

  uint16_t            txbufwr = 0;
  uint16_t            txlen = 0;

public:
  bool                initialized = false;
  bool                link_up = false;

  THwUart             uart;
  THwDmaChannel       dma_rx;
  THwDmaChannel       dma_tx;
  TGpioPin            pin_rst;
  TGpioPin            pin_en;

  THwDmaTransfer      dmaxfer_tx;
  THwDmaTransfer      dmaxfer_rx;

  bool                Init();
  void                Run();  // processes Rx and Tx
  void                RunRx();

public:
  TIp4Addr            ipaddress;
  TIp4Addr            netmask;
  TIp4Addr            gwaddress;
  TIp4Addr            dns;
  TIp4Addr            dns2;

  const char *        ssid = "SSID";
  const char *        password = "PASSWORD";

  TEspAtUdpSocket *   udp_first = nullptr;
  TEspAtUdpSocket *   udp_last  = nullptr;


  void                AddUdpSocket(TEspAtUdpSocket * audp);

  bool                IsLinkUp() { return link_up; }

  void                StartCommand(const char * fmt, ...);

  unsigned            AddTx(void * asrc, unsigned len); // returns the amount actually written
  void                StartSendTxBuffer();
  inline unsigned     TxAvailable() { return sizeof(txbuf[0]) - txlen; }

public:
  void                AddTxMessage(const char * fmt, ...);



protected:
  const char *        msg_ready = "\r\nready\r\n"; // the text for ready
  const char *        msg_ok    = "\r\nOK\r\n"; // the text for ready
  const char *        msg_error = "\r\nERROR\r\n"; // the text for ready


  bool                InitHw();  // board specific implementation

  bool                MsgOkDetected();
  bool                MsgErrorDetected();

protected: // these big buffers must come to the last

  uint8_t             rxmsgbuf[UARTCOMM_MAX_RX_MSG_LEN];  // parsed message buffer
  uint8_t             rxdmabuf[UARTCOMM_RXBUF_SIZE];  // circular buffer, might contain more messages
  uint8_t             txbuf[2][UARTCOMM_TXBUF_SIZE];

};

class TEspAtUdpSocket
{
public:
public:
  TIp4Addr          destaddr;
  uint16_t          destport = 0;

  TIp4Addr          srcaddr;
  uint16_t          srcport = 0;

  uint16_t          listenport = 0;

  uint16_t          idcounter = 0;

  TEspWifiUart *    pwifim = nullptr;

  TPacketMem *      rxpkt_first = nullptr;
  TPacketMem *      rxpkt_last  = nullptr;

  TPacketMem *      txpkt_first = nullptr;  // waiting for address resolution
  TPacketMem *      txpkt_last  = nullptr;

  TEspAtUdpSocket *     nextsocket = nullptr;

  void Init(TEspWifiUart * awifim, uint16_t alistenport);

  int Send(void * adataptr, unsigned adatalen);
  int Receive(void * adataptr, unsigned adatalen);
};

#endif /* SRC_ESPWIFI_UART_H_ */
