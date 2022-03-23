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
 *  file:     usbdev_main.cpp
 *  brief:    USB Device Definition (USB to Serial USB Device)
 *  version:  1.00
 *  date:     2020-07-31
 *  authors:  nvitya
*/

/* TEST String

12345678901234567890123456789012345678901234567890123456789012345678901234567890
12345678901234567890123456789012345678901234567890123456789012345678901234567890
12345678901234567890123456789012345678901234567890123456789012345678901234567890

*/

#include <usbdev_main.h>
#include "string.h"
#include "traces.h"
#include "hwpins.h"
#include "usbdevice.h"
#include "hwusbctrl.h"
#include "board_pins.h"

TUsbDevMain          usbdev;
TUsbFuncCdcUart      cdcuart;

void usb_device_init()
{
	TRACE("Initializing USB Device\r\n");

	if (!usbdev.Init()) // calls InitDevice first which sets up the device
	{
		TRACE("Error initializing USB device!\r\n");
		return;
	}

	TRACE("IF input endpoint: %02X\r\n", cdcuart.uif_data.ep_input.index);
	TRACE("IF output endpoint: %02X\r\n", cdcuart.uif_data.ep_output.index);
}

void usb_device_run()
{
	usbdev.HandleIrq();

	usbdev.Run();
}

//------------------------------------------------

bool TUsbDevMain::InitDevice()
{
	devdesc.vendor_id = 0xDEAD;
	devdesc.product_id = 0xCDC5;

	manufacturer_name = "github.com/vihal";
	device_name = "VIHAL CDC UART Example";
	device_serial_number = "VIHAL-CDC-UART-1";

  usbuart.DmaAssign(true,  &usbuart_dma_tx);
  usbuart.DmaAssign(false, &usbuart_dma_rx);

	cdcuart.AssignUart(&usbuart);
	AddFunction(&cdcuart);

	return true;
}

