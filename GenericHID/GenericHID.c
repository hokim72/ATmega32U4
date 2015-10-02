// Main source file for the GenericHID demo. This file contains the main tasks
// of the demo and is responsible for the initial application hardware configuration.
#include "GenericHID.h"
#include "global.h"
#include "uart.h"
#include "rprintf.h"

// Buffer to hold the previously generated HID report, for comparison purposes
// inside the HID class driver.
static uint8_t PrevHIDReportBuffer[GENERIC_REPORT_SIZE];

// LUFA HID Class driver interface configuration and state information. This
// structure is passed to all HID Class driver functions, so that multiple
// instances of the same class within a device can be differentiated from
// one another.
USB_ClassInfo_HID_Device_t Generic_HID_Interface =
{
	.Config =
		{
			.InterfaceNumber = INTERFACE_ID_GenericHID,
			.ReportINEndpoint =
				{
					.Address = GENERIC_IN_EPADDR,
					.Size = GENERIC_EPSIZE,
					.Banks = 1,
				},
			.PrevReportINBuffer = PrevHIDReportBuffer,
			.PrevReportINBufferSize = sizeof(PrevHIDReportBuffer),
		},
};

// Main program entry point. This routine contains the overall program flow,
// including initial setup of all components and the main program loop.
int main(void)
{
	#ifdef MY_DEBUG
	uartInit();
	uartSetBaudRate(9600);
	rprintfInit(uartSendByte);
	rprintf("test start...\n");
	#endif

	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	#ifdef MY_DEBUG
	rprintf("loop start...\n");
	#endif
	for (;;)
	{
		HID_Device_USBTask(&Generic_HID_Interface);
		USB_USBTask();
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

// Event handler for the library USB Connection event.
void EVENT_USB_Device_Connect(void)
{
	#ifdef MY_DEBUG
	rprintf("USB connect...\n");
	#endif
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

// Event handler for the library USB Disconnection event.
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

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Generic_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

// Event handler for the library USB Control Request reception event.
void EVENT_USB_Device_ControlRequest(void)
{
	#ifdef MY_DEBUG
	rprintf("USB control request...0x%x, 0x%x\n", USB_ControlRequest.bRequest,
	USB_ControlRequest.bmRequestType);
	#endif
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);
}

// Event handler for the USB device Start Of Frame event.
void EVENT_USB_Device_StartOfFrame(void)
{
	#ifdef MY_DEBUG
	//rprintf("USB start of frame...\n");
	#endif
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
}

// HID class driver callback function for the creation of HID reports to the host.
// [in] HIDInterfaceInfo : Pointer to the HID class interface configuration
// structure being referenced.
// [in,out] Report ID : Report ID requested by the host if non-zero, otherwise
// callback should set to the generated report ID.
// [in] ReportType : Type of the report to create, either HID_REPORT_ITEM_In or
// HID_REPORT_ITEM_Feature.
// [out] ReportData : Pointer to a buffer where the created report should be
// stored.
// [out] ReportSize : Number of bytes written in the report (or zero if no
// report is to be sent)
//
// return Boolean true to force the sending of the report, false to let the 
// library determine if it needs to be sent
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
										 uint8_t* const ReportID,
										 const uint8_t ReportType,
										 void* ReportData,
										 uint16_t* const ReportSize)
{
	uint8_t* Data = (uint8_t*)ReportData;
	uint8_t CurrLEDMask = LEDs_GetLEDs();

	Data[0] = ((CurrLEDMask & LEDS_LED1) ? 1 : 0);
	Data[1] = ((CurrLEDMask & LEDS_LED2) ? 1 : 0);
	Data[2] = ((CurrLEDMask & LEDS_LED3) ? 1 : 0);
	Data[3] = ((CurrLEDMask & LEDS_LED4) ? 1 : 0);

	#ifdef MY_DEBUG
	rprintf("create HID report...0x%x : 0x%x 0x%x 0x%x 0x%x\n", 
	ReportID, Data[0], Data[1], Data[2], Data[3]);
	#endif

	*ReportSize = GENERIC_REPORT_SIZE;
	return false;
}

// HID class driver callback function for the processing of HID reports from
// the host.
// [in] HIDInterfaceInfo : Pointer to the HID class interface configuration
// structure being referenced.
// [in] ReportID : Report ID of the received report from the host
// [in] ReportType : The type of report that the host has sent, either
// HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
// [in] ReportData : Pointer to a buffer where the received report has been
// stored.
// [in] ReportSize : Size in bytes of the received HID report
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
										  const uint8_t ReportID,
										  const uint8_t ReportType,
										  const void* ReportData,
										  const uint16_t ReportSize)
{
	uint8_t* Data = (uint8_t*)ReportData;
	uint8_t NewLEDMask = LEDS_NO_LEDS;

	if (Data[0])
		NewLEDMask |= LEDS_LED1;

	if (Data[1])
		NewLEDMask |= LEDS_LED2;

	if (Data[2])
		NewLEDMask |= LEDS_LED3;

	if (Data[3])
		NewLEDMask |= LEDS_LED4;

	LEDs_SetAllLEDs(NewLEDMask);
	#ifdef MY_DEBUG
	rprintf("process HID report...0x%x : 0x%x 0x%x 0x%x 0x%x\n", 
	ReportID, Data[0], Data[1], Data[2], Data[3]);
	#endif
}
