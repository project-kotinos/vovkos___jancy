//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_io_UsbDesc.h"
#include "jnc_io_UsbLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	UsbEndpointDesc,
	"io.UsbEndpointDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpointDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbEndpointDesc)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	UsbInterfaceDesc,
	"io.UsbInterfaceDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbInterfaceDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbInterfaceDesc)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	UsbConfigurationDesc,
	"io.UsbConfigurationDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbConfigurationDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbConfigurationDesc)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	UsbDeviceDesc,
	"io.UsbDeviceDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDeviceDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
initUsbEndpointDesc (
	UsbEndpointDesc* dstDesc,
	const libusb_endpoint_descriptor* srcDesc
	)
{
	dstDesc->m_endpointId = srcDesc->bEndpointAddress;
	dstDesc->m_transferType = (srcDesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK);
	dstDesc->m_isoSyncType = (srcDesc->bmAttributes & LIBUSB_ISO_SYNC_TYPE_MASK);
	dstDesc->m_isoUsage = (srcDesc->bmAttributes & LIBUSB_ISO_USAGE_TYPE_MASK);
	dstDesc->m_maxPacketSize = srcDesc->wMaxPacketSize;
	dstDesc->m_interval = srcDesc->bInterval;
	dstDesc->m_refresh = srcDesc->bRefresh;
	dstDesc->m_synchAddress = srcDesc->bSynchAddress;
}

//..............................................................................

UsbEndpointDesc*
UsbInterfaceDesc::findEndpointDesc (uint8_t endpointId)
{
	DataPtr* ptrArray = (DataPtr*) m_endpointTable.m_p;
	for (size_t i = 0; i < m_endpointCount; i++)
	{
		UsbEndpointDesc* endpointDesc = (UsbEndpointDesc*) ptrArray [i].m_p;
		if (endpointDesc->m_endpointId == endpointId)
			return endpointDesc;
	}

	return NULL;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbInterfaceDesc (
	Runtime* runtime,
	UsbInterfaceDesc* dstDesc,
	libusb_interface_descriptor* srcDesc
	)
{
	Type* endpointDescType = UsbEndpointDesc::getType (runtime->getModule ());

	dstDesc->m_nextAlternatePtr = g_nullPtr;
	dstDesc->m_endpointTable = runtime->getGcHeap ()->allocateArray (endpointDescType, srcDesc->bNumEndpoints);
	dstDesc->m_endpointCount = srcDesc->bNumEndpoints;

	UsbEndpointDesc* dstEndpointDescTable = (UsbEndpointDesc*) dstDesc->m_endpointTable.m_p;
	for (size_t i = 0; i < srcDesc->bNumEndpoints; i++)
		initUsbEndpointDesc (&dstEndpointDescTable [i], &srcDesc->endpoint [i]);

	dstDesc->m_interfaceId = srcDesc->bInterfaceNumber;
	dstDesc->m_altSettingId = srcDesc->bAlternateSetting;
	dstDesc->m_descriptionStringId = srcDesc->iInterface;
	dstDesc->m_class = srcDesc->bInterfaceClass;
	dstDesc->m_subClass = srcDesc->bInterfaceSubClass;
	dstDesc->m_protocol = srcDesc->bInterfaceProtocol;
}

void
initUsbInterfaceDesc (
	Runtime* runtime,
	UsbInterfaceDesc* dstDesc,
	libusb_interface* srcDesc
	)
{
	if (!srcDesc->num_altsetting)
		return;

	JNC_BEGIN_CALL_SITE (runtime)

	initUsbInterfaceDesc (runtime, dstDesc, &srcDesc->altsetting [0]);

	UsbInterfaceDesc* prevDesc = dstDesc;
	for (size_t i = 1; i < srcDesc->num_altsetting; i++)
	{
		DataPtr descPtr = createData <UsbInterfaceDesc> (runtime);
		dstDesc = (UsbInterfaceDesc*) descPtr.m_p;
		initUsbInterfaceDesc (runtime, dstDesc, &srcDesc->altsetting [i]);

		prevDesc->m_nextAlternatePtr = descPtr;
		prevDesc = dstDesc;
	}

	JNC_END_CALL_SITE ()
}

//..............................................................................

UsbInterfaceDesc*
UsbConfigurationDesc::findInterfaceDesc (
	uint8_t interfaceId,
	uint8_t altSettingId
	)
{
	if (interfaceId >= m_interfaceCount)
		return NULL;

	DataPtr ifaceDescPtr = ((DataPtr*) m_interfaceTable.m_p) [interfaceId];
	UsbInterfaceDesc* ifaceDesc = (UsbInterfaceDesc*) ifaceDescPtr.m_p;
	for (size_t i = 0; i < altSettingId; i++)
	{
		if (!ifaceDesc->m_nextAlternatePtr.m_p)
			return NULL;

		ifaceDesc = (UsbInterfaceDesc*) ifaceDesc->m_nextAlternatePtr.m_p;
	}

	return ifaceDesc;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbConfigurationDesc (
	Runtime* runtime,
	UsbConfigurationDesc* dstDesc,
	libusb_config_descriptor* srcDesc
	)
{
	JNC_BEGIN_CALL_SITE (runtime)

	Type* ifaceDescType = UsbInterfaceDesc::getType (runtime->getModule ());
	dstDesc->m_interfaceTable = runtime->getGcHeap ()->allocateArray (ifaceDescType, srcDesc->bNumInterfaces);
	dstDesc->m_interfaceCount = srcDesc->bNumInterfaces;

	UsbInterfaceDesc* dstInterfaceDescTable = (UsbInterfaceDesc*) dstDesc->m_interfaceTable.m_p;
	for (size_t i = 0; i < srcDesc->bNumInterfaces; i++)
		initUsbInterfaceDesc (runtime, &dstInterfaceDescTable [i], &srcDesc->interface [i]);

	dstDesc->m_configurationId = srcDesc->bConfigurationValue;
	dstDesc->m_descriptionStringId = srcDesc->iConfiguration;
	dstDesc->m_attributes = srcDesc->bmAttributes;
	dstDesc->m_maxPower = srcDesc->MaxPower;

	JNC_END_CALL_SITE ()
}

DataPtr
createUsbConfigurationDesc (
	Runtime* runtime,
	const libusb_config_descriptor* srcDesc
	)
{
	DataPtr resultPtr = g_nullPtr;

	JNC_BEGIN_CALL_SITE (runtime)

	resultPtr = createData <UsbConfigurationDesc> (runtime);
	UsbConfigurationDesc* dstDesc = (UsbConfigurationDesc*) resultPtr.m_p;
	initUsbConfigurationDesc (runtime, dstDesc, srcDesc);

	JNC_END_CALL_SITE ()

	return resultPtr;
}

//..............................................................................

DataPtr
createUsbDeviceDesc (
	Runtime* runtime,
	libusb_device_descriptor* srcDesc,
	axl::io::UsbDevice* srcDevice
	)
{
	DataPtr resultPtr = g_nullPtr;

	JNC_BEGIN_CALL_SITE (runtime)

	resultPtr = createData <UsbDeviceDesc> (runtime);
	UsbDeviceDesc* deviceDesc = (UsbDeviceDesc*) resultPtr.m_p;

	Type* configDescType = UsbConfigurationDesc::getType (runtime->getModule ());
	deviceDesc->m_configurationTable = runtime->getGcHeap ()->allocateArray (configDescType, srcDesc->bNumConfigurations);
	deviceDesc->m_configurationCount = srcDesc->bNumConfigurations;

	UsbConfigurationDesc* dstConfigDescTable = (UsbConfigurationDesc*) deviceDesc->m_configurationTable.m_p;
	for (size_t i = 0; i < srcDesc->bNumConfigurations; i++)
	{
		axl::io::UsbConfigDescriptor srcConfigDesc;
		srcDevice->getConfigDescriptor (i, &srcConfigDesc);
		initUsbConfigurationDesc (runtime, &dstConfigDescTable [i], srcConfigDesc);
	}

	deviceDesc->m_usbVersion = srcDesc->bcdUSB;
	deviceDesc->m_deviceVersion = srcDesc->bcdDevice;
	deviceDesc->m_vendorId = srcDesc->idVendor;
	deviceDesc->m_productId = srcDesc->idProduct;
	deviceDesc->m_vendorStringId = srcDesc->iManufacturer;
	deviceDesc->m_productStringId = srcDesc->iProduct;
	deviceDesc->m_serialStringId = srcDesc->iSerialNumber;
	deviceDesc->m_class = srcDesc->bDeviceClass;
	deviceDesc->m_subClass = srcDesc->bDeviceSubClass;
	deviceDesc->m_protocol = srcDesc->bDeviceProtocol;

	JNC_END_CALL_SITE ()

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
