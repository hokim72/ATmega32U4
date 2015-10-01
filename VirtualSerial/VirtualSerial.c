#include "VirtualSerial.h"
#include "global.h"
#ifdef MY_DEBUG
#include "uart.h"
#include "rprintf.h"
#endif

// Global buffer for use with STDIO functions
volatile char buffer[CDC_TXRX_EPSIZE];

// LUFA CDC Class driver interface configuration and state information. This
// structure is passed to all CDC Class driver functions, so that multiple
// intances of the same class within a device can be differentiated from
// one another.
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
	.Config =
		{
			.ControlInterfaceNumber = 0,
			.DataINEndpoint =
				{
					.Address = CDC_TX_EPADDR,
					.Size = CDC_TXRX_EPSIZE,
					.Banks = 1,
				},
			.DataOUTEndpoint =
				{
					.Address = CDC_RX_EPADDR,
					.Size = CDC_TXRX_EPSIZE,
					.Banks = 1,
				},
			.NotificationEndpoint =
				{
					.Address = CDC_NOTIFICATION_EPADDR,
					.Size = CDC_NOTIFICATION_EPSIZE,
					.Banks = 1,
				},
		},
};

// Standard file stream for the CDC interface when set up, so that the virtual
// CDC COM port can be used like any regular character stream in the C APIs
static FILE USBSerialStream;

int main(void)
{
	#ifdef MY_DEBUG
	uartInit();
	uartSetBaudRate(9600);
	rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
	rprintf("test start....\n");
	uartSendTxBuffer();
	#endif

	SetupHardware();

	// Create a regular character stream for the interface so that it can be used with stdio.h functions.
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	GlobalInterruptEnable();

	for (;;)
	{
		MainTask();

		// Must throw away unused bytes from the host, or it will lock up while
		// waiting for the device
		//CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
		#ifdef MY_DEBUG
		uartSendTxBuffer();
		#endif
	}
}

// Configures the board hardware and chip peripherals for the demo's functionality.
void SetupHardware(void)
{
	// Disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disble clock division
	clock_prescale_set(clock_div_1);

	// Hardware Initialization
	LEDs_Init();
	LEDs_TurnOnLEDs(LEDS_LED1);
	#ifdef MY_DEBUG
	rprintf("before USB_Init...\n");
	#endif
	USB_Init();
	#ifdef MY_DEBUG
	rprintf("before USB_Init...\n");
	#endif
}

// Event handler for the library USB Connection event.
void EVENT_USB_Device_Connect(void)
{
	#ifdef MY_DEBUG
	rprintf("USB connect...\n");
	#endif
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

// Event handler for the library USB Disconnect event.
void EVENT_USB_Device_Disconnect(void)
{
	#ifdef MY_DEBUG
	rprintf("USB disconnect...\n");
	#endif
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

// Event handler for the library USB Configuration Changed event.
void EVENT_USB_Device_ConfigurationChanged(void)
{
	#ifdef MY_DEBUG
	rprintf("USB configuration...\n");
	#endif
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

// Event handler for the library USB Control Request reception event
void EVENT_USB_Device_ControlRequest(void)
{
	#ifdef MY_DEBUG
	rprintf("USB control request...0x%x\n", USB_ControlRequest.bRequest);
	#endif
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void MainTask(void)
{
	int count = 0;

	// If the host has sent data then echo it back
	// Throughput is maxmized if the full EP buffer is read and sent each time
	// Throughput approaches CDC_TXRX_EPSIZE kbytes/second and depends on transfer size from host
	if ((count = fread(&buffer, 1, CDC_TXRX_EPSIZE, &USBSerialStream)) > 0) {
		#ifdef MY_DEBUG
		for(uint8_t i=0; i<count; i++)
			rprintfChar(buffer[i]);
		rprintfCRLF();
		#endif
		fwrite(&buffer, 1, count, &USBSerialStream);
	}
}
