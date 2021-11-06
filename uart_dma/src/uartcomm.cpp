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
 *  version:  1.00
 *  date:     2021-11-06
 *  authors:  nvitya
*/


#include "hwpins.h"
#include "uartcomm.h"
#include "mp_printf.h"


TUartComm g_uartcomm;

#if 0

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

bool TUartComm::InitHw()
{
  // USART3: Stlink USB / Serial converter
  hwpinctrl.PinSetup(PORTNUM_D, 8,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART3_TX: PD.8
  hwpinctrl.PinSetup(PORTNUM_D, 9,  PINCFG_INPUT  | PINCFG_AF_7);  // USART3_RX: Pd.9
  uart.Init(3); // USART3

  dma_tx.Init(1, 3, 4);  // dma1, stream3, ch4 = USART3_TX
  dma_rx.Init(1, 1, 4);  // dma1, stream1, ch4 = USART3_RX

  return true;
}

#else
  #error "Define board specific uart comm init here"
#endif

bool TUartComm::Init()
{
  initialized = false;

  if (!InitHw())
  {
    return false;
  }

  uart.DmaAssign(true,  &dma_tx);
  uart.DmaAssign(false, &dma_rx);

  // start the DMA receive with circular DMA buffer
  rxdmapos = 0;
  dmaxfer_rx.bytewidth = 1;
  dmaxfer_rx.count = sizeof(rxdmabuf);
  dmaxfer_rx.dstaddr = &rxdmabuf[0];
  dmaxfer_rx.flags = DMATR_CIRCULAR;
  uart.DmaStartRecv(&dmaxfer_rx);

  rxmsglen = 0;
  txlen = 0;

  // Send some wellcome message
  AddTxMessage("\r\n-------------------------------\r\n");
  AddTxMessage("VIHAL UART DMA DEMO\r\n");
  AddTxMessage("Enter some text (not echoed as you type) and press enter!\r\n");
  StartSendTxBuffer(); // send it with DMA now

  initialized = true;
  return true;
}

unsigned TUartComm::AddTx(void * asrc, unsigned len) // returns the amount actually written
{
  unsigned available = TxAvailable();
  if (0 == available)
  {
    return 0;
  }

  if (len > available)  len = available;

  uint8_t * srcp = (uint8_t *)asrc;
  uint8_t * dstp = &txbuf[txbufwr][txlen];
  uint8_t * endp = dstp + len;
  while (dstp < endp)
  {
    *dstp++ = *srcp++;
  }

  txlen += len;

  return len;
}

void TUartComm::AddTxMessage(const char * fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);

  // allocate format buffer on the stack:
  char fmtbuf[FMT_BUFFER_SIZE];

  char * pch = &fmtbuf[0];
  *pch = 0;

  int len = mp_vsnprintf(pch, FMT_BUFFER_SIZE, fmt, arglist);

  if (len > 0)
  {
    AddTx(&fmtbuf[0], len);
  }

  va_end(arglist);
}

void TUartComm::StartSendTxBuffer()
{
  if (txlen && !dma_tx.Active())
  {
    // setup the TX DMA and flip the buffer

    dmaxfer_tx.flags = 0;
    dmaxfer_tx.bytewidth = 1;
    dmaxfer_tx.count = txlen;
    dmaxfer_tx.srcaddr = &txbuf[txbufwr][0];

    uart.DmaStartSend(&dmaxfer_tx);

    // change the buffer
    txbufwr ^= 1;
    txlen = 0;
  }
}

void TUartComm::Run()
{
  // RX processing
  uint16_t newrxdmapos = sizeof(rxdmabuf) - dma_rx.Remaining();

  while (rxdmapos != newrxdmapos)
  {
    uint8_t b = rxdmabuf[rxdmapos];
    if (13 == b)
    {
      AddTxMessage("You entered: \"");
      AddTx(&rxmsgbuf[0], rxmsglen);
      AddTxMessage("\"\r\n");

      AddTxMessage("[RX DMA POS=%u]\r\n", rxdmapos);

      rxmsglen = 0;
    }
    else if (8 == b) // backspace
    {
      if (rxmsglen > 0)  --rxmsglen;
    }
    else if (b >= 32) // add to the rx message length
    {
      if (rxmsglen < sizeof(rxmsgbuf))
      {
        rxmsgbuf[rxmsglen] = b;
        ++rxmsglen;
      }
    }

    ++rxdmapos;
    if (rxdmapos >= sizeof(rxdmabuf))  rxdmapos = 0;
  }


  // Sending buffered tx messages
  StartSendTxBuffer();
}
