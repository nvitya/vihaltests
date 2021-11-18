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
 *  file:     usb_hid_test.h
 *  brief:    A simple USB device that provides a mouse emulation (HID) function
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#ifndef SRC_USB_HID_TEST_H_
#define SRC_USB_HID_TEST_H_

#include "usbdevice.h"

typedef struct THidData
{
	uint8_t  data1;
	int8_t   dx;
	int8_t   dy;
	uint8_t  data2;
//
} THidData;

class TUifHidTest : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:
	THidData        hiddata;

	uint8_t         protocol = 0;
	uint8_t         idlestate = 1;

	TUsbEndpoint    ep_hidreport;
	void            SendReport(int8_t adx, int8_t ady);

public: // mandatory functions
	virtual bool    InitInterface();
	virtual void    OnConfigured();
	virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);
	virtual bool    HandleSetupRequest(TUsbSetupRequest * psrq);

};

extern TUifHidTest hidtest;

void usb_hid_test_init();
void usb_hid_test_run();
void usb_hid_test_heartbeat();


#endif /* SRC_USB_HID_TEST_H_ */
