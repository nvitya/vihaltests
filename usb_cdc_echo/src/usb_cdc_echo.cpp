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
 *  file:     usb_cdc_echo.cpp
 *  brief:    CDC Echo USB Device definition
 *  version:  1.00
 *  date:     2020-06-19
 *  authors:  nvitya
*/

#include "string.h"
#include <usb_cdc_echo.h>
#include "traces.h"
#include "hwpins.h"
#include "usbdevice.h"
#include "hwusbctrl.h"


TUsbFuncCdcEcho  cdc_echo;
TUsbDevCdcEcho   usbdev;

void usb_device_init()
{
	TRACE("Initializing CDC Echo USB Device\r\n");

	if (!usbdev.Init()) // calls InitDevice first which sets up the device
	{
		TRACE("Error initializing USB device!\r\n");
		return;
	}

	TRACE("IF input endpoint: %02X\r\n", cdc_echo.uif_data.ep_input.index);
	TRACE("IF output endpoint: %02X\r\n", cdc_echo.uif_data.ep_output.index);
}

void usb_device_run()
{
	usbdev.HandleIrq();
}

//----------------------------------------------------

bool TUifCdcControl::InitInterface()
{
	intfdesc.interface_class = 2; // CDC
	intfdesc.interface_subclass = 2; // Abstract Control Model
	intfdesc.interface_protocol = 0; // 0 = no class specitic control

	interface_name = "VCP Control";

	// some other descriptors are required

  desc_cdc_callmgmt.data_interface = index + 1;  // the following interface must be the data interface
  desc_cdc_union.master_interface = index;
  desc_cdc_union.slave_interface = index + 1;

	AddConfigDesc((void *)&cdc_desc_header_func[0],     true);
	AddConfigDesc((void *)&desc_cdc_callmgmt, true);
	AddConfigDesc((void *)&cdc_desc_call_acm_func[0],   true);
	AddConfigDesc((void *)&desc_cdc_union, true);

	// endpoints

	ep_manage.Init(HWUSB_EP_TYPE_INTERRUPT, false, 64);
	ep_manage.interval = 10; // polling interval
	AddEndpoint(&ep_manage);

	return true;
}

void TUifCdcControl::OnConfigured()
{
	//TRACE("SPEC Device Configured.\r\n");

	//ep_manage.EnableRecv();
}

bool TUifCdcControl::HandleTransferEvent(TUsbEndpoint * aep, bool htod)
{
	return false;
}

bool TUifCdcControl::HandleSetupRequest(TUsbSetupRequest * psrq)
{
	if (0x20 == psrq->request) // set line coding, data stage follows !
	{
		TRACE("VCP Set line coding (SETUP)\r\n");
		device->StartSetupData();  // start receiving the data part, which will be handled at the HandleSetupData()
		return true;
	}
	else if (0x21 == psrq->request) // Get line coding
	{
		TRACE("VCP Get line coding\r\n");
		device->StartSendControlData(&linecoding, sizeof(linecoding));
		return true;
	}
	else if (0x22 == psrq->request) // Set Control Line State
	{
		TRACE("VCP Set Control Line State: %04X\r\n", psrq->value);
		device->SendControlStatus(true);
		return true;
	}

	return false;
}

// the setup request's data part comes in a separate phase and so it has a separate callback:
bool TUifCdcControl::HandleSetupData(TUsbSetupRequest * psrq, void * adata, unsigned adatalen)
{
	if (0x20 == psrq->request) // set line coding
	{
		memcpy(&linecoding, adata, sizeof(linecoding));

		TRACE("VCP Line Coding data:\r\n  baud=%i, format=%i, parity=%i, bits=%i\r\n",
				linecoding.baudrate, linecoding.charformat, linecoding.paritytype, linecoding.databits
		);

		// todo: update UART

		device->SendControlStatus(true);
		return true;
	}

	return false;
}

//------------------------------------------------

bool TUifCdcData::InitInterface()
{
	intfdesc.interface_class = 0x0A; // CDC Data
	intfdesc.interface_subclass = 0;
	intfdesc.interface_protocol = 0; // no specific protocol

	interface_name = "VCP Data";

	// endpoints

	ep_input.Init(HWUSB_EP_TYPE_BULK, true, 64);
	AddEndpoint(&ep_input);
	ep_output.Init(HWUSB_EP_TYPE_BULK, false, 64);
	AddEndpoint(&ep_output);

	return true;
}

void TUifCdcData::OnConfigured()
{
	TRACE("VCP Data Configured.\r\n");

	ep_input.EnableRecv();
}

bool TUifCdcData::HandleTransferEvent(TUsbEndpoint * aep, bool htod)
{
	int r;
	if (htod)
	{
		r = ep_input.ReadRecvData(&databuf[0], sizeof(databuf));
		//TRACE("%i byte VCP data arrived\r\n", r);

#if 0
		for (unsigned n = 0; n < r; ++n)
		{
		   if (databuf[n] >=32)
		   {
		     TRACE("%c", databuf[n]);
		   }
		   else
		   {
		     TRACE(".");
		   }
		}
		TRACE("\r\n");
#endif

		// send it back
		ep_output.StartSendData(&databuf[0], r);
	}
	else
	{
		ep_input.EnableRecv();  // enable new data receive only after when the response was successfully sent
	}

	return true;
}

//------------------------------------------------

bool TUsbFuncCdcEcho::InitFunction()
{
  funcdesc.function_class = 2;
  funcdesc.function_sub_class = 2;
  funcdesc.function_protocol = 1;

  AddInterface(&uif_control);
  AddInterface(&uif_data);

  return true;
}

//------------------------------------------------

bool TUsbDevCdcEcho::InitDevice()
{
	devdesc.vendor_id = 0xDEAD;
	devdesc.product_id = 0xCDCE;

	manufacturer_name = "github.com/vihal";
	device_name = "VIHAL CDC Echo Example";
	device_serial_number = "VIHAL-CDC-Echo-1";

	AddFunction(&cdc_echo);

	return true;
}

