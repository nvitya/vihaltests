/*
 * udp_test.cpp
 *
 *  Created on: Mar 14, 2022
 *      Author: vitya
 */

#include "string.h"
#include "board_pins.h"
#include "traces.h"
#include "udp_test.h"
#include "netadapter.h"
#include "net_ip4.h"

// combined buffer for the Ethernet RX, TX descriptors, packet buffers and later TCP buffers
uint8_t   network_memory[32 * 1024] __attribute__((aligned(32)));

TNetAdapter  nadap;
TIp4Handler  ip4_handler;
TUdp4Socket  udp;

bool prev_link_up = false;

unsigned send_cnt = 0;
unsigned last_send_time;
unsigned last_recv_time;
uint8_t pbuf[1536] __attribute__((aligned(16)));  // alignment is useful for the debugging

//--------------------------------------------------------

void udp_test_init()
{
  // 1. INITIALIZE THE ETHERNET HARDWARE

  //eth.promiscuous_mode = true;

  // random generated mac address:
  eth.mac_address[0] = 0xE4;
  eth.mac_address[1] = 0x88;
  eth.mac_address[2] = 0xF9;
  eth.mac_address[3] = 0xB4;
  eth.mac_address[4] = 0xFE;
  eth.mac_address[5] = 0x70;

  // 2. INITIALIZE THE Adapter

  nadap.Init(&eth, &network_memory[0], sizeof(network_memory));  // Ethernet initialization included

  ip4_handler.ipaddress.Set(192, 168, 2, 10);
  ip4_handler.netmask.Set(255, 255, 255, 0);
  ip4_handler.gwaddress.u32 = 0; //.Set(255, 255, 255, 0);
  ip4_handler.Init(&nadap);

  // 3. INITIALIZE THE UDP4 SOCKET
  udp.Init(&ip4_handler, 5000);

  last_recv_time = CLOCKCNT;
  last_send_time = CLOCKCNT;
}

void udp_test_run()
{
  int len;
  int i;

  unsigned t = CLOCKCNT;

  nadap.Run(); // must be called regularly

  if (nadap.IsLinkUp())
  {
    if (!prev_link_up)
    {
      TRACE("Link up detected.\r\n");
      last_send_time = t;
    }
    prev_link_up = true;


    if ((send_cnt < 2) && (t - last_send_time > SystemCoreClock / 10))
    {
      udp.destaddr.Set(192, 168, 2, 1);
      udp.destport = 5000;
      len = 80;
      for (i = 0; i < len; ++i)
      {
        pbuf[i] = 1 + i;
      }

      TRACE("Sending UDP...\r\n");
      udp.Send(&pbuf[0], len);

      ++send_cnt;

      last_send_time = t;
    }
  }


  // receive does not block
  // if there are something in the receive queue then this returns with a positive number
  len = udp.Receive(&pbuf[0], sizeof(pbuf));
  if (len > 0)
  {
    // increment data and send it back
    for (i = 0; i < len; ++i)
    {
      ++(pbuf[i]);
    }

    udp.destaddr = udp.srcaddr;
    udp.destport = udp.srcport;

    int r = udp.Send(&pbuf[0], len);
    if (r <= 0)
    {
      // if there are no more place in send the queue
      TRACE("UDP Send Error: %i\r\n", r);
    }
  }
}
