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
 *  file:     usb_spec_dev.h
 *  brief:    Special USB Device definition
 *  version:  1.00
 *  date:     2020-06-19
 *  authors:  nvitya
 *  notes:
 *   The "definition" of this special USB device is originated from Niklas Gürtler:
 *     https://github.com/Erlkoenig90/f1usb
 *     https://www.mikrocontroller.net/articles/USB-Tutorial_mit_STM32
 *
 *   This implementation is somewhat different, because using enpoint 2 for buld data in.
 *
 *   The device does not provide a standard interface, it can be controller with libusb.
 *   The project for testing it: https://github.com/nvitya/nvcmtests/usbclient_spec
 *   Or using the one from Niklas Gürtler: https://github.com/Erlkoenig90/usbclient
*/

#ifndef SRC_USB_SPEC_DEV_H_
#define SRC_USB_SPEC_DEV_H_

#include "usbdevice.h"

class TUifSpecTest : public TUsbInterface
{
private:
  typedef TUsbInterface super;

public:
  uint8_t         protocol = 0;
  uint8_t         idlestate = 1;

  TUsbEndpoint    ep_input;
  TUsbEndpoint    ep_output;

  uint8_t         databuf[64];

public: // mandatory functions
  virtual bool    InitInterface();
  virtual void    OnConfigured();
  virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);
};

class TUsbFuncSpec : public TUsbFunction
{
public:
  TUifSpecTest         uif_spectest;

  virtual bool         InitFunction();
};

class TUsbDevSpec : public TUsbDevice
{
private:
  typedef TUsbDevice super;

public: // mandatory functions

  virtual bool    InitDevice();

  virtual bool    HandleSpecialSetupRequest();  // returns true when handled

};

extern TUsbFuncSpec usb_spec;
extern TUsbDevSpec  usbdev;

void usb_device_init();
void usb_device_run();

#endif /* SRC_USB_SPEC_DEV_H_ */
