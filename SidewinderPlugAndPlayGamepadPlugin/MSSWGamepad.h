
#ifndef MSSW_GAMEPAD_DEVICE_H
#define MSSW_GAMEPAD_DEVICE_H

#include "MonkeyWorksCore/IODevice.h"

#include "HIDUtilities.h"
#include "ButtonMap.h"
using namespace mw;

enum mMSSWGamepadDataType {
	M_EVENT_DRIVEN,
	M_CONTINUOUS,
	M_UNKNOWN_DATA_TYPE
};


class mMSSWGamepad : public IODevice {
	
protected:
	boost::shared_ptr <Scheduler> scheduler;
	
	boost::shared_ptr <Variable> A;
	boost::shared_ptr <Variable> B;
	boost::shared_ptr <Variable> X;
	boost::shared_ptr <Variable> Y;
	boost::shared_ptr <Variable> TL;
	boost::shared_ptr <Variable> TR;
	boost::shared_ptr <Variable> U;
	boost::shared_ptr <Variable> D;
	boost::shared_ptr <Variable> L;
	boost::shared_ptr <Variable> R;
	MonkeyWorksTime update_period;
	mMSSWGamepadDataType data_type;
	
	std::vector<mButtonMap> button_ids;
	
	IOHIDDeviceInterface ** pphidDeviceInterface;
	io_iterator_t hidObjectIterator;
	mach_port_t masterPort;
	io_object_t hidDevice;
	
	void registerHIDProperties(CFTypeRef object);
	void registerHID_CFDictionary(CFDictionaryRef object);
	void registerHID_CFArray(CFArrayRef object);
	void registerHID_CFArrayApplier (const void * value, void * parameter);
	
	virtual bool updateButtonsDiscrete();
	virtual bool updateButtonsContinuous();
	
public:
	
	
	mMSSWGamepad(const boost::shared_ptr <Scheduler> &a_scheduler,
				 const mMSSWGamepadDataType data_type,
				 const boost::shared_ptr <Variable> button_A,
				 const boost::shared_ptr <Variable> button_B,
				 const boost::shared_ptr <Variable> button_X,
				 const boost::shared_ptr <Variable> button_Y,
				 const boost::shared_ptr <Variable> button_TL,
				 const boost::shared_ptr <Variable> button_TR,
				 const boost::shared_ptr <Variable> pad_U,
				 const boost::shared_ptr <Variable> pad_D,
				 const boost::shared_ptr <Variable> pad_L,
				 const boost::shared_ptr <Variable> pad_R,
				 const MonkeyWorksTime update_time);						// initialize with the first available HID device	
	~mMSSWGamepad();
	
	// start acquiring
	virtual bool startup();
	virtual bool updateButtons();
	virtual bool shutdown();	
	virtual bool attachPhysicalDevice();
	virtual bool startDeviceIO();
	
	virtual void addChild(std::map<std::string, std::string> parameters,
						  mw::mwComponentRegistry *reg,
						  shared_ptr<mw::Component> child);
	
	shared_ptr<mMSSWGamepad> shared_from_this() { return static_pointer_cast<mMSSWGamepad>(IODevice::shared_from_this()); }
};


#endif
