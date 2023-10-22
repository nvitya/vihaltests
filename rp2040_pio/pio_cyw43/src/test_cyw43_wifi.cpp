/*
 * test_cyw43_wifi.cpp
 *
 *  Created on: Oct 8, 2023
 *      Author: vitya
 */

#include "board_pins.h"
#include "wifi_cyw43_spi.h"

#include "test_cyw43_wifi.h"

#include "traces.h"

#include "wifi_secure.h"  // copy and adjust the wifi_secure.h.template when not existing

TWifiCyw43SpiComm    cyw43_comm;
TWifiCyw43Spi        cyw43;

void test_cyw43_wifi_init()
{
  TRACE("Initializing CYW43 WiFi...\r\n");

  cyw43_comm.prgoffset = 0;
  cyw43_comm.frequency = 8000000; // slower speed for better analysis
  //cyw43_comm.frequency = 16000000; // slower speed for better analysis
  //cyw43_comm.frequency = 33000000; // high speed, requires extra pulse before the read data
  cyw43_comm.Init(0, 0);

  if (!cyw43.Init(&cyw43_comm, &spiflash))
  {
    TRACE("ERROR Initializing CYW43!\r\n");
    return;
  }

  TRACE("CYW43 WiFi Initialized.\r\n");

  cyw43.GpioSetTo(0, 1); // turn the GPIO LED on

  cyw43.WifiOn();

  cyw43.wifi_ssid     = WIFI_SECURE_SSID;
  cyw43.wifi_password = WIFI_SECURE_PASSWORD;

  if (!cyw43.WifiJoin())
  {
    TRACE("WiFi Join Failed!\r\n");
  }
  else
  {
    TRACE("WiFi Join OK!\r\n");
  }

}

void test_cyw43_wifi_run()
{
  cyw43.Run();
}
