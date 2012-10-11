
#ifndef USB_HID_DEVICE_H
#define USB_HID_DEVICE_H

#include "HIDUtilities.h"
#include "MWorksCore/LegacyIODevice.h"

using namespace mw;

class mUSBHID : public LegacyIODevice {
	
protected:
	
	IOHIDDeviceInterface ** pphidDeviceInterface;
	io_iterator_t hidObjectIterator;
	mach_port_t masterPort;
	io_object_t hidDevice;
	
	void registerHIDProperties(CFTypeRef object);
	void registerHID_CFDictionary(CFDictionaryRef object);
	void registerHID_CFArray(CFArrayRef object);
	void registerHID_CFArrayApplier (const void * value, void * parameter);
	
public:
	
	
	mUSBHID();						// initialize with the first available HID device	
	~mUSBHID();
	
	// start acquiring
	virtual bool updateChannel(int channel_number);
	virtual bool attachPhysicalDevice();
	virtual bool startDeviceIO();
	
	shared_ptr<mUSBHID> shared_from_this() { return boost::dynamic_pointer_cast<mUSBHID>(IODevice::shared_from_this()); }
};


#endif
