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

#include "net_ip4.h"

uint8_t   my_ip_address[4] = {192, 168, 2, 10};

// combined buffer for the Ethernet RX, TX descriptors, packet buffers and later TCP buffers
uint8_t   network_memory[32 * 1024] __attribute__((aligned(32)));

unsigned last_recv_time = 0;

TNetAdapter    netif;

TIp4Handler    ip4_handler;

TUdp4Socket  udp;

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

  netif.Init(&eth, &network_memory[0], sizeof(network_memory));  // Ethernet initialization included

  ip4_handler.ipaddress.Set(192, 168, 2, 10);
  ip4_handler.netmask.Set(255, 255, 255, 0);
  ip4_handler.gwaddress.u32 = 0;
  ip4_handler.Init(&netif);

  // 3. INITIALIZE THE UDP4 SOCKET
  //udp.Init(&netif, 3000);


  last_recv_time = CLOCKCNT;
}

void udp_test_run()
{
  int len;
  int i;

  netif.Run(); // must be called regularly

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
      TRACE("UDP Send Error: %i\r\n", r);
    }
  }
}
