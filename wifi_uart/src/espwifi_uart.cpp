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

  if (pin_en.Assigned())
  {
    pin_en.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  }

  if (pin_rst.Assigned())
  {
    pin_rst.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
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
  state = 0;
  cmd_running = false;
  initstate = 1;  // start initialization
  prev_state_time = CLOCKCNT;

  ResetConnection();

  us_clocks = SystemCoreClock / 1000000;
  ms_clocks = SystemCoreClock / 1000;
  cmd_timeout_clocks = ms_clocks * 5000;

  initialized = true;
  return true;
}

void TEspWifiUart::RunInit()
{
  unsigned t = CLOCKCNT;
  unsigned elapsedclocks = t - prev_state_time;

  if (cmd_error && !cmd_ignore_error && (initstate < 100))
  {
    initstate = 100;
  }

  if (1 == initstate)
  {
    TRACE("ESP-AT: starting initialization...\r\n");

    uart.SetBaudRate(initial_uart_speed);  // switch back to the initial speed mode

    // start the initialization with reset
    if (pin_en.Assigned())
    {
      pin_en.Set1();
    }

    if (pin_rst.Assigned())
    {
      pin_rst.Set0();
      prev_state_time = t;
      ++initstate;
    }
  }
  else if (2 == initstate) // hold the reset low
  {
    if (elapsedclocks > 10 * ms_clocks)
    {
      if (pin_rst.Assigned())  pin_rst.Set1();

      rxmsglen = 0;

      ready_received = false;
      ResetConnection();

      prev_state_time = t;
      ++initstate;
    }
  }
  else if (3 == initstate)  // wait for the ready message
  {
    if (ready_received)
    {
      StartCommand("AT+CWQAP");  // do not connect to the stored access point
      cmd_ignore_error = true;
      initstate = 4;
    }
    else if (elapsedclocks > 5000 * ms_clocks)  // 5 s timeout
    {
      initstate = 1; // back to the reset
      prev_state_time = t;
    }
  }
  else if (4 == initstate)
  {
    StartCommand("AT+CWMODE=1");
    initstate = 10;
  }

  // switch to high-speed mode

  else if (10 == initstate)
  {
    StartCommand("AT+UART_CUR?");
    //StartCommand("AT+GMR");
    //initstate = 17;
    //initstate = 20;
    ++initstate;
  }
  else if (11 == initstate)
  {
    TRACE("ESP-AT: switch to %u bit/s\r\n", uart_speed);

    StartCommand("AT+UART_CUR=%u,8,1,0,0", uart_speed);
    ++initstate;
  }
  else if (12 == initstate) // wait until the TX message is sent
  {
    uart.SetBaudRate(uart_speed);

    TRACE("ESP-AT: high speed uart configured.\r\n");
    StartCommand("AT+UART_CUR?");
    ++initstate;
  }
  else if (13 == initstate)
  {
    initstate = 20;
  }

  else if (17 == initstate)
  {
    // stay here
  }

  // prepare the connections

  else if (20 == initstate)
  {
    // configure the IP address
    StartCommand("AT+CIPSTA=\"%u.%u.%u.%u\",\"%u.%u.%u.%u\",\"%u.%u.%u.%u\"",
        ipaddress.u8[0], ipaddress.u8[1], ipaddress.u8[2], ipaddress.u8[3],
        gwaddress.u8[0], gwaddress.u8[1], gwaddress.u8[2], gwaddress.u8[3],
        netmask.u8[0], netmask.u8[1], netmask.u8[2], netmask.u8[3]
    );
    ++initstate;
  }
  else if (21 == initstate) // process setting static IP
  {
    StartCommand("AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    ++initstate;
  }
  else if (22 == initstate) // joining the AP
  {
    if (wifi_got_ip)
    {
      StartCommand("AT+CIPMUX=1");
      ++initstate;
    }
  }
  else if (23 == initstate) // show the remote IP address in the +IPD data
  {
    StartCommand("AT+CIPDINFO=1");
    ++initstate;
  }
  else if (24 == initstate) // processing CIPMUX=1 result
  {
    StartCommand("AT+CIPSTART=1,\"UDP\",\"%u.%u.%u.%u\",5000,5000,2",
        0,0,0,0
    );
    ++initstate;
  }
  else if (25 == initstate)
  {
    initstate = 200; // the end so far
  }


  //---------------------------------------------------------------------------

  else if (100 == initstate)
  {
    TRACE("ESP-AT: Initialization error.\r\n");
    ++initstate;
  }
  else if (101 == initstate)
  {
    // stay here
  }

  else if (200 == initstate)
  {
    TRACE("ESP-AT: Init finished.\r\n");
    initstate = 0;
  }

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
    TRACE("ESP-AT -> \"%s\"\r\n", &fmtbuf[0]);

    AddTx(&fmtbuf[0], len);
    AddTx((void *)"\r\n", 2);
  }

  va_end(arglist);

  cmd_ignore_error = false;
  cmd_running = true;
  cmd_echo_received = false;
  cmd_start_time = CLOCKCNT;
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

void TEspWifiUart::ResetConnection()
{
  wifi_connected = false;
  wifi_got_ip = false;
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

    if (10 == b)
    {
      rxmsgbuf[rxmsglen] = 0; // zero terminate the message

      if (rxmsglen > 0)
      {
        ProcessRxMessage();
        rxmsglen = 0;
      }
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
        // TODO: figure out something better
        rxmsglen = 0;  // reset on overflow
      }
    }

    ++rxdmapos;
    if (rxdmapos >= sizeof(rxdmabuf))  rxdmapos = 0;
  }
}

void TEspWifiUart::Run()
{
  RunRx();

  if (cmd_running)
  {
    if (CLOCKCNT - cmd_start_time > cmd_timeout_clocks)
    {
      cmd_error = true;
      cmd_running = false;
      TRACE("ESP-AT: cmd timeout!\r\n");
    }
  }

  if (!cmd_running)
  {
    if (initstate)
    {
      RunInit();
    }
    else
    {
      // run transactions
    }
  }

  StartSendTxBuffer();  // Sending buffered tx messages
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

void TEspWifiUart::ProcessRxMessage()
{
  if (!ready_received)
  {
    // check for ready
    if (memcmp(&rxmsgbuf[0], msg_ready, 5) == 0)
    {
      TRACE("ESP-AT: ready detected.\r\n");
      ready_received = true;
      return;
    }

    return; // at the beginning there comes garbage stop here
  }

  if (cmd_running)
  {
    if (memcmp(&rxmsgbuf[0], msg_ok, 2) == 0)
    {
      cmd_running = false;
      cmd_error = false;
      return;
    }

    if (memcmp(&rxmsgbuf[0], msg_error, 5) == 0)
    {
      cmd_running = false;
      cmd_error = true;
      return;
    }

    if (!cmd_echo_received)
    {
      cmd_echo_received = true;
      return;
    }
  }

  if (0 == memcmp(&rxmsgbuf[0], "WIFI CONNECTED", 14))
  {
    TRACE("ESP-AT: Wifi Connected.\r\n");
    wifi_connected = true;
    return;
  }

  if (0 == memcmp(&rxmsgbuf[0], "WIFI GOT IP", 11))
  {
    TRACE("ESP-AT: Wifi got IP.\r\n");
    wifi_got_ip = true;
    return;
  }

  // some other asynchronous messages

  TRACE("ESP-AT: Unprocessed msg: \"%s\"\r\n", &rxmsgbuf[0]);
}
