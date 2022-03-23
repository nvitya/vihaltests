// linux test project for the USB-spec device
//
// The USB device function is originated from Niklas Gürtler:
//   https://github.com/Erlkoenig90/f1usb
//   https://www.mikrocontroller.net/articles/USB-Tutorial_mit_STM32
//
// This test is quite different, somewhat simpler than his: https://github.com/Erlkoenig90/usbclient
//
// This was tested only from Linux so far.

#include "stdio.h"
#include "stdlib.h"
#include "libusb.h"

libusb_context * luctx = nullptr;
libusb_device_handle * handle = nullptr;

void list_usb_devices()
{
	int r;

	printf("Listing connected USB devices:\n");

	libusb_device ** usb_dev_list;
	unsigned devcnt = libusb_get_device_list(nullptr, &usb_dev_list);
	for (unsigned n = 0; n < devcnt; ++n)
	{
		libusb_device * device = usb_dev_list[n];

		libusb_device_descriptor devdesc;
		r = libusb_get_device_descriptor(device, &devdesc);
		if (r < 0)
		{
			printf("Error getting device descriptor: %i\n", r);
			break;
		}

		printf("Bus %2i, Port %2i, Addr %3i, Vendor:Product %04X:%04X\n",
			libusb_get_bus_number(device),
			libusb_get_port_number(device),
			libusb_get_device_address(device),
			devdesc.idVendor, devdesc.idProduct
		);
	}

	libusb_free_device_list(usb_dev_list, 1);
}

void test_usb_control_transfer()
{
	printf("USB Control transfer test\n");
	int r;
	uint8_t data;
	r = libusb_control_transfer(handle, 0xC0, 2, 0, 0, &data, 1, 0);  // getting the data
	if (r < 0)
	{
		printf("  Error getting data: %i\n", r);
		return;
	}

	printf("  actual LED data: %i\n", data);

	data = (~data & 3);
	r = libusb_control_transfer(handle, 0x40, 1, data, 0, nullptr, 0, 0); // sending back to wValue
	if (r < 0)
	{
		printf("  Error setting data: %i\n", r);
		return;
	}
	printf("  LED data was set to: %i\n", data);
}

void test_usb_bulk_transfer()
{
	int r;
	unsigned n;
	int count;
	uint8_t txbuf[64];
	uint8_t rxbuf[64];

	printf("USB Bulk transfer test\n");

	printf("  TX DATA:\n   ");

	for (n = 0; n < sizeof(txbuf); ++n)
	{
		txbuf[n] = 0x5A + n;
		printf(" %02X", txbuf[n]);
	}
	printf("\n");

	// send

	r = libusb_bulk_transfer(handle, 1, &txbuf[0], sizeof(txbuf), &count, 0);
	if (r)
	{
		printf("  Error sending data: %i\n", r);
		return;
	}
	printf("  %i bytes sent.\n", count);

	// Warning, this is a difference from the original test from Niklas Gürtler:
	//   Enpoint 2 (instead of endpoint 1) is used here, because the NVCM does not support bi-directional endpoints !
	//   (some ARM microcontrollers (e.g. ATSAM) does not support bi-directional endpoints other than control endpoints)

	uint8_t epnum = 2;
	printf("Receiving from ep %i...\n", epnum);
	r = libusb_bulk_transfer(handle, 0x80 + epnum, &rxbuf[0], sizeof(rxbuf), &count, 0);
	if (r)
	{
		printf("  Error receiving data: %i\n", r);
		return;
	}
	printf("  %i bytes received:\n   ", count);
	for (n = 0; n < unsigned(count); ++n)
	{
		printf(" %02X", rxbuf[n]);
	}
	printf("\n");
}


int main(int argc, char * argv[])
{
	int r;

	printf("Special USB Device test (libusb)\n");

	// Init libusb
	r = libusb_init(&luctx);
	if (r)
	{
		printf("libusb init error: %i\n", r);
		exit(1);
	}

	printf("libusb init ok.\n");

	// list devices
	list_usb_devices();

	handle = libusb_open_device_with_vid_pid(luctx, 0xDEAD, 0xBEEF);
	if (!handle)
	{
		printf("Error Opening DEAD:BEEF!\n");
	}
	else
	{
		printf("USB device DEAD:BEEF opened successfully.\n");

		r = libusb_claim_interface(handle, 0);
		if (r)
		{
			printf("Claim interface error: %i\n", r);
		}

		// show device strings
		unsigned char sbuf[127];
		int len;
		libusb_device * device = libusb_get_device(handle);
		libusb_device_descriptor devdesc;
		if (0 == libusb_get_device_descriptor(device, &devdesc))
		{
			if (devdesc.iManufacturer)
			{
				len = libusb_get_string_descriptor_ascii(handle, devdesc.iManufacturer, &sbuf[0], sizeof(sbuf) - 1);
				if (len >= 0)
				{
					sbuf[len] = 0;
					printf("  Manufacturer: \"%s\"\n", &sbuf[0]);
				}
			}
			if (devdesc.iProduct)
			{
				len = libusb_get_string_descriptor_ascii(handle, devdesc.iProduct, &sbuf[0], sizeof(sbuf) - 1);
				if (len >= 0)
				{
					sbuf[len] = 0;
					printf("  Product: \"%s\"\n", &sbuf[0]);
				}
			}
			if (devdesc.iSerialNumber)
			{
				len = libusb_get_string_descriptor_ascii(handle, devdesc.iSerialNumber, &sbuf[0], sizeof(sbuf) - 1);
				if (len >= 0)
				{
					sbuf[len] = 0;
					printf("  Serial: \"%s\"\n", &sbuf[0]);
				}
			}
		}

		// do some tests

		test_usb_control_transfer();

		test_usb_bulk_transfer();
	}

	printf("USB Test finished.\n");

	if (handle)
	{
		libusb_close(handle);
	}

	if (luctx)
	{
		libusb_exit(luctx);
	}
}

