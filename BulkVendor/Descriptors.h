#ifndef	DESCRIPTORS_H
#define	DESCRIPTORS_H

// Includes:
#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>

// Macros:
// Endpoint address of the Bulk Vendor device-to-host data IN endpoint.
#define VENDOR_IN_EPADDR	(ENDPOINT_DIR_IN | 3)

// Endpoint address of the Bulk Vendor host-to-device data OUT endpoint.
#define VENDOR_OUT_EPADDR	(ENDPOINT_DIR_OUT | 4)

// Size in bytes of the Bulk Vendor data endpoints.
#define VENDOR_IO_EPSIZE	64

// Type Defines:
// Type define for the device configuration descriptor structure. This must be
// defined in the application code, as the configuration descriptor contains
// serveral sub-descriptors which vary between devices, and which describe
// the device's usage to the host.
typedef struct
{
	USB_Descriptor_Configuration_Header_t Config;

	// Vendor Interface
	USB_Descriptor_Interface_t Vendor_Interface;
	USB_Descriptor_Endpoint_t Vendor_DataInEndpoint;
	USB_Descriptor_Endpoint_t Vendor_DataOutEndpoint;
} USB_Descriptor_Configuration_t;

// Enum for the device interface descriptor IDs whithin the device. Each
// interface descriptor should have a unique ID index associated with it,
// which can be used to refer to the interface from other descriptors.
enum InterfaceDescriptors_t
{
	INTERFACE_ID_Vendor = 0, // Vendor interface descriptor ID
};

// Enum for the device string descriptor IDs within the device. Each string
// descriptor should have a unique ID index associated with it, which can be
// used to refer to the string from other descriptors.
enum StringDescriptors_t
{
	STRING_ID_Language = 0, // Supported Languages string descriptor ID (must be zero)
	STRING_ID_Manufacturer =1, // Manufacturer string ID
	STRING_ID_Product = 2, // Product string ID
};

// Function Prototypes:
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
									const uint8_t wIndex,
									const void** const DescriptorAddress)
									ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
