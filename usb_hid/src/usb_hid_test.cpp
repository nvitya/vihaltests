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
 *  file:     usb_hid_test.cpp
 *  brief:    A simple USB device that provides a mouse emulation (HID) function
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#include "usb_hid_test.h"
#include "traces.h"
#include "usbdevice.h"
#include "hwusbctrl.h"

TUsbDevice       usbdev;
TUsbFuncHidTest  hidtest;

#define HID_DESCRIPTOR_TYPE           0x21
#define HID_REPORT_DESC               0x22

//nvitya: this report descriptor is a special one, it describes the report format and its usage
//        these descriptors are usually generated with a special program

const uint8_t HID_MOUSE_ReportDesc[] =
{
  0x05,   0x01,
  0x09,   0x02,
  0xA1,   0x01,
  0x09,   0x01,

  0xA1,   0x00,
  0x05,   0x09,
  0x19,   0x01,
  0x29,   0x03,

  0x15,   0x00,
  0x25,   0x01,
  0x95,   0x03,
  0x75,   0x01,

  0x81,   0x02,
  0x95,   0x01,
  0x75,   0x05,
  0x81,   0x01,

  0x05,   0x01,
  0x09,   0x30,
  0x09,   0x31,
  0x09,   0x38,

  0x15,   0x81,
  0x25,   0x7F,
  0x75,   0x08,
  0x95,   0x03,

  0x81,   0x06,
  0xC0,   0x09,
  0x3c,   0x05,
  0xff,   0x09,

  0x01,   0x15,
  0x00,   0x25,
  0x01,   0x75,
  0x01,   0x95,

  0x02,   0xb1,
  0x22,   0x75,
  0x06,   0x95,
  0x01,   0xb1,

  0x01,   0xc0
};

/* USB HID device Configuration Descriptor */
const uint8_t USBD_HID_Desc[] =
{
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  sizeof(HID_MOUSE_ReportDesc),/*wItemLength: Total length of Report descriptor*/
  0x00,
};

//-----------------------------------------------------------

bool TUifHidTest::InitInterface()
{
	hiddata.dx = 0;
	hiddata.dy = 0;

	intfdesc.interface_class = 3; // HID
	intfdesc.interface_subclass = 1; // boot
	intfdesc.interface_protocol = 2; // mouse

	interface_name = "VIHAL USB HID Test";

	AddConfigDesc((void *)&USBD_HID_Desc[0], false);
	AddDesc(HID_REPORT_DESC, (void *)&HID_MOUSE_ReportDesc[0], sizeof(HID_MOUSE_ReportDesc), 0);

	ep_hidreport.Init(HWUSB_EP_TYPE_INTERRUPT, false, 4);
	AddEndpoint(&ep_hidreport);

	return true;
}

void TUifHidTest::SendReport(int8_t adx, int8_t ady)
{
	hiddata.dx = adx;
	hiddata.dy = ady;

	ep_hidreport.StartSendData(&hiddata, sizeof(hiddata));
}

void TUifHidTest::OnConfigured()
{
	// after the configuration it must send a report immediately!
	SendReport(0, 0);
}

bool TUifHidTest::HandleSetupRequest(TUsbSetupRequest * psrq)
{
	if (psrq->rqtype == 0x21)
	{
		if (psrq->request == 0x0A)
		{
			// set idle frequency.
			device->SendControlStatus(true);
			return true;
		}
	}
	else if (psrq->rqtype == 0xA1) // vendor request
	{
		device->SendControlStatus(true);
		return true;
	}

	return super::HandleSetupRequest(psrq);
}

bool TUifHidTest::HandleTransferEvent(TUsbEndpoint * aep, bool htod)
{
	if (htod)
	{
		// impossible case
		return false;
	}
	else
	{
		// called when the hid report sent successfully
		return true;
	}
}

//-----------------------------------------------------------

bool TUsbFuncHidTest::InitFunction()
{
  funcdesc.function_class = 0;
  funcdesc.function_sub_class = 0;
  funcdesc.function_protocol = 0;

  AddInterface(&uif);

  return true;
}

void TUsbFuncHidTest::Run()
{
}

//-----------------------------------------------------------

void usb_hid_test_init()
{
	TRACE("Initializing USB HID Test\r\n");

	usbdev.devdesc.vendor_id = 0xDEAD;
	usbdev.devdesc.product_id = 0x0A1D;
	usbdev.manufacturer_name = "github.com/vihal";
	usbdev.device_name = "VIHAL Example HID Joystick";
	usbdev.device_serial_number = "VIHAL-HID-1";

	usbdev.AddFunction(&hidtest);

	if (!usbdev.Init()) // this must be the last one, when the interfaces added
	{
		TRACE("Error initializing USB device!\r\n");
		return;
	}
}

void usb_hid_test_run()
{
	usbdev.HandleIrq();

	usbdev.Run();
}

void usb_hid_test_heartbeat()
{
	if (hidtest.uif.configured)
	{
		hidtest.uif.SendReport(2, 3);
	}
}

