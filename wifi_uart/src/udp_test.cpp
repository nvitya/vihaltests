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

// copy the wifi_secret.h.template, and adjust for your needs
#include "wifi_secret.h"

bool prev_link_up = false;

unsigned send_cnt = 0;
unsigned last_send_time;
unsigned last_recv_time;

// combined buffer for storeing packets
uint8_t network_memory[6 * 1024] __attribute__((aligned(32)));

uint8_t pbuf[1536] __attribute__((aligned(16)));  // 16 alignment is useful for the debugging

const char * udp_test_message = "VIHAL UDP TEST MESSAGE";

TEspAtUdpSocket  udp;

//--------------------------------------------------------

void udp_test_init()
{
  // 1. INITIALIZE THE WIFI MODULE

  wifi.ssid = WIFI_SECRET_SSID;
  wifi.password = WIFI_SECRET_PW;

  wifi.ipaddress.Set(192, 168, 0, 77);
  wifi.netmask.Set(255, 255, 255, 0);
  wifi.gwaddress.Set(192, 168, 0, 1);
  wifi.dns.Set(192, 168, 0, 1);

  if (!wifi.Init(&network_memory[0], sizeof(network_memory)))
  {
    TRACE("Error initializing the WiFi module !\r\n");
    return;
  }

  udp.Init(&wifi, 5000);

  last_recv_time = CLOCKCNT;
  last_send_time = CLOCKCNT;
}

void udp_test_run()
{
  int len;
  int i;

  unsigned t = CLOCKCNT;

  wifi.Run(); // must be called regularly

  if (wifi.IsLinkUp())
  {
    if (!prev_link_up)
    {
      TRACE("Link up detected.\r\n");
      last_send_time = t;
    }
    prev_link_up = true;

    if ((send_cnt < 0) && (t - last_send_time > SystemCoreClock / 10))
    {
      udp.destaddr.Set(192, 168, 0, 99);
      udp.destport = 5000;
      len = 80;
      for (i = 0; i < len; ++i)
      {
        pbuf[i] = 1 + i;
      }

      TRACE("Sending UDP...\r\n");
      udp.Send((void *)udp_test_message, strlen(udp_test_message));

      ++send_cnt;

      last_send_time = t;
    }
  }

  // receive does not block
  // if there are something in the receive queue then this returns with a positive number
  len = udp.Receive(&pbuf[0], sizeof(pbuf));
  if (len > 0)
  {
#if 1 // no traces for benchmarking
    TRACE("UDP MESSAGE RECEIVED: \"");
    for (i = 0; i < len; ++i)
    {
      TRACE("%c", pbuf[i]);
    }
    TRACE("\"\r\n");
#endif

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
