
#include "MSSWGamepad.h"
#include "boost/bind.hpp"

// External function for scheduling
void *update_buttons(const shared_ptr<mMSSWGamepad> &gp){
	gp->updateButtons();                 
	return NULL;
}



mMSSWGamepad::mMSSWGamepad(const boost::shared_ptr <Scheduler> &a_scheduler,
						   const mMSSWGamepadDataType data_type_,
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
						   const MonkeyWorksTime update_time) {
	scheduler = a_scheduler;
	
	A = button_A;
	B = button_B;
	X = button_X;
	Y = button_Y;
	TL = button_TL;
	TR = button_TR;
	U = pad_U;
	D = pad_D;
	L = pad_L;
	R = pad_R;
	
	update_period = update_time;
	data_type = data_type_;
	
	// Create and initialize an HID Device Interface
	
	masterPort = NULL;
	hidObjectIterator = NULL;
	IOReturn ioReturnValue = kIOReturnSuccess;
	
	// Get a Mach port to initiate communication with I/O Kit.
	ioReturnValue = IOMasterPort (bootstrap_port, &masterPort);
	//PrintErrMsgIfIOErr (ioReturnValue, "Couldn’t create a master I/O Kit Port.");
	
	printf("\n\n--- Finding GamePads ---\n");
	hidObjectIterator = MyFindHIDDevices (masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
	if (hidObjectIterator != 0){
		hidDevice = NULL;
		pphidDeviceInterface = NULL;
		IOReturn ioReturnValue = kIOReturnSuccess;
		short numDevice = 0;
		
		hidDevice = IOIteratorNext (hidObjectIterator);
		
		printf ("\n--- Device %d ---\n", numDevice++);
		MyShowHIDProperties (hidDevice);
		pphidDeviceInterface = MyCreateHIDDeviceInterface (hidDevice);
		
		// Release the device
		ioReturnValue = IOObjectRelease (hidDevice);
		//PrintErrMsgIfIOErr (ioReturnValue, "Error releasing HID device");
		
		if (*pphidDeviceInterface != NULL){
			// open the device
			ioReturnValue = (*pphidDeviceInterface)->open (pphidDeviceInterface, 0);
			printf ("Open result = %d\n", ioReturnValue);
			
		} else {
			throw SimpleException("Error creating HID device interface");
		}
		
	} else {
		throw SimpleException("Error creating HID device interface");
	}
	
	//////////////////////////////////////////////
	// Register the capabilities of this device
	/////////////////////////////////////////////
	
	kern_return_t result;
	CFMutableDictionaryRef properties = 0;
	
	//Create a CF dictionary representation of the I/O Registry entry’s properties
	result = IORegistryEntryCreateCFProperties (hidDevice, &properties, kCFAllocatorDefault, kNilOptions);
	if ((result == KERN_SUCCESS) && properties){
		
		
		CFTypeRef object = CFDictionaryGetValue (properties, CFSTR(kIOHIDElementKey));
		registerHIDProperties(object);
		
		//Release the properties dictionary
		CFRelease (properties);
		
	} else {
		throw SimpleException("Failed to create properties via IORegistryEntryCreateCFProperties.");
	}
	
	
}

mMSSWGamepad::~mMSSWGamepad(){
	
	IOReturn ioReturnValue = kIOReturnSuccess;
	
	ioReturnValue = (*pphidDeviceInterface)->close (pphidDeviceInterface);
	
	// Release the device interface
	(*pphidDeviceInterface)->Release (pphidDeviceInterface);
	
	IOObjectRelease (hidObjectIterator);
	
	//Free master port if we created one.
	if (masterPort)
		mach_port_deallocate (mach_task_self (), masterPort);
}


void mMSSWGamepad::registerHIDProperties(CFTypeRef object){
	CFTypeID type = CFGetTypeID(object);
	if (type == CFArrayGetTypeID()){
		registerHID_CFArray((const __CFArray*)object);
	} else if (type == CFDictionaryGetTypeID()){
		registerHID_CFDictionary((const __CFDictionary*)object);
	} else if (type == CFBooleanGetTypeID()){
		//MyCFBoolean(object);
	} else if (type == CFNumberGetTypeID()){
		//MyCFNumberShow(object);
	} else if (type == CFStringGetTypeID()){
		//MyCFStringShow(object);
	} else{
		//mprintf("<unknown hid object>");
	}
}

void mMSSWGamepad::registerHID_CFDictionary(CFDictionaryRef object)
{
	// Capability identifier = HID cookie
	CFTypeRef identifier_object = CFDictionaryGetValue (object, CFSTR(kIOHIDElementCookieKey));
	
	if(!identifier_object){
		throw SimpleException("Error registering HID device capabilities");
	}
	
	long capability_identifier;
	CFNumberGetValue((const __CFNumber*)identifier_object, kCFNumberLongType, &capability_identifier);
	
	// Get capability name
	CFStringRef keyUsagePage = CFSTR(kIOHIDElementUsagePageKey);
	CFStringRef keyUsage = CFSTR(kIOHIDElementUsageKey);
	CFTypeRef objectUsagePage = CFDictionaryGetValue (object, keyUsagePage);
	CFTypeRef objectUsage = CFDictionaryGetValue (object, keyUsage);
	long valueUsage;
	CFNumberGetValue ((const __CFNumber*)objectUsage, kCFNumberLongType, &valueUsage);
	
	char *usagePageString, *usageString;
	hidUsageLookup(objectUsagePage, objectUsage, &usagePageString, &usageString);
	char *capability_name = usageString;
	
	char *capability_hid_type;
	CFStringRef keyType = CFSTR(kIOHIDElementTypeKey);
	CFTypeRef objectType = CFDictionaryGetValue (object, keyType);
	hidTypeLookup(objectType, &capability_hid_type);
	
	
	if(strcmp(capability_hid_type, "Collection") == 0){
		//mprintf("Registering collection of capabilities (%s, %s)", usagePageString, capability_name);
		CFStringRef keyElements = CFSTR(kIOHIDElementKey);
		CFTypeRef subelements = CFDictionaryGetValue (object, keyElements);
		registerHIDProperties(subelements); // recurse
	}
	else {
		std::string name(capability_name);		
		button_ids.push_back(mButtonMap(capability_identifier, name));
	}
}



void mMSSWGamepad::registerHID_CFArray(CFArrayRef object)
{
	
	for(int i = 0; i < CFArrayGetCount(object); i++){
		registerHIDProperties(CFArrayGetValueAtIndex(object, i));
	}
}

// --------------------------------------------------------------------------

void mMSSWGamepad::registerHID_CFArrayApplier (const void * value, void * parameter)
{
	if (CFGetTypeID (value) != CFDictionaryGetTypeID ())
		return;
	registerHIDProperties(value);
}


// initialize the device (pre- channel creation/initialization
bool mMSSWGamepad::startup(){
	// no special action required  
	return true;
}


// this could all be a bit more efficient, but it's much clearer what is happening here
bool mMSSWGamepad::updateButtons() {	
	switch(data_type) {
		case M_EVENT_DRIVEN:
			return this->updateButtonsDiscrete();
			break;
		case M_CONTINUOUS:
			return this->updateButtonsContinuous();
			break;
		default:
			return false;
	}
}


bool mMSSWGamepad::updateButtonsDiscrete() {	
	for(std::vector<mButtonMap>::const_iterator i=button_ids.begin();
		i != button_ids.end();
		++i) {
		IOHIDEventStruct hidEvent;
		(*pphidDeviceInterface)->getElementValue(pphidDeviceInterface, (void *)i->getCapabilityID(), &hidEvent);
		
		long value = (long)hidEvent.value;
		
		
		if(i->getName() == "X-Axis") {
			if(value == 63) {
				if(L->getValue().getInteger() != 0) {
					L->setValue(Data(0L));
				}				
				if(R->getValue().getInteger() != 0) {
					R->setValue(Data(0L));
				}				
			}
			
			if(value < 63) {
				if(L->getValue().getInteger() != 1) {
					L->setValue(Data(1L));
				}				
				if(R->getValue().getInteger() != 0) {
					R->setValue(Data(0L));
				}				
			}
			
			if(value > 63) {
				if(L->getValue().getInteger() != 0) {
					L->setValue(Data(0L));
				}				
				if(R->getValue().getInteger() != 1) {
					R->setValue(Data(1L));
				}				
			}
		} else if (i->getName() == "Y-Axis") {			
			if(value == 63) {
				if(D->getValue().getInteger() != 0) {
					D->setValue(Data(0L));
				}				
				if(U->getValue().getInteger() != 0) {
					U->setValue(Data(0L));
				}				
			}
			
			if(value < 63) {
				if(D->getValue().getInteger() != 1) {
					D->setValue(Data(1L));
				}				
				if(U->getValue().getInteger() != 0) {
					U->setValue(Data(0L));
				}				
			}
			
			if(value > 63) {
				if(D->getValue().getInteger() != 0) {
					D->setValue(Data(0L));
				}				
				if(U->getValue().getInteger() != 1) {
					U->setValue(Data(1L));
				}				
			}
		} else if (i->getName() == "Button_1") {
			if(A->getValue().getInteger() != hidEvent.value) {
				A->setValue(Data((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_2") {
			if(B->getValue().getInteger() != hidEvent.value) {
				B->setValue(Data((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_3") {
			if(X->getValue().getInteger() != hidEvent.value) {
				X->setValue(Data((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_4") {
			if(Y->getValue().getInteger() != hidEvent.value) {
				Y->setValue(Data((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_5") {
			if(TR->getValue().getInteger() != hidEvent.value) {
				TR->setValue(Data((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_6") {
			if(TL->getValue().getInteger() != hidEvent.value) {
				TL->setValue(Data((long)hidEvent.value));
			}			
		}
	}
	
	return true;
}

bool mMSSWGamepad::updateButtonsContinuous() {	
	for(std::vector<mButtonMap>::const_iterator i=button_ids.begin();
		i != button_ids.end();
		++i) {
		IOHIDEventStruct hidEvent;
		(*pphidDeviceInterface)->getElementValue(pphidDeviceInterface, (void *)i->getCapabilityID(), &hidEvent);
		
		long value = (long)hidEvent.value;
		
		
		if(i->getName() == "X-Axis") {
			if(value == 63) {
				L->setValue(Data(0L));
				R->setValue(Data(0L));
			}
			
			if(value < 63) {
				L->setValue(Data(1L));
				R->setValue(Data(0L));
			}
			
			if(value > 63) {
				L->setValue(Data(0L));
				R->setValue(Data(1L));
			}
		} else if (i->getName() == "Y-Axis") {			
			if(value == 63) {
				D->setValue(Data(0L));
				U->setValue(Data(0L));
			}
			
			if(value < 63) {
				D->setValue(Data(1L));
				U->setValue(Data(0L));
			}
			
			if(value > 63) {
				D->setValue(Data(0L));
				U->setValue(Data(1L));
			}
		} else if (i->getName() == "Button_1") {
			A->setValue(Data((long)hidEvent.value));
		} else if (i->getName() == "Button_2") {
			B->setValue(Data((long)hidEvent.value));
		} else if (i->getName() == "Button_3") {
			X->setValue(Data((long)hidEvent.value));
		} else if (i->getName() == "Button_4") {
			Y->setValue(Data((long)hidEvent.value));
		} else if (i->getName() == "Button_5") {
			TR->setValue(Data((long)hidEvent.value));
		} else if (i->getName() == "Button_6") {
			TL->setValue(Data((long)hidEvent.value));
		}
	}
	
	return true;
}

bool mMSSWGamepad::attachPhysicalDevice(){                                      // attach next avaialble device to this object
	
	// bypass all of this bullshit for now--
	attached_device = new IOPhysicalDeviceReference(0, "gamepad");
	
	return true;
}


bool mMSSWGamepad::shutdown(){
	// no special action required
	return true;
}

bool mMSSWGamepad::startDeviceIO(){
	schedule_nodes_lock.lock();
	
	shared_ptr<mMSSWGamepad> this_one = shared_from_this();
	shared_ptr<ScheduleTask> node = scheduler->scheduleUS(std::string(FILELINE ": ") + tag,
														   (MonkeyWorksTime)0, 
														   update_period, 
														   M_REPEAT_INDEFINITELY, 
														   boost::bind(update_buttons, 
																	   this_one),
														   M_DEFAULT_IODEVICE_PRIORITY,
														   M_DEFAULT_IODEVICE_WARN_SLOP_US,
														   M_DEFAULT_IODEVICE_FAIL_SLOP_US,
														   M_MISSED_EXECUTION_DROP);
	schedule_nodes.push_back(node);       
	schedule_nodes_lock.unlock();
	
	return true;
}

void mMSSWGamepad::addChild(std::map<std::string, std::string> parameters,
							mw::mwComponentRegistry *reg,
							shared_ptr<mw::Component> child){}


