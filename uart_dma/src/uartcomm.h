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
 *  file:     uartcomm.h
 *  brief:    Realistic example for UART communication using DMA
 *
 *            The RX bytes are received with DMA using a circular buffer
 *            These bytes are then collected into the rx message buffer (the messages
 *            might be chunked) for processing
 *            The responses are added to the Tx Buffer which will be sent using DMA
 *
 *            open a terminal to the board UART (baudrate 115200, 8 bit data, 1 stop bit)
 *
 *  version:  1.00
 *  date:     2021-11-06
 *  authors:  nvitya
*/

/* test texts, they have known length and content, must be echoed back properly

1. single line
1234567890123456789012345678901234567890

2. multi line
1234567890
1234567890
1234567890
1234567890

*/

#ifndef SRC_UART_COMM_H_
#define SRC_UART_COMM_H_

#include "stdint.h"
#include "platform.h"
#include "hwuart.h"
#include "hwdma.h"

// you might choose the rx buffer much bigger (like 512), it is relative small here
// to test the circular DMA buffers
#define UARTCOMM_RXBUF_SIZE  128  // circular buffer
#define UARTCOMM_TXBUF_SIZE  256  // one tx response must fit into it, allocated two times

#define UARTCOMM_MAX_RX_MSG_LEN  64  // maximal length of a parsed message

class TUartComm
{
public:
  bool              initialized = false;

  THwUart           uart;
  THwDmaChannel     dma_rx;
  THwDmaChannel     dma_tx;

  THwDmaTransfer    dmaxfer_tx;
  THwDmaTransfer    dmaxfer_rx;

  uint16_t          rxmsglen = 0;
  uint16_t          rxdmapos = 0;

  uint16_t          txbufwr = 0;
  uint16_t          txlen = 0;

  uint8_t           rxmsgbuf[UARTCOMM_MAX_RX_MSG_LEN];  // parsed message buffer

  uint8_t           rxdmabuf[UARTCOMM_RXBUF_SIZE];  // circular buffer, might contain more messages

  uint8_t           txbuf[2][UARTCOMM_TXBUF_SIZE];

  bool              Init();
  void              Run();  // processes Rx and Tx

  unsigned          AddTx(void * asrc, unsigned len); // returns the amount actually written
  void              StartSendTxBuffer();
  inline unsigned   TxAvailable() { return sizeof(txbuf[0]) - txlen; }

public: // Example text protocol
  void              AddTxMessage(const char * fmt, ...);

protected:
  bool              InitHw();  // board specific implementation

};

extern TUartComm g_uartcomm;

#endif /* SRC_UART_COMM_H_ */
