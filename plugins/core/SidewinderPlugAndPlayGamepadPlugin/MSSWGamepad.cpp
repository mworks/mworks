
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
						   const MWorksTime update_time) {
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
	
	//printf("\n\n--- Finding GamePads ---\n");
	hidObjectIterator = MyFindHIDDevices (masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
  
  if(hidObjectIterator == 0){
      hidObjectIterator = MyFindHIDDevices (masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
  }
  
	if (hidObjectIterator != 0){
		hidDevice = NULL;
		pphidDeviceInterface = NULL;
		IOReturn ioReturnValue = kIOReturnSuccess;
		//short numDevice = 0;
		
		hidDevice = IOIteratorNext (hidObjectIterator);
		
		//printf ("\n--- Device %d ---\n", numDevice++);
		MyShowHIDProperties (hidDevice);
		pphidDeviceInterface = MyCreateHIDDeviceInterface (hidDevice);
		
		// Release the device
		ioReturnValue = IOObjectRelease (hidDevice);
		//PrintErrMsgIfIOErr (ioReturnValue, "Error releasing HID device");
		
		if (*pphidDeviceInterface != NULL){
			// open the device
			ioReturnValue = (*pphidDeviceInterface)->open (pphidDeviceInterface, 0);
			//printf ("Open result = %d\n", ioReturnValue);
			
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
        
        CFNumberRef objectMin = (const __CFNumber*)CFDictionaryGetValue(object, CFSTR(kIOHIDElementMinKey));
        long valueMin;
        bool valueMinOK = CFNumberGetValue(objectMin, kCFNumberLongType, &valueMin);
        
        CFNumberRef objectMax = (const __CFNumber*)CFDictionaryGetValue(object, CFSTR(kIOHIDElementMaxKey));
        long valueMax;
        bool valueMaxOK = CFNumberGetValue(objectMax, kCFNumberLongType, &valueMax);
        
        if (!valueMinOK || !valueMaxOK) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  "Cannot determine value range for HID capability",
                                  name);
        }
        
		button_ids.push_back(ButtonMap(capability_identifier, name, valueMin, valueMax));
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
	for(std::vector<ButtonMap>::const_iterator i=button_ids.begin();
		i != button_ids.end();
		++i) {
		IOHIDEventStruct hidEvent;
		(*pphidDeviceInterface)->getElementValue(pphidDeviceInterface, (IOHIDElementCookie)i->getCapabilityID(), &hidEvent);
		
		long value = (long)hidEvent.value;
		
		
		if(i->getName() == "X-Axis") {
			if(value == i->getMeanValue()) {
				if(L->getValue().getInteger() != 0) {
					L->setValue(Datum(0L));
				}				
				if(R->getValue().getInteger() != 0) {
					R->setValue(Datum(0L));
				}				
			}
			
			if(value < i->getMeanValue()) {
				if(L->getValue().getInteger() != 1) {
					L->setValue(Datum(1L));
				}				
				if(R->getValue().getInteger() != 0) {
					R->setValue(Datum(0L));
				}				
			}
			
			if(value > i->getMeanValue()) {
				if(L->getValue().getInteger() != 0) {
					L->setValue(Datum(0L));
				}				
				if(R->getValue().getInteger() != 1) {
					R->setValue(Datum(1L));
				}				
			}
		} else if (i->getName() == "Y-Axis") {			
			if(value == i->getMeanValue()) {
				if(D->getValue().getInteger() != 0) {
					D->setValue(Datum(0L));
				}				
				if(U->getValue().getInteger() != 0) {
					U->setValue(Datum(0L));
				}				
			}
			
			if(value < i->getMeanValue()) {
				if(D->getValue().getInteger() != 0) {
					D->setValue(Datum(0L));
				}				
				if(U->getValue().getInteger() != 1) {
					U->setValue(Datum(1L));
				}				
			}
			
			if(value > i->getMeanValue()) {
				if(D->getValue().getInteger() != 1) {
					D->setValue(Datum(1L));
				}				
				if(U->getValue().getInteger() != 0) {
					U->setValue(Datum(0L));
				}				
			}
		} else if (i->getName() == "Button_1") {
			if(A->getValue().getInteger() != hidEvent.value) {
				A->setValue(Datum((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_2") {
			if(B->getValue().getInteger() != hidEvent.value) {
				B->setValue(Datum((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_3") {
			if(X->getValue().getInteger() != hidEvent.value) {
				X->setValue(Datum((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_4") {
			if(Y->getValue().getInteger() != hidEvent.value) {
				Y->setValue(Datum((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_5") {
			if(TL->getValue().getInteger() != hidEvent.value) {
				TL->setValue(Datum((long)hidEvent.value));
			}			
		} else if (i->getName() == "Button_6") {
			if(TR->getValue().getInteger() != hidEvent.value) {
				TR->setValue(Datum((long)hidEvent.value));
			}			
		}
	}
	
	return true;
}

bool mMSSWGamepad::updateButtonsContinuous() {	
	for(std::vector<ButtonMap>::const_iterator i=button_ids.begin();
		i != button_ids.end();
		++i) {
		IOHIDEventStruct hidEvent;
		(*pphidDeviceInterface)->getElementValue(pphidDeviceInterface, (IOHIDElementCookie)i->getCapabilityID(), &hidEvent);
		
		long value = (long)hidEvent.value;
		
		
		if(i->getName() == "X-Axis") {
			if(value == i->getMeanValue()) {
				L->setValue(Datum(0L));
				R->setValue(Datum(0L));
			}
			
			if(value < i->getMeanValue()) {
				L->setValue(Datum(1L));
				R->setValue(Datum(0L));
			}
			
			if(value > i->getMeanValue()) {
				L->setValue(Datum(0L));
				R->setValue(Datum(1L));
			}
		} else if (i->getName() == "Y-Axis") {			
			if(value == i->getMeanValue()) {
				D->setValue(Datum(0L));
				U->setValue(Datum(0L));
			}
			
			if(value < i->getMeanValue()) {
				D->setValue(Datum(0L));
				U->setValue(Datum(1L));
			}
			
			if(value > i->getMeanValue()) {
				D->setValue(Datum(1L));
				U->setValue(Datum(0L));
			}
		} else if (i->getName() == "Button_1") {
			A->setValue(Datum((long)hidEvent.value));
		} else if (i->getName() == "Button_2") {
			B->setValue(Datum((long)hidEvent.value));
		} else if (i->getName() == "Button_3") {
			X->setValue(Datum((long)hidEvent.value));
		} else if (i->getName() == "Button_4") {
			Y->setValue(Datum((long)hidEvent.value));
		} else if (i->getName() == "Button_5") {
			TL->setValue(Datum((long)hidEvent.value));
		} else if (i->getName() == "Button_6") {
			TR->setValue(Datum((long)hidEvent.value));
		}
	}
	
	return true;
}


bool mMSSWGamepad::startDeviceIO(){
    // Ensure that button variables are up to date before exiting this function
    updateButtons();
	
	shared_ptr<mMSSWGamepad> this_one = component_shared_from_this<mMSSWGamepad>();
	schedule_node = scheduler->scheduleUS(std::string(FILELINE ": ") + getTag(),
														   (MWorksTime)0, 
														   update_period, 
														   M_REPEAT_INDEFINITELY, 
														   boost::bind(update_buttons, 
																	   this_one),
														   M_DEFAULT_IODEVICE_PRIORITY,
														   M_DEFAULT_IODEVICE_WARN_SLOP_US,
														   M_DEFAULT_IODEVICE_FAIL_SLOP_US,
														   M_MISSED_EXECUTION_DROP);
	
	return true;
}


bool mMSSWGamepad::stopDeviceIO() {
    if (schedule_node) {
        schedule_node->cancel();
        schedule_node.reset();
    }
    return true;
}





















