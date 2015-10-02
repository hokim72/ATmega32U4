// Main source file for the Bulk Vendor demo. This file contains the main tasks
// of the demo and is responsible for the initial application hardware configuration.

#include "LufaUtil.h"
#include "BulkVendor.h"
#include "global.h"
#ifdef MY_DEBUG
#include "uart.h"
#include "rprintf.h"
#endif

USB_EPInfo_Device_t BulkVendor_EPs=
{
	.DataINEPAddress = VENDOR_IN_EPADDR,
	.DataOUTEPAddress = VENDOR_OUT_EPADDR
};

// Main program entry point. This routine configures the hardware required by
// the application, then enters a loop to run the application tasks in sequnece.

static FILE USBBulkStream;

int main(void)
{
	#ifdef MY_DEBUG
	uartInit();
	uartSetBaudRate(9600);
	rprintfInit(uartSendByte);
	rprintf("test start....\n");
	#endif

	SetupHardware();

	Device_CreateStream(&BulkVendor_EPs, &USBBulkStream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	#ifdef MY_DEBUG
	rprintf("loop start...\n");
	#endif
	for (;;)
	{
		USB_USBTask();

		int count = 0;
		uint8_t ReceivedData[VENDOR_IO_EPSIZE];
		memset(ReceivedData, 0x00, sizeof(ReceivedData));

		#if 0
		Endpoint_SelectEndpoint(VENDOR_OUT_EPADDR);
		if (Endpoint_IsOUTReceived())
		{
			Endpoint_Read_Stream_LE(ReceivedData, VENDOR_IO_EPSIZE, NULL);
			Endpoint_ClearOUT();

			#ifdef MY_DEBUG
			rprintfStr((char*)ReceivedData); rprintfCRLF();
			#endif

			Endpoint_SelectEndpoint(VENDOR_IN_EPADDR);
			Endpoint_Write_Stream_LE(ReceivedData, VENDOR_IO_EPSIZE, NULL);
			Endpoint_ClearIN();
		}
		#endif
		if ((count = fread(&ReceivedData, 1, VENDOR_IO_EPSIZE, &USBBulkStream)) > 0) {
			#ifdef MY_DEBUG
			for(uint8_t i=0; i<count; i++)
				rprintfChar(ReceivedData[i]);
			rprintfCRLF();
			#endif
			fwrite(&ReceivedData, 1, count, &USBBulkStream);
			Endpoint_ClearIN();
		}

	}
}

// Configures the board hardware and chip peripherals for the demo's functionality.
void SetupHardware(void)
{
	// Disable watchdog if enabled by bootloader/fuses
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disable clock division
	clock_prescale_set(clock_div_1);

	// Hardware Initialization
	LEDs_Init();
	#ifdef MY_DEBUG
	rprintf("before USB_Init...\n");
	#endif
	USB_Init();
	#ifdef MY_DEBUG
	rprintf("after USB_Init...\n");
	#endif
}

// Event handler for the USB_Connect event. This indicates that the device is
// enuerating via the status LEDs.
void EVENT_USB_Device_Connect(void)
{
	#ifdef MY_DEBUG
	rprintf("USB connect...\n");
	#endif
	// Indicate USB enumerating
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

// Event handler for the USB_Disconnect event. This indicates that the device is
// no longer connected to a host via the status LEDs.
void EVENT_USB_Device_Disconnect(void)
{
	#ifdef MY_DEBUG
	rprintf("USB disconnect...\n");
	#endif
	// Indicate USB not ready
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

// Event handler for the USB_ConfigurationChanged event. This is fired when
// the host set the current configuration of the USB device after enumeration
// - the device endpoints are configured.
void EVENT_USB_Device_ConfigurationChanged(void)
{
	#ifdef MY_DEBUG
	rprintf("USB configuration...\n");
	#endif
	bool ConfigSuccess = true;

	// Setup Vendor Data Endpoints
	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_IN_EPADDR, EP_TYPE_BULK, VENDOR_IO_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_OUT_EPADDR, EP_TYPE_BULK, VENDOR_IO_EPSIZE, 1);

	// Indicate endpoint configuration success or failure
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

// Event handler for the USB_ControlRequest event. This is used to catch and
// process control requests sent to the device from the USB host before passing
// along unhandled control requests to the library for processing internally.
void EVENT_USB_Device_ControlRequest(void)
{
	#ifdef MY_DEBUG
	rprintf("USB control request...0x%x, 0x%x\n", USB_ControlRequest.bRequest,
	USB_ControlRequest.bmRequestType);
	#endif
	// Process vendor specific control requests here
}
