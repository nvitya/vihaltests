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
 *  file:     espwifi_uart.cpp
 *  brief:    UART (AT) WiFi Driver for Espressif Modules
 *  date:     2022-11-27
 *  authors:  nvitya
*/

#include "string.h"
#include "hwpins.h"
#include "espwifi_uart.h"
#include "clockcnt.h"
#include "mp_printf.h"
#include "traces.h"

// the TEspWifiUart::InitHw() are moved to the board_pins.cpp

bool TEspWifiUart::Init()
{
  initialized = false;

  udp_first = nullptr;
  udp_last  = nullptr;

  if (!InitHw()) // must be provided externally
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

  if (pin_en.Assigned())
  {
    pin_en.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  }

  if (pin_rst.Assigned())
  {
    pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
    delay_us(100);
    pin_rst.Set1();
  }

  delay_us(100);

  rxmsglen = 0;
  txlen = 0;
  state = 0;

  initialized = true;
  return true;
}

unsigned TEspWifiUart::AddTx(void * asrc, unsigned len) // returns the amount actually written
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

void TEspWifiUart::StartCommand(const char *fmt, ...)
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

  state = 10; // start a command
}

void TEspWifiUart::AddTxMessage(const char * fmt, ...)
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

void TEspWifiUart::StartSendTxBuffer()
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

void TEspWifiUart::RunRx()
{
  // RX processing
  uint16_t newrxdmapos = sizeof(rxdmabuf) - dma_rx.Remaining();
  if (newrxdmapos >= sizeof(rxdmabuf))
  {
    // fix for Remaining() == 0 (sometimes the linked list operation is processed only later)
    newrxdmapos = 0;
  }

  while (rxdmapos != newrxdmapos)
  {
    uint8_t b = rxdmabuf[rxdmapos];

    //TRACE("%c", b);

    if (0 == state) // waiting for the ready message
    {
      rxmsgbuf[rxmsglen] = b;
      if (b == msg_ready[rxmsglen])
      {
        ++rxmsglen;
        if (strlen(msg_ready) == rxmsglen)
        {
          TRACE("WiFi Ready detected !\r\n");

          //AddTxMessage("AT\r\n");
          //AddTxMessage("AT+CWQAP\r\n");  // do not connect to an access point

          rxmsglen = 0;
          state = 1;

          StartCommand("AT+CWQAP\r\n");  // do not connect to the stored access point
        }
      }
      else
      {
        // reset the search
        rxmsgbuf[0] = b;
        rxmsglen = 1;
      }
    }
    else if (10 == state)  // running a command
    {
      // collect the response until OK or ERROR is detected

      if (rxmsglen < sizeof(rxmsgbuf))
      {
        rxmsgbuf[rxmsglen] = b;
        ++rxmsglen;

        if (MsgOkDetected())
        {
          TRACE("CMD OK detected.\r\n");
          rxmsglen = 0;
          state = 1;
        }
        else if (MsgErrorDetected())
        {
          TRACE("CMD ERROR detected.\r\n");
          rxmsglen = 0;
          state = 1;
        }
      }
      else
      {
        TRACE("WiFi RX buffer overflow\r\n");

        state = 1; // go to idle
        rxmsglen = 0;  // reset on overflow
      }
    }
    else // idle state (1)
    {
      if (10 == b)
      {
        rxmsgbuf[rxmsglen] = 0; // zero terminate the message
        TRACE("WiFi MSG: \"%s\"\r\n", &rxmsgbuf[0]);

        // process the message

        rxmsglen = 0;
      }
      else if (13 == b) // ignore, usually comes before 10
      {

      }
      else
      {
        if (rxmsglen < sizeof(rxmsgbuf))
        {
          rxmsgbuf[rxmsglen] = b;
          ++rxmsglen;
        }
        else
        {
          rxmsglen = 0;  // reset on overflow
        }
      }
    }

    ++rxdmapos;
    if (rxdmapos >= sizeof(rxdmabuf))  rxdmapos = 0;
  }


  // Sending buffered tx messages
  StartSendTxBuffer();
}

void TEspWifiUart::Run()
{
  RunRx();
}

bool TEspWifiUart::MsgOkDetected()
{
  if (0 == memcmp(&rxmsgbuf[rxmsglen - 6], msg_ok, 6))
  {
    return true;
  }
  return false;
}

bool TEspWifiUart::MsgErrorDetected()
{
  if (0 == memcmp(&rxmsgbuf[rxmsglen - 9], msg_error, 9))
  {
    return true;
  }
  return false;
}

void TEspWifiUart::AddUdpSocket(TEspAtUdpSocket * audp)
{
  if (udp_last)
  {
    udp_last->nextsocket = audp;
  }
  else
  {
    udp_last = audp;
    udp_first = audp;
  }

  audp->nextsocket = nullptr;
}

//--------------------------------------------------------

void TEspAtUdpSocket::Init(TEspWifiUart * awifim, uint16_t alistenport)
{
  pwifim = awifim;
  listenport = alistenport;

  pwifim->AddUdpSocket(this);
}

int TEspAtUdpSocket::Send(void * adataptr, unsigned adatalen)
{
  return -1;
}

int TEspAtUdpSocket::Receive(void * adataptr, unsigned adatalen)
{
  int err = 0;

  return err;
}

