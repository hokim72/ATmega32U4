#include "LufaUtil.h"

uint8_t Device_SendByte(USB_EPInfo_Device_t* EPInfo, const uint8_t Data)
{
	if (USB_DeviceState != DEVICE_STATE_Configured) return ENDPOINT_RWSTREAM_DeviceDisconnected;
	// USB_DeviceState 
	// USBTask.h
	// Indicates the current device state machine state. When in device mode, 
	// this indicates the state via one of the values of the 
	// USB_Device_States_t enum values.
	//
	// DEVICE_STATE_Configured(4)
	// Device.h
	// This state indicates that the device has been enumerated by the host 
	// and is ready for USB communications to begin.
	//
	// ENDPOINT_RWSTREAM_DeviceDisconnected(2)
	// EndpointStream.h
	// Device was disconnected from the host during the transfer.
	
	Endpoint_SelectEndpoint(EPInfo->DataINEPAddress);
	// Endpoint_AVR8.h
	// Select the given endpoint address.
	if (!(Endpoint_IsReadWriteAllowed()))
	// Endpoint_AVR8.h
	// Determines if the currently selected endpoint may be read from 
	// (if data is waiting in the endpoint bank and the endpoint is an OUT 
	// direction, or if the bank is not yet full if the endpoint is an IN 
	// direction). This function will return false if an error has occurred in 
	// the endpoint, if the endpoint is an OUT direction and no packet (or an 
	// empty packet) has been received, or if the endpoint is an IN direction 
	// and the endpoint bank is full.
	{
		Endpoint_ClearIN();
		// Endpoint_AVR8.h
		// Sends an IN packet to the host on the currently selected endpoint, freeing up the endpoint for the next packet and switching to the alternative endpoint bank if double banked
		
		uint8_t ErrorCode;

		if ((ErrorCode = Endpoint_WaitUntilReady()) != ENDPOINT_READYWAIT_NoError)
		// Endpoint_WaitUntilReady
		// Endpoint_AVR8.h
		// Spin-loops until the currently selected non-control endpoint is 
		// ready for the next packet of data to be read or written to it.
		//
		// ENDPOINT_READYWAIT_NoError(0)
		// Endpoint_AVR8.h
		// Endpoint is ready for next packet, no error.
			return ErrorCode;
	}

	Endpoint_Write_8(Data);
	// Endpoint_AVR8.h
	// Writes one byte to the currently selected endpoint's bank, for IN direction endpoints.
	return ENDPOINT_READYWAIT_NoError;
}

int16_t Device_ReceiveByte(USB_EPInfo_Device_t* const EPInfo)
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return -1;

	int16_t ReceivedByte = -1;

	Endpoint_SelectEndpoint(EPInfo->DataOUTEPAddress);

	if (Endpoint_IsOUTReceived())
	// Endpoint_AVR8.h
	// Determines if the selected OUT endpoint has received new packet from 
	// the host.
	{
		if (Endpoint_BytesInEndpoint())
		// Endpoint_AVR8.h
		// Indicates the number of bytes currently stored in the current 
		// endpoint's selected bank.
			ReceivedByte = Endpoint_Read_8();
			// Endpoint_AVR8.h
			// Reads one byte from the currently selected endpoint's bank, 
			// for OUT direction endpoints.
		if (!(Endpoint_BytesInEndpoint()))
			Endpoint_ClearOUT();
			// Endpoint_AVR8.h
			// Acknowledges an OUT packet to the host on the currently selected
			// endpoint, freeing up the endpoint for the next packet and 
			// switching to the alternative endpoint bank if double banked.
	}

	return ReceivedByte;

}

void Device_CreateStream(USB_EPInfo_Device_t* const EPInfo, FILE* const Stream)
{
	*Stream = (FILE)FDEV_SETUP_STREAM(Device_putchar, Device_getchar, _FDEV_SETUP_RW);
	fdev_set_udata(Stream, EPInfo);
}

int Device_putchar(char c, FILE* Stream)
{
	return Device_SendByte((USB_EPInfo_Device_t*)fdev_get_udata(Stream), c) ? _FDEV_ERR : 0;
}

int Device_getchar(FILE* Stream)
{
	int16_t ReceivedByte = Device_ReceiveByte((USB_EPInfo_Device_t*)fdev_get_udata(Stream));

	if (ReceivedByte < 0)
		return _FDEV_EOF;

	return ReceivedByte;
}

