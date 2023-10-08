/*
 * test_cyw43_wifi.cpp
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

#include "wifi_cyw43_spi.h"

#include "test_cyw43_wifi.h"

#include "traces.h"

TWifiCyw43Spi    cyw43;
TPioAppCyw43Spi  pioapp_cyw43;

void test_cyw43_wifi_init()
{
  TRACE("Initializing CYW43 WiFi...\r\n");

  pioapp_cyw43.prgoffset = 0;
  pioapp_cyw43.frequency = 8000000; // slower speed for better analysis
  //pioapp_cyw43.frequency = 33000000; // high speed, requires extra pulse before the read data
  pioapp_cyw43.Init(0, 0);

  if (!cyw43.Init(&pioapp_cyw43))
  {
    TRACE("ERROR Initializing CYW43!\r\n");
    return;
  }

  TRACE("CYW43 WiFi Initialized.\r\n");
}

void test_cyw43_wifi_run()
{

}
