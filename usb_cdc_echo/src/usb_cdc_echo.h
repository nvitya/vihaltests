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
 *  file:     usb_cdc_echo.h
 *  brief:    CDC Echo USB Device definition
 *  version:  1.00
 *  date:     2020-06-19
 *  authors:  nvitya

*/

#ifndef SRC_USB_CDC_ECHO_H_
#define SRC_USB_CDC_ECHO_H_

#include "usbdevice.h"
#include "usbif_cdc.h"

class TUifCdcControl : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:

  TUsbCdcDescCallUnionFunc  desc_cdc_union =
  {
    .length = 5,
    .descriptor_type = 0x24,  // 0x24 = CS_INTERFACE
    .descriptor_subtype = 6,  // 6 = union func
    .master_interface = 0,    // will be set automatically
    .slave_interface = 1      // will be set automatically
  };

  TUsbCdcDescCallManagement  desc_cdc_callmgmt =
  {
    .length = 5,
    .descriptor_type = 0x24,  // 0x24 = CS_INTERFACE
    .descriptor_subtype = 6,  // 6 = union func
    .capabilities = 0,        // 0 = no call management
    .data_interface = 0       // will be set automatically
  };

	TCdcLineCoding  linecoding;

	TUsbEndpoint    ep_manage;
	uint8_t         databuf[64];

public: // mandatory functions
	virtual bool    InitInterface();
	virtual void    OnConfigured();
	virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);
	virtual bool    HandleSetupRequest(TUsbSetupRequest * psrq);
	virtual bool    HandleSetupData(TUsbSetupRequest * psrq, void * adata, unsigned adatalen);

};

class TUifCdcData : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:
	TUsbEndpoint    ep_input;
	TUsbEndpoint    ep_output;

	uint8_t         databuf[64];

public: // mandatory functions
	virtual bool    InitInterface();
	virtual void    OnConfigured();
	virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);
};

class TUsbFuncCdcEcho : public TUsbFunction
{
public:
  TUifCdcControl       uif_control;
  TUifCdcData          uif_data;

  virtual bool         InitFunction();
};


class TUsbDevCdcEcho : public TUsbDevice
{
private:
	typedef TUsbDevice super;

public: // mandatory functions

	virtual bool    InitDevice();

};

extern TUsbFuncCdcEcho  cdc_echo;
extern TUsbDevCdcEcho   usbdev;

void usb_device_init();
void usb_device_run();


#endif /* SRC_USB_CDC_ECHO_H_ */
