#include "USBHID.h"
#include "boost/bind.hpp"


mUSBHID::mUSBHID(){						// initialize with the first available HID device
	
	
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
			mprintf("Error creating HID device interface");
		}
		
	} else {
		
		mprintf("Error creating HID device iterator");
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
		
		mprintf("Failed to create properties via IORegistryEntryCreateCFProperties.");
	}
	
	
}

mUSBHID::~mUSBHID(){
	
	IOReturn ioReturnValue = kIOReturnSuccess;
	
	ioReturnValue = (*pphidDeviceInterface)->close (pphidDeviceInterface);
	
	// Release the device interface
	(*pphidDeviceInterface)->Release (pphidDeviceInterface);
	
	IOObjectRelease (hidObjectIterator);
	
	//Free master port if we created one.
	if (masterPort)
		mach_port_deallocate (mach_task_self (), masterPort);
}


//mUSBHID(io_object_t hidDevice); // initialize with a particular HID device (you'd need to supply this from a dialog)


void mUSBHID::registerHIDProperties(CFTypeRef object){
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

void mUSBHID::registerHID_CFDictionary(CFDictionaryRef object)
{
	// Capability identifier = HID cookie
	CFTypeRef identifier_object = CFDictionaryGetValue (object, CFSTR(kIOHIDElementCookieKey));
	
	if(!identifier_object){
		mprintf("Error registering HID device capabilities");
		return;
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
	
	
	long capability_max_sampling_frequency = -1; // infinite (for now)
	
	IODataType capability_io_type = M_ANALOG_DATA; // for now
	
	if(strcmp(capability_hid_type, "Collection") == 0){
		mprintf("Registering collection of capabilities (%s, %s)", usagePageString, capability_name);
		CFStringRef keyElements = CFSTR(kIOHIDElementKey);
		CFTypeRef subelements = CFDictionaryGetValue (object, keyElements);
		registerHIDProperties(subelements); // recurse
	} else {
		mprintf("Registering %s (page: %s, type %s)...", capability_name, usagePageString, capability_hid_type);
		registerCapability(new IOCapability(capability_identifier, capability_name, M_INPUT_DATA,  capability_io_type, M_ASYNCHRONOUS_IO, capability_max_sampling_frequency, 0.0, 0.0, 1));
	}
}


void mUSBHID::registerHID_CFArray(CFArrayRef object)
{
	
	for(int i = 0; i < CFArrayGetCount(object); i++){
		registerHIDProperties(CFArrayGetValueAtIndex(object, i));
	}
}

// --------------------------------------------------------------------------

void mUSBHID::registerHID_CFArrayApplier (const void * value, void * parameter)
{
	if (CFGetTypeID (value) != CFDictionaryGetTypeID ())
		return;
	registerHIDProperties(value);
}


// initialize the device (pre- channel creation/initialization
bool mUSBHID::startup(){
	// no special action required  
	return true;
}



// mUSBHID can use the base methods for starting and stopping acquisition
/*bool mUSBHID::startAcquisition(){}
 bool mUSBHID::stopAcquisition(){}*/



bool mUSBHID::updateChannel(int channel_number){
	// get data for this channel
	if(channel_number > channels->getNElements()){
		// error
		return false;
	}

	boost::shared_ptr <IOChannel> this_channel(channels->getElement(channel_number)); 		
	
	IOHIDEventStruct hidEvent;
	(*pphidDeviceInterface)->getElementValue(pphidDeviceInterface, (void *)this_channel->getCapabilityIdentifier(), &hidEvent);
	
	boost::shared_ptr <Variable> the_variable(this_channel->getVariable());
	
	if(the_variable->getValue().getInteger() != hidEvent.value) {
		the_variable->setValue(Data((long)hidEvent.value));
	}
	
	return true;
}

bool mUSBHID::attachPhysicalDevice(){                                      // attach next avaialble device to this object
	
	// bypass all of this bullshit for now--
	attached_device = new IOPhysicalDeviceReference(0, "gamepad");
	
	return true;
}


bool mUSBHID::shutdown(){
	// no special action required
	return true;
}

bool mUSBHID::startDeviceIO(){
	schedule_nodes_lock.lock();
	for(int i = 0; i < channels->getNElements(); i++){
		
		shared_ptr<IOChannel>  the_channel = channels->getElement(i);
		
		shared_ptr<UpdateIOChannelArgs> args = shared_ptr<UpdateIOChannelArgs>(new UpdateIOChannelArgs());
		args->device = shared_from_this();
		args->channel_index = i;
		
		// here, we ask the scheduler to call update_io_channel at an interval specified by the channel
		// JJD note: for continously sampled data types, we expect that this each such call will cause ALL the available to data to come down off the device 
		//  (e.g. the parameter may be updated several times by a single call to update_io_channel)
		// of course, this depends on the sampling rate of the channel and the update rate of the channel (there is no requirement that they be matched)
		mprintf("Scheduling channel matched to capability = %s ...",  
				((the_channel->getCapability())->getName()).c_str());
		
		shared_ptr<Scheduler> scheduler = Scheduler::instance();
		shared_ptr<ScheduleTask> node = scheduler->scheduleUS(std::string(FILELINE ": ") + (the_channel->getCapability())->getName(),
															   (MonkeyWorksTime)0, 
															   (MonkeyWorksTime)(the_channel->getRequest())->getRequestedUpdateIntervalUsec(), 
															   M_REPEAT_INDEFINITELY, 
															   boost::bind(update_io_channel, 
															   args), 
															   M_DEFAULT_IODEVICE_PRIORITY,
															   M_DEFAULT_IODEVICE_WARN_SLOP_US,
															   M_DEFAULT_IODEVICE_FAIL_SLOP_US,
		M_MISSED_EXECUTION_DROP);
		schedule_nodes.push_back(node);       
	}
	schedule_nodes_lock.unlock();
	
	return true;
}


