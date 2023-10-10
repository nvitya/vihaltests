/*
 * test_cyw43_wifi.cpp
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

#include "wifi_cyw43_spi.h"

#include "test_cyw43_wifi.h"

#include "traces.h"

TWifiCyw43SpiComm    cyw43_comm;
TWifiCyw43Spi        cyw43;

void test_cyw43_wifi_init()
{
  TRACE("Initializing CYW43 WiFi...\r\n");

  cyw43_comm.prgoffset = 0;
  cyw43_comm.frequency = 8000000; // slower speed for better analysis
  //cyw43_comm.frequency = 33000000; // high speed, requires extra pulse before the read data
  cyw43_comm.Init(0, 0);

  if (!cyw43.Init(&cyw43_comm))
  {
    TRACE("ERROR Initializing CYW43!\r\n");
    return;
  }

  TRACE("CYW43 WiFi Initialized.\r\n");
}

void test_cyw43_wifi_run()
{

}
