#ifndef LUFAUTIL_H
#define LUFAUTIL_H

#include <LUFA/Drivers/USB/USB.h>
#include <stdio.h>

typedef struct
{
	uint8_t DataINEPAddress; // Data IN endpoint address;
	uint8_t DataOUTEPAddress; // Data OUT endpoint address;
} USB_EPInfo_Device_t;

uint8_t Device_SendByte(USB_EPInfo_Device_t* EPInfo, const uint8_t Data) ATTR_NON_NULL_PTR_ARG(1);
int16_t Device_ReceiveByte(USB_EPInfo_Device_t* const EPInfo) ATTR_NON_NULL_PTR_ARG(1);
void Device_CreateStream(USB_EPInfo_Device_t* const EPInfo, FILE* const Stream) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);
int Device_putchar(char c, FILE* Stream) ATTR_NON_NULL_PTR_ARG(2);
int Device_getchar(FILE* Stream) ATTR_NON_NULL_PTR_ARG(1);

#endif
