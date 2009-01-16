
#include "IODeviceVariableNotification.h"
#include "IODevice.h"
#include "Experiment.h"
#include "boost/bind.hpp"

using namespace mw;

// logic of channels and channel updating:
// --------------------------------------
// The core should create a new request object for each request that it makes. (The IODevice object is responsible for cleaning up (i.e. deleting) these requests?? ASK_DAVE)
// 
// CHANNEL NEGOTIATION PHASE:
// a bunch of channel objects are created by calling the method ("mapRequestsToChannels") -- this will try to negotiate requests and create a new channel object for each channels that the request could be met.
//      For the base class, the "available" flag of the capabilities matched to channels will be set false.  Unresolved requests will each cause the creation of an incompatability object.
//      The channel object points at both the request object (maintained by the core) and the capability object that was matched (maintained by the IOdevice object).
//
// INITIALIZATION PHASE:
// next, "initializeChannels" should be called to actually initialize all the created channels (all the matches) at the hardware level
//  e.g. for the ITC, this means building and instruction sequence and loading it into the ITC, and preparing software buffers.
//
// START DATA COLLECTION PHASE:
// when IO is to be started for a device, "startDeviceIO" should be called. 
//  
// In the base IODevice class: "startDeviceIO" makes Scheduler requests(one request for each channel object) 
// to start calling the update_io_channel function with args that are specific to the channel
// Execution of the update_io_channel method will run the updateChannel method for the particular channel 
//   (i.e. a particular IOdevice, and a particular channel on that device)
// The updateChannel method for the channel causes the the Variableobject associated with the channel to update its value 
//   (using the "update" method of the channel object and the set method of the Variableobject)
// This updating causes event placement to the data stream, and thus the response of any "listeners" of the parameter (all in the core)
// (NOTE: in the base class, the updateChannel method is not specified, but is left to the builder of the IO device to specify)
//
// For the ITC18_IODevice class: same as above, except that the updateChannel(channel_index) method is overridden to do all of the following:
//		1) check that the channel index provided is valid (on the list of channel objects created by the device object)
//		2) try to get data off the software buffer for this channel, if found, call the update method for the channel (see above)
//		
//	In addition, the ITC schedules calls to service the FIFO at regular intervals:
//		1) flush the FIFO of the device (get any data from hardware down to the software buffers -- 
//				for ITC18, this will only happen after the instruction sequence has been run through as things come off 
//				in "chuncks" of data that are specified by the instruction sequence)
// 
// STOP DATA COLLECTION PHASE:
//  TODO !!!




namespace mw {
	// External function for scheduling
	void *update_io_channel(const shared_ptr<UpdateIOChannelArgs> &update_io_channels){
		// runs the updateChannel method for a particular IOdevice
		(update_io_channels->device)->updateChannel(update_io_channels->channel_index);                 
		
		// TODO: check this
		return NULL;
	}
	
	
	void *update_all_io_channels(void *void_args){
		
		// cast the null pointer as a pointer to a UpdateIOChannelArgs structure
		UpdateIOChannelArgs *args = (UpdateIOChannelArgs *)void_args;    
		
		// runs the updateChannel method for a particular IOdevice
		(args->device)->updateAllChannels();                 
		
		// TODO: check this
		return NULL;
	}
}


// ---------------------------------------------------------------------------------------------------------------------------
// A basic description of what the device can do
//class IOCapability {
//    protected:
//        char *name;
//        IOType type;
//		long max_sampling_frequency;

IOCapability::IOCapability(long _id, std::string _name, 
							 IODataDirection _dataDirection, IODataType _dataType, 
							 IODataSynchronyType _sync_type, 
							 long _min_data_interval_usec, double _range_min, 
							 double _range_max, int _resolution){
	
	identifier = _id;
	name = _name;
	data_direction = _dataDirection;
	data_type = _dataType;
	synchrony_type = _sync_type;
	min_data_interval_usec = _min_data_interval_usec;       		
	range_min = _range_min;
	range_max = _range_max;
	resolution_bits = _resolution;
	
	// the capability is announced by the device and isassumed to be avaialble 
	// until the object says otherwise ( device decides)
	available = true;           
}

IOCapability::IOCapability(IOCapability& copy){
	
	name = copy.name;
	
	data_type = copy.getDataType();
	data_direction = copy.getDataDirection();
	synchrony_type = copy.synchrony_type;
	min_data_interval_usec = copy.getMinDataIntervalUsec();
	identifier = copy.getIdentifier();
	range_min = copy.getRangeMin();
	range_max = copy.getRangeMax();
	resolution_bits = copy.getResolution();
	available = copy.isAvailable();
}

IOCapability::~IOCapability(){}

long				IOCapability::getIdentifier(){ return identifier; }
std::string			IOCapability::getName(){ return name; }
IODataDirection    IOCapability::getDataDirection(){ return data_direction; }
IODataType         IOCapability::getDataType(){ return data_type; }
IODataSynchronyType	IOCapability::getSynchronyType(){return synchrony_type;}
long				IOCapability::getMinDataIntervalUsec(){ return min_data_interval_usec; }
double				IOCapability::getRangeMin(){ return range_min; }
double				IOCapability::getRangeMax(){ return range_max; }
int					IOCapability::getResolution(){ return resolution_bits; }
void				IOCapability::setAvailable(bool _available) {available = _available; }
bool				IOCapability::isAvailable(){ return available; }


IOChannelRequest::IOChannelRequest(
									 std::string _channel_name,
									 shared_ptr<Variable> _param, 
									 std::string _requested_capability_name, 
									 IODataDirection _requested_data_direction, 
									 IODataType _requested_data_type, 
									 IODataSynchronyType _requested_synchrony_type,
									 long _requested_data_interval_usec, 
									 long _requested_update_interval_usec, 
									 double _requested_range_min, 
									 double _requested_range_max, 
									 int _requested_resolution_bits):Lockable(), mw::Component(_channel_name) {
	
	
    if (VERBOSE_IO_DEVICE) mprintf("mIOChannelRequest object has just been instantiated.");
	
	channel_name = _channel_name;
	parameter = _param;
	requested_capability_name = _requested_capability_name;
	requested_data_direction = _requested_data_direction;
	requested_data_type = _requested_data_type;
	requested_synchrony_type = _requested_synchrony_type;
	requested_data_interval_usec = _requested_data_interval_usec;         
	requested_update_interval_usec = _requested_update_interval_usec;   
	requested_range_min = _requested_range_min;
	requested_range_max = _requested_range_max;
	requested_resolution_bits = _requested_resolution_bits;
	resolved = false;       // the request is not yet resolved to a channel
	
}

IOChannelRequest::IOChannelRequest(IOChannelRequest& copy){
	channel_name = copy.getChannelName();
	parameter = copy.getVariable();
	
	requested_capability_name = copy.getRequestedCapabilityName();
	
	requested_data_direction = copy.getRequestedDataDirection();
	requested_data_type = copy.getRequestedDataType();
	requested_synchrony_type = copy.requested_synchrony_type;
	// the interval that data should be sampled (for input data)  
	// -- not relevant for asych data (e.g. is the line high now?)
	requested_data_interval_usec = copy.getRequestedDataIntervalUsec();        
	// the interval that the channel should be checked by the scheduler
	requested_update_interval_usec = copy.getRequestedUpdateIntervalUsec();     
	requested_range_min = copy.getRequestedRangeMin();
	requested_range_max = copy.getRequestedRangeMax();
	requested_resolution_bits = copy.getRequestedResolution();
	// the request is not yet resolved to a channel
	resolved = copy.isResolved();       
	
    if (VERBOSE_IO_DEVICE) mprintf("Copy constructor called on IOChannelRequest object");
}

IOChannelRequest::~IOChannelRequest(){ }

shared_ptr<Variable>  IOChannelRequest::getVariable(){ 
	SAFE_GET(shared_ptr<Variable>, parameter); 
	
}
std::string IOChannelRequest::getRequestedCapabilityName(){ 
	SAFE_GET(std::string,  requested_capability_name); 
	
}
IODataDirection  IOChannelRequest::getRequestedDataDirection(){ 
	SAFE_GET(IODataDirection, requested_data_direction); 
}

IODataType  IOChannelRequest::getRequestedDataType(){ 
	SAFE_GET(IODataType, requested_data_type); 
}

IODataSynchronyType	IOChannelRequest::getRequestedSynchronyType(){ 
	SAFE_GET(IODataSynchronyType, requested_synchrony_type); 
}

long IOChannelRequest::getRequestedDataIntervalUsec(){ 
	SAFE_GET(long, requested_data_interval_usec); 
}

long IOChannelRequest::getRequestedUpdateIntervalUsec(){ 
	SAFE_GET(long, requested_update_interval_usec); 
}

double IOChannelRequest::getRequestedRangeMin(){ 
	SAFE_GET(double, requested_range_min);
}

double IOChannelRequest::getRequestedRangeMax(){ 
	SAFE_GET(double, requested_range_max);
}

int	IOChannelRequest::getRequestedResolution(){
	SAFE_GET(int, requested_resolution_bits);
}

std::string IOChannelRequest::getChannelName() {
	SAFE_GET(std::string, channel_name);
}

void IOChannelRequest::setRequestedDataDirection(IODataDirection _direction){ 
	SAFE_SET(requested_data_direction, _direction); 
}

void IOChannelRequest::setRequestedDataType(IODataType _type){ 
	SAFE_SET(requested_data_type, _type);
}

void IOChannelRequest::setRequestedDataIntervalUsec(long _interval){
	SAFE_SET(requested_data_interval_usec,  _interval);
}

void IOChannelRequest::setRequestedUpdateIntervalUsec(long _interval){
	SAFE_SET(requested_update_interval_usec, _interval);
}

void IOChannelRequest::setRequestedRangeMin(double _min){
	SAFE_SET(requested_range_min, _min);
}

void IOChannelRequest::setRequestedRangeMax(double _max){
	SAFE_SET(requested_range_max, _max);
}

void IOChannelRequest::setRequestedResolution(int _bits){
	SAFE_SET(requested_resolution_bits, _bits);
}

bool IOChannelRequest::isResolved(){
	SAFE_GET(bool, resolved); 
}

void IOChannelRequest::setResolved(bool _resolved){
	SAFE_SET(resolved, _resolved); 
}

// ---------------------------------------------------------------------------------------------------------------------------
// A generated link between an advertised capability and a parameter (variable)
//   this link is the result of any device specific resolution of requests and capabilities.
//   it should contain information about the capability of the channel, the request, and the actual operating characteristics of the channel
//      We may want to make these COPY! so that any core changes to the requests do not change what was initialized used to setup the channel
//      We may also want to attach a monitor to the channel to periodically check and update the data and update intervals (for appropriate data types)
//class IOChannel {
//    protected:    
//        char *name;
//        IOCapability *capability;
//        Variable *parameter;
//		long sample_interval;
//        bool active;

IOChannel::IOChannel(IOChannelRequest *  _request, 
					   IOCapability *  _capability) : Lockable(){
	
	//request = new IOChannelRequest(*_request);       
	request = (IOChannelRequest*)_request->clone();   // copy the request      
	capability = _capability;               
	
	name = request->getChannelName();
	
	// this is the most important thing about the request                       
	variable = request->getVariable();		
	
	//nominal_data_interval_usec = 0;       
	// this should contain the value currently estimated for the channel
	//actual_data_interval_usec = 0;				
	
	// should be true before update is scheduled (indicates that hardware 
	// thinks that the channel is properly setup)
	initialized = false;						
	
	// will have to be activated to run
	active = false;						
	
	// ?
	// attach monitor (triggered by the update function)
	// channelMonitor = new monitor;
}

IOChannel::IOChannel(IOChannel& copy){
	//request = new IOChannelRequest(*copy.request);
	name = copy.getName();
    request = copy.getRequest()->clone();  
	capability = copy.getCapability();
	variable = copy.getVariable();
	initialized = copy.isInitialized();
	active = copy.isActive();
}

IOChannel::~IOChannel(){ }


std::string IOChannel::getName(){ 
	SAFE_GET(std::string, name); 
}

void IOChannel::setName(std::string _name){
	SAFE_SET(name, _name);
}

//TODO -- data freshness monitor?

// this use of the set method causes the core to change the value of the 
// variable associated with this channel to the value contained in data
//  The set method should be called once for each new piece of data.  
// Any changes to the running of code should be initiated by "listeners" of 
// particular parameters (in the core)

void IOChannel::update(Data data){
	lock();		// JJD dropped this.  This is NOT a public method.
	// DDC added this back in.  This is NOT the issue
	bool _active = active;
	unlock();
	
	if (_active) variable->setValue(data);
} 

void IOChannel::update(Data data, MonkeyWorksTime timeUS){ 
	lock();		// JJD dropped this.  This is NOT a public method. 
	// DDC added this back in.  This is NOT the issue
	bool _active = active;
	unlock();
	
	if (_active) variable->setValue(data, timeUS); 
} 

// JJD commented out on Aug 15, 2005 -- all variables have data?
void IOChannel::update(MonkeyWorksTime timeUS){ 
	lock();		// JJD dropped this.  This is NOT a public method. 
	bool _active = active;
	unlock();
	
    
	if (_active) variable->setValue((Data)((long)M_NO_DATA), timeUS); 
} 

IOChannelRequest *  IOChannel::getRequest(){ 
	SAFE_GET(IOChannelRequest * , request); 
}

IOCapability *  IOChannel::getCapability(){ 
	SAFE_GET(IOCapability * , capability); 
}

shared_ptr<Variable> IOChannel::getVariable(){ 
	SAFE_GET(shared_ptr<Variable>, variable); 
}


//void    IOChannel::setNominalDataIntervalUsec(long _nominal_data_interval_usec) { nominal_data_interval_usec = _nominal_data_interval_usec;}
//long    IOChannel::getNominalDataIntervalUsec(){ return nominal_data_interval_usec; }
//void    IOChannel::setActualDataIntervalUsec(long _actual_data_interval_usec) { actual_data_interval_usec = _actual_data_interval_usec;}

long    IOChannel::getActualDataIntervalUsec(){ 
	SAFE_GET(long, request->getRequestedDataIntervalUsec()); 
}

// done by the device when the hardware channel has actually been initialized
bool    IOChannel::initialize(){ 
	SAFE_SET(initialized, true);
	return true;
}      

bool    IOChannel::isInitialized(){
	SAFE_GET(bool, initialized);
}

bool    IOChannel::isActive(){ 
	SAFE_GET(bool, active); 
}

bool    IOChannel::activate(){ 
	bool init;
	SAFE_SET(init, initialized);
	
	if (init) {
		SAFE_SET(active,true); 
		return true;
	} else {
		return false;
	} 
}


bool    IOChannel::deactivate(){ 
	SAFE_SET(active, false);
	return true; 
}

long    IOChannel::getCapabilityIdentifier() { 
	// already thread safe
	return (getCapability())->getIdentifier();
}          

// ---------------------------------------------------------------------------------------------------------------------------
//class IOChannelIncompatibility{
//	protected:	
//		IOChannelRequest *request; // the request that could not be satisfied perfectly
//		IOChannel *channel;  // the channel that the request ended up getting assigned, or NULL, if was really impossible

IOChannelIncompatibility::IOChannelIncompatibility(
													 IOChannelRequest *  _request, 
													 IOChannelRequest *  _modified_request){
	
    //request = new IOChannelRequest(*_request);
    //modified_request = new IOChannelRequest(*_modified_request);
    request = (IOChannelRequest *)_request->clone();  
    modified_request = _modified_request->clone(); 
	incompatibilities = new ExpandableList<IOIncompatibilityType>();
}

IOChannelIncompatibility::IOChannelIncompatibility(
													 IOChannelRequest *  _request, 
													 IOChannelRequest *  _modified_request, 
													 IOIncompatibilityType _incompatabilityType){
	
    //request = new IOChannelRequest(*_request);
    //modified_request = new IOChannelRequest(*_modified_request);
    request = _request->clone();  
    modified_request = _modified_request->clone(); 
	incompatibilities = new ExpandableList<IOIncompatibilityType>();
	
    this->addIncompatibilityType(_incompatabilityType);
}



IOChannelIncompatibility::IOChannelIncompatibility(
													 IOChannelRequest *  _request, 
													 IOChannelRequest * _modified_request, 
													 ExpandableList<IOIncompatibilityType> *_incompatibilities){
	
    request = _request->clone();  
    modified_request = _modified_request->clone(); 
    incompatibilities = _incompatibilities;
}


IOChannelIncompatibility::IOChannelIncompatibility(IOChannelIncompatibility& copy){
    request = copy.getRequest();
    modified_request = copy.getModifiedRequest();
    incompatibilities = new ExpandableList<IOIncompatibilityType>(*(copy.getIncompatibilityTypes()));
}



IOChannelIncompatibility::~IOChannelIncompatibility(){
	if(incompatibilities != NULL){
		delete(incompatibilities);
	}
}

IOChannelRequest *  IOChannelIncompatibility::getRequest(){ return request; }
IOChannelRequest *  IOChannelIncompatibility::getModifiedRequest(){ return modified_request; }

ExpandableList<IOIncompatibilityType>  *IOChannelIncompatibility::getIncompatibilityTypes(){ return incompatibilities; }

void IOChannelIncompatibility::setRequest(IOChannelRequest *  _request){ request = _request; }
void IOChannelIncompatibility::setModifiedRequest(IOChannelRequest *  _modified_request){ modified_request = _modified_request; }
void IOChannelIncompatibility::setIncompatibilities(ExpandableList<IOIncompatibilityType> *_incompatibilities){ incompatibilities = _incompatibilities; }
void IOChannelIncompatibility::addIncompatibilityType(IOIncompatibilityType _type){ 
	shared_ptr<IOIncompatibilityType> incomp(new IOIncompatibilityType(_type));
	incompatibilities->addElement(incomp); 
}

// ---------------------------------------------------------------------------------------------------------------------------
// class IODeviceReference{
//        protected:
//            long    uniqueID;         // must be greater than 0 to be a valid device reference  
//            char    *name;
//            char    *description;

//public:
IOPhysicalDeviceReference::IOPhysicalDeviceReference(long _id, std::string _name) {      
	uniqueID = _id;      /// assume no valid device until explicitly set
	setAvailable(true);  // JJD added July 2005
    
	name = _name;
	
	description = "";
}

IOPhysicalDeviceReference::IOPhysicalDeviceReference(const IOPhysicalDeviceReference& copy) {      
	uniqueID = copy.uniqueID;      /// assume no valid device until explicitly set
	setAvailable(true);             // JJD added July 2005
    
	name = copy.name;
	
	description = copy.description;
}

IOPhysicalDeviceReference::~IOPhysicalDeviceReference(){ }


bool    IOPhysicalDeviceReference::isAvailable() {
	return available;
}

void	IOPhysicalDeviceReference::setAvailable(bool _available){
	available = _available;
}

long    IOPhysicalDeviceReference::getUniqueID() { return uniqueID;}
void    IOPhysicalDeviceReference::setUniqueID(long _uniqueID) {uniqueID = _uniqueID;}
std::string IOPhysicalDeviceReference::getName() {return name;}
void    IOPhysicalDeviceReference::setName(std::string _name) {
	name = _name;
}
std::string IOPhysicalDeviceReference::getDescription() {return description;}

void    IOPhysicalDeviceReference::setDescription(std::string _description) {
	description = _description;
}


// ---------------------------------------------------------------------------------------
// class IODevice {
//    protected data:
//          NOTE: capabilities should not be set without first attaching a specific device.  This means that no requests will be met until a specific device is attached.
//        IODeviceReference                          *attachedDeviceRef;      
//        ExpandableList<IOCapability>              *capabilities;              // pointer to list of all advertised capabilities of the specific device
//        ExpandableList<IOChannelRequest>          *pending_channel_requests;  // pointer to list of all requests that are currently unmet
//        ExpandableList<IOChannel>                 *channels;                  // pointer to list of all established channels (links between params and devices)
//        ExpandableList<ScheduleTask>              *schedule_nodes;            // pointer to list of all nodes that should be run by the scheduler (e.g. get latest data)
//        ExpandableList<IOChannelIncompatibility>  *incompatibilities;         // pointer to list of all alterations and failures that occured during the fulfillment of the IOChannelRequest's
//    protected methods:
//        bool    registerCapability(IOCapability *capability);      // add a capability to the list of available capabilities of the specific device           
//        int     matchCapabilityByName(char *name);                  // will use this to campare avaialble capability names to a requested capability name


// class IODevice:  protected methods follow: ------------------------    

// add a capability to the list of available capabilities
bool IODevice::registerCapability(IOCapability *  capability){
	
	shared_ptr<IOCapability> carrier(capability);
	capabilities->addElement(carrier);
	// might need to check with the device to see if this is reasonable...
	return true;
}

// loop through all the capabilities of this device.
// if one of the capability names matches the requested capability name (the arg), then return the index that points to the capability.
int IODevice::matchCapabilityByName(std::string name){
	
	for(int i = 0; i < capabilities->getNElements(); i++){
		//mprintf("%s ~ %s", name, (capabilities->getElement(i))->getName());
		std::string capability_name = capabilities->getElement(i)->getName();
		if(name == capability_name) { 
			return i;
		}
	}
	return -1;		// no match
}


// class IODevice:  public methods follow: ------------------------

// constructor function called when IODevice is instantiated  -- start expandable lists
IODevice::IODevice(){
	
	attached_device = 0;
	capabilities = new ExpandableList<IOCapability>();
	channels = new ExpandableList<IOChannel>();
	pending_channel_requests = new ExpandableList<IOChannelRequest>();
	incompatibilities = new ExpandableList<IOChannelIncompatibility>();
}


IODevice::IODevice(const IODevice& copy){
	
	//	attached_device = copy.getAttachedPhysicalDevice();
	capabilities = new ExpandableList<IOCapability>(*(copy.capabilities));
	channels = new ExpandableList<IOChannel>(*(copy.channels));
	pending_channel_requests = new ExpandableList<IOChannelRequest>(*(copy.pending_channel_requests));
	incompatibilities = new ExpandableList<IOChannelIncompatibility>(*(copy.incompatibilities));
	schedule_nodes = copy.schedule_nodes;
}

IODevice::~IODevice(){
	
	// -------------------------------------------------------------
	if (channels != NULL) delete channels;									// delete the channels
	if (capabilities != NULL) delete capabilities;							// delete the expandable list object		
	if (pending_channel_requests != NULL) delete pending_channel_requests;  // delete the expandable list object
	if (incompatibilities != NULL) delete incompatibilities;				// delete the expandable list object
}

// make new channel -- this is virtual so that devices can support derived channel classes and still use base class functionality
IOChannel *  IODevice::makeNewChannel(
										IOChannelRequest *  _request, 
										IOCapability *  _capability) {
	return new IOChannel(_request, _capability);
}

// find and return available devices of this type (note: these calls should be created in the plugin)
//  -- this function can do this in any way that it likes, but it should return a list (may be of length 0)
//  (NOTE: the memory for this list may not live with the object, but could be static in the plugin)
ExpandableList<IOPhysicalDeviceReference> *IODevice::getAvailablePhysicalDevices(){
	return new ExpandableList<IOPhysicalDeviceReference>();       // should create list of devices
}

// the attach functions should do two things:
//          1) prevent this attached device from showing up on the available list in future calls to GetAvailableIODevices 
//                  (e.g. move the device from the available list to a busy list)
//          2) assign the IOdevice index so that all calls to THIS object will go to the specified device
//                  (e.g. set a pointer to the requested device)
bool IODevice::attachPhysicalDevice(){  
	return false; 
}         // attach next avaialble device

bool IODevice::attachPhysicalDevice(IOPhysicalDeviceReference* deviceToAttach){  // overriden versions will want to validate
	attached_device = new IOPhysicalDeviceReference(*deviceToAttach);
	attached_device->setAvailable(false);	// assumes 1 physical device : 1 IODevice
	// could be overridden for special devices
	
	return true;
} 

IOPhysicalDeviceReference* IODevice::getAttachedPhysicalDevice() {            // allows the core to query to see which device this object is currently attached to (if any)
	return attached_device;
}

bool IODevice::isAttached(){
	if(attached_device != NULL){
		return true;
	}
	
	return false;
}


// Use this to find out what the device can do -- requires that a specifc device is attached,
ExpandableList<IOCapability> *IODevice::getCapabilities(){
	if (!isAttached()) {       // if deviced not attached, return NULL pointer and error message
		mprintf("Error: IODevice::getCapabilities:  call to get capabilities without first attaching device.");
		return(NULL);
	} 
	// should set capabilities in here (if they are not already set)
	return capabilities;        // return a pointer to the list of capabilities (protected data)
}

// use this to add to your list of requests you would like to make of the device (which may or may not yet be attached)
bool IODevice::requestChannel(shared_ptr<IOChannelRequest>  request){
	
    pending_channel_requests->addReference(request);    // JJD change, Jan 2007
	//pending_channel_requests->addElement(request);   // these are device-independent requests (checking should be done at match time)
	return true;
}

void IODevice::addChild(std::map<std::string, std::string> parameters,
						 mwComponentRegistry *reg,
						 shared_ptr<mw::Component> child){
	
	shared_ptr<IOChannelRequest> request = dynamic_pointer_cast<IOChannelRequest, mw::Component>(child);
	
	if(request == NULL){
		// TODO: better throw
		throw SimpleException("Attempt to add an invalid IO channel");
	}
	
	if(!requestChannel(request)){
		// TODO: better throw
		throw SimpleException("Unable to request IO channel");
	}
}

// Running the device, acquiring data, closing up shop
bool IODevice::startup(){
	if (!isAttached()) {       // if deviced not attached, return NULL pointer and error message
		mprintf("Error: IODevice::startup:  startup request made without first attaching device.");
		return false;
	} 
	
	// should override to do something
	return false;
}

void IODevice::finalize(std::map<std::string, std::string> parameters,
						 mwComponentRegistry *reg){
	
	shared_ptr <IODevice> this_one = shared_from_this();
	shared_ptr<IODeviceVariableNotification> notification(new IODeviceVariableNotification(this_one));
	task_mode->addNotification(notification);			
	
	if(!attachPhysicalDevice()) {
		// if we can't attach to it, map the tag to the alt device
		if(parameters.find("alt") == parameters.end()) {
			throw SimpleException("Can't start iodevice (" + tag + ") and no alt tag specified");
		} else {
			shared_ptr <IODevice> alt_io_device = reg->getObject<IODevice>(parameters.find("alt")->second);
			reg->registerAltObject(tag, alt_io_device);
			return;
		}
	}
	
	startup();
	
	if(!mapRequestsToChannels()){
		// TODO: better throw
		throw SimpleException("Failed to map IO Device channels");
	}
	
	initializeChannels();
}


// Attempt to match up all requests with capabilities, warn if something is not possible
// The need to override this method would come if there was additional device specific logic
// (e.g. only so much sampling throughput on a given A/D device)

// JJD update July 27, 2005
// there are two possible outcomes to calling this routine
// 1) ALL pending_channel_requests are matched (individually and as a group)
//      in this case, we create a set of channels (=number of requests)
//      and we clear the pending requests
//      no incompatabilities are created
// 2) one or more requests could not be met
//      in this case, we do NOT create any channels
//      we clear the pending requests
//      we create a set of incompatabilities (may be more than one per request, or none for some requests)
//     Eventually, the system should be smart enough to use these incompatabilities to generate a valid set of requests

bool IODevice::mapRequestsToChannels(){
	
	IOCapability *    matched_capability;
	//	IOChannel *  tentative_channel;
    ExpandableList<IOChannel> *first_pass_validated_channels; 
    
    // RESET THINGS ==================================
    
	// delete all incompatibilties and channels
    incompatibilities->clear();     
    channels->clear();
	
    // set all the request resolved fields to false
    for(int i = 0; i < pending_channel_requests->getNElements(); i++){
        (pending_channel_requests->getElement(i))->setResolved(false);
    }
    // ================================================ 
	
	
    /*						
	 if (!isAttached())  {       // if deviced not attached, return false and error message
	 mprintf("Error: IODevice::initializeChannels:  initializeChannels request made without first attaching device.");
	 return false;
	 } 
	 */
    
	// TODO: must negotiate all of this correctly!!
	// create a temporary list to store first-pass validated channels
	// we do this because some conflicts will only become apparent
	// when the device can look at everything all at once (e.g. 
	// aggregate sampling rate is too high.  This step 
	// must be done in a separate routine because each device must
	// override this method ("ValidateMultipleChannels") to use it.
	
    // will build this list of channels
    first_pass_validated_channels = new ExpandableList<IOChannel>();
    IOChannelRequest *  request;
    IOChannelIncompatibility* incompatibility;
    
	// for each request, try to find a matching capability and then setup a new channel
	for(int i = 0; i < pending_channel_requests->getNElements(); i++){
		
		// get the request in question
        request = (pending_channel_requests->getElement(i)).get();		
		
		// check if the capability has a name
		if ( (request->getRequestedCapabilityName() == "") ) {
            incompatibility = new IOChannelIncompatibility(request, request, 
															M_NO_MATCHING_CAPABILITY);
			incompatibility->setModifiedRequest(NULL); // nothing we can do...
			
			shared_ptr<IOChannelIncompatibility> carrier(incompatibility);
            incompatibilities->addReference(carrier);
            continue;             // continue to next iteration in the for loop
		}
		
		
		// TODO: there's a problem here if more than one capability has the
		//		 same name
		//		This could be a real problem depending on how the derived
		//		classes capabilities are handled (e.g. the same port could 
		//		be capable of many different synchrony types)
		//  The ITC is OK with more than one request having the same capability name.  
		//   For example, one "channel" can listen to TTL port 0 and reports only edges, while
		//   another "channel" also listens to TTL port and reports continuous digital data.
		//   Thus, they have the same capability name, with different data types.
		//  DDC and JJD discussed:  plan is that it is OK to have same capability name, but device 
		//	 should do checking to make sure that is OK.  In general the capability name
		//   should be matched to a physical port on the device.
        int index = matchCapabilityByName(request->getRequestedCapabilityName());
		if (index<0) {   // No match
			if(VERBOSE_IO_DEVICE) mprintf("No matching capability... %s", 
										  (request->getRequestedCapabilityName()).c_str());
            incompatibility = new IOChannelIncompatibility(request, request, 
															M_NO_MATCHING_CAPABILITY);
			incompatibility->setModifiedRequest(NULL); // nothing we can do...
			
			shared_ptr<IOChannelIncompatibility> carrier(incompatibility);
            incompatibilities->addReference(carrier);
            continue;       // continue to next iteration in the for loop
		}
        
        matched_capability = (capabilities->getElement(index)).get();
		
        if (!matched_capability->isAvailable()) {
            if(VERBOSE_IO_DEVICE) mprintf("Capability not available... %s",
										  (request->getRequestedCapabilityName()).c_str());
            incompatibility = new IOChannelIncompatibility(request, 
															request, M_CAPABILITY_NOT_AVAILABLE);
            incompatibility->setModifiedRequest(NULL); // nothing we can do...
			
			shared_ptr<IOChannelIncompatibility> carrier(incompatibility);
            incompatibilities->addReference(carrier);
            continue;       // continue to next iteration in the for loop
        }
		
        // if we get this far, then at least we have a capability this might work out.
        // go ahead an make a channel object 
        // this list is only used if all channels check out individually 
        IOChannel *  tentative_channel = 
		makeNewChannel(request, matched_capability);
		shared_ptr<IOChannel> carrier(tentative_channel);
        first_pass_validated_channels->addReference(carrier);	// handle to new channel
		
    }
    
	
    // all "first-pass' channels must have been created, else we return
    if (first_pass_validated_channels->getNElements() < pending_channel_requests->getNElements()) {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, 
				 "mIODevice:  only %d of %d channel requests could be matched to the capabilities of the IO device. No channels created.", 
				 first_pass_validated_channels->getNElements(), pending_channel_requests->getNElements());
        first_pass_validated_channels->clear();
        delete first_pass_validated_channels;
        return false;       // incompatibilities (list) contains list of all the problems (so far)
    }
    
    
    
    // ===============================================================
    // now build a temp list of channels and check out each individually -- this routine may be device sepecific
    if(VERBOSE_IO_DEVICE) mprintf("mIODevice: Validating all %d proposed channels at an individual level.",first_pass_validated_channels->getNElements());
    int n_requests_that_are_OK_individually = 0;
    for(int c = 0; c <first_pass_validated_channels->getNElements(); c++) { 
		
        incompatibility = new IOChannelIncompatibility(request, request);
        bool chanIndividuallyValidated = 
		validateIndividualChannel(
								  (first_pass_validated_channels->getElement(c)).get(), 
								  incompatibility);
        if (!chanIndividuallyValidated) {
			shared_ptr<IOChannelIncompatibility> carrier(incompatibility);
            incompatibilities->addReference(carrier);
        }
        else {
            n_requests_that_are_OK_individually++;
        }
    }
	
    if (n_requests_that_are_OK_individually< first_pass_validated_channels->getNElements()) {
        // note, we still have not created and "formal" channels (the "channels" list is still empty) 
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, 
				 "mIODevice:  only %d of %d channel requests could be resolved at the individual validation step. No channels created.", 
				 n_requests_that_are_OK_individually, first_pass_validated_channels->getNElements());
        first_pass_validated_channels->clear();
        delete first_pass_validated_channels;
        return false;       // incompatibilities (list) contains list of all the problems (so far)
    }       
	
	
	
    // ===============================================================        
    // if we are here, all request are individually OK. 
    // check as a group
    // if it has passed all of this, put it into the "first pass"
    // validated queue.  We still need to make sure that we
    // don't blow the aggregate sampling limit or other device-
    // specific constraint 
    if (VERBOSE_IO_DEVICE) mprintf("mIODevice: Validating all %d proposed channels as a group.", first_pass_validated_channels->getNElements());
    if(!validateMultipleChannels(first_pass_validated_channels)) {      // will add to incompatabiliteis list if any problems
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, 
				 "mIODevice:  Channel group validation was NOT successful. No channels created.");
        first_pass_validated_channels->clear();
        delete first_pass_validated_channels;
        return false;       // incompatibilities (list) contains list of all the problems (so far)
    }
    
    
    // ===============================================================        
    // if we are here, everything checks out. -- formally establish the channels
    channels->clear();
    if (incompatibilities->getNElements()>0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
			   "IODevice match channels:  we should not have any incompatabilities if we have gotten here.");
    }
	for(int i = 0; i < first_pass_validated_channels->getNElements(); i++){	
		shared_ptr<IOChannel> channel = 
		first_pass_validated_channels->getElement(i);
		(channel->getRequest())->setResolved(true);
        channels->addReference(channel);
	}
    first_pass_validated_channels->releaseElements();	// DO NOT USE CLEAR -- note that we just moved the pointer above!
	delete first_pass_validated_channels; 
    
    pending_channel_requests->clear();  // all are resolved
    //request setResolved(true); // put this somewhere ??  // not clear if this is needed because resolved requests are removed                                                                                         
	
	
    if(VERBOSE_IO_DEVICE) mprintf("mIODevice:mapRequestsToChannels: successful mapping done.");
    return true;                                      					  			  
}





// return true if channel is valid.  
// if false, then incompatability should (ideally) now contain more info about what is wrong.    
bool IODevice::validateIndividualChannel(
										  IOChannel *  tentative_channel, 
										  IOChannelIncompatibility* incompatibility) {
	
	bool success = true;      // glass is half full, at least until it isn't.
	IOCapability *  matched_capability = 
	tentative_channel->getCapability();
	IOChannelRequest *  request = tentative_channel->getRequest();   
	
	// check data direction
	if(request->getRequestedDataDirection() != matched_capability->getDataDirection()){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Incorrect data direction for requested capability...");
		incompatibility->addIncompatibilityType(M_INCORRECT_DATA_DIRECTION);
		(incompatibility->getModifiedRequest())->setRequestedDataDirection(matched_capability->getDataDirection());
		success = false;
	}
	
	// check data type
	if (matched_capability->getDataType() != M_DEVICE_SPECIFIC_DATA)  {  // no checking in this case, us to device to check the data request
		if(request->getRequestedDataType() != matched_capability->getDataType()){
			mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Incorrect data type for requested capability ...");
			incompatibility->addIncompatibilityType(M_INCORRECT_DATA_TYPE);
			(incompatibility->getModifiedRequest())->setRequestedDataType(matched_capability->getDataType());
			success = false;
		}
	}
	
	// check synchrony type
	if(request->getRequestedSynchronyType() != matched_capability->getSynchronyType()){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Unsupported synchrony requested...");
		incompatibility->addIncompatibilityType(M_UNSUPPORTED_SYNCHRONY_TYPE);
		(incompatibility->getModifiedRequest())->setRequestedDataType(matched_capability->getDataType());
		success = false;
	}
	
	// check data interval
	if(request->getRequestedDataIntervalUsec() < matched_capability->getMinDataIntervalUsec()){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Requested sampling rate too high... %d us < %d us",
				 request->getRequestedDataIntervalUsec(), matched_capability->getMinDataIntervalUsec());
		incompatibility->addIncompatibilityType(M_SAMPLING_RATE_TOO_HIGH);
		(incompatibility->getModifiedRequest())->setRequestedDataIntervalUsec(matched_capability->getMinDataIntervalUsec());
		success = false;
	}
	
	// check min range
	if(request->getRequestedRangeMin() < matched_capability->getRangeMin()){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Requested data range too low...");
		incompatibility->addIncompatibilityType(M_DATA_RANGE_TOO_LOW);
		(incompatibility->getModifiedRequest())->setRequestedRangeMin(matched_capability->getRangeMin());
		success = false;
	}
	
	// check max range
	if(request->getRequestedRangeMax() > matched_capability->getRangeMax()){
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Requested data range too high...");
		incompatibility->addIncompatibilityType(M_DATA_RANGE_TOO_HIGH);
		(incompatibility->getModifiedRequest())->setRequestedRangeMax(matched_capability->getRangeMax());
		success = false;
	}
	
	// check resolution
	if(request->getRequestedResolution() > matched_capability->getResolution()){		
		mwarning(M_IODEVICE_MESSAGE_DOMAIN,"IO Device channel: Requested data res too high...");
		incompatibility->addIncompatibilityType(M_DATA_RESOLUTION_TOO_HIGH);
		(incompatibility->getModifiedRequest())->setRequestedResolution(matched_capability->getResolution());
		success = false;
	}        
	
	return (success);       
}



// loop through all channels -- any newby channels should be initialized at the device level
// (this is hardware-specific)  For example, the ITC18 must see all requests at once to make set everything up.
// this method should be overriden in the ITC18 device class
bool IODevice::initializeChannels(){
	for (int i=0;i<channels->getNElements(); i++){
		shared_ptr<IOChannel>  chan = channels->getElement(i); 
		chan->initialize();            // this code is fine only if the hardware does not actually need initialized (it just unlocks the channel so that it can be "activated")
		//chan->setNominalDataIntervalUsec(??);     // hardware should specify what it is going to try to achieve on this channel (this allows monitors to work)
	}
    return true;
}


// this method should be overriden if any special channel startup is needed
bool IODevice::startDeviceIO(){
	if (!isAttached())  {       // if deviced not attached, return false and error message
		mprintf("Error: IODevice::startDeviceIO:  startDeviceIO request made without first attaching device.");
		return false;
	} 
	stopAllScheduleNodes(); // DDC experimental
	if (!startDeviceIOchannels()) return false;
	
	return true;
}		


// method to start all channels (private)
bool IODevice::startDeviceIOchannels() {
	
	for(int i = 0; i < channels->getNElements(); i++){
		
		shared_ptr<IOChannel>  the_channel = channels->getElement(i);
		
		if (the_channel->isInitialized()) {		// check that we have already initialized the cahnnel before we try to schedule its update method 
			
			shared_ptr<UpdateIOChannelArgs> args = shared_ptr<UpdateIOChannelArgs>(new UpdateIOChannelArgs());
			args->device = shared_from_this();
			args->channel_index = i;
			
			// here, we ask the scheduler to call update_io_channel at an interval specified by the channel
			// JJD note: for continously sampled data types, we expect that this each such call will cause ALL the available to data to come down off the device 
			//  (e.g. the parameter may be updated several times by a single call to update_io_channel)
			// of course, this depends on the sampling rate of the channel and the update rate of the channel (there is no requirement that they be matched)
			mprintf("Scheduling channel matched to capability = %s ...",  
					((the_channel->getCapability())->getName()).c_str());
			
			// the next call starts the scheduled task immediately
			shared_ptr<Scheduler> scheduler = Scheduler::instance();	
			shared_ptr<ScheduleTask> node = scheduler->scheduleMS(std::string(FILELINE) + std::string(": Capability: ") + (the_channel->getCapability())->getName(),
																   0, 
																   (the_channel->getRequest())->getRequestedUpdateIntervalUsec(), 
																   M_REPEAT_INDEFINITELY,
																   boost::bind(update_io_channel, 
																			   args),
																   M_DEFAULT_IODEVICE_PRIORITY,
																   M_DEFAULT_IODEVICE_WARN_SLOP_MS, 
																   M_DEFAULT_IODEVICE_FAIL_SLOP_MS,
																   M_MISSED_EXECUTION_DROP);
			schedule_nodes_lock.lock();		 // JJD Feb 2007
			schedule_nodes.push_back(node);       
			schedule_nodes_lock.unlock();		// JJD Feb 2007
		}		
		
	}
	return true;
}



bool IODevice::stopDeviceIO(){
	if (!isAttached())  {       // if deviced not attached, return false and error message
		mprintf("Error: IODevice::stopDeviceIO:  stopDeviceIO request made without first attaching device.");
		return false;
	} 
	
	// JJD updated this June 2006
	stopAllScheduleNodes();
	
    return true;
}

// this is called by the scheduler once every 'requested update interval' for EACH channel
// the channel should have been initialized to get this far, but it may not be active.
// Thus, the updateChannel method must decide how to handle the 'active' and 'inactive' channel states
bool IODevice::updateChannel(int channel_index){ return false;}     

bool IODevice::updateChannel(int channel_index, Data data){ 
	return updateChannel(channel_index);
}

bool IODevice::updateAllChannels(){
	for(int i = 0; i < channels->getNElements(); i++){
		updateChannel(i);
	}
	return true;
}

bool IODevice::shutdown(){
	if (!isAttached())  {       // if deviced not attached, return NULL pointer and error message
		mprintf("Error: IODevice::startup:  shutdown request made without first attaching device.");
		return false;
	}
	
	// should override to do something
	return true;
}

// Managing and querying the device
int IODevice::getChannelIndex(std::string channel_name){
	for(int i = 0; i < channels->getNElements(); i++){
		if(channel_name == (*channels)[i]->getName()){
			return i;
		}
	}
	mprintf("Error: IODevice::getChannelIndex:  could not find channel index from channel name.");
	return -1; // no such channel
}

// access channel by name  
IOChannel *  IODevice::getChannel(std::string channel_name){
	int channelIndex;
	channelIndex = getChannelIndex(channel_name);
	if (channelIndex >= 0) {
		return getChannel(channelIndex);
	} else {
		mprintf("Error: IODevice::getChannel(name):  could not find channel index from channel name.");
		return NULL;
	}    
}

// access channel by index  
IOChannel *  IODevice::getChannel(int channel_index){
	if ( (channel_index >= 0) || (channel_index < channels->getNElements()) ) {
		return ((*channels)[channel_index]).get();
	} else {
		mprintf("Error: IODevice::getChannel(int): channel index out of range.");
		return NULL;        
	}
}

ExpandableList<IOChannel> *IODevice::getChannels(){
	return channels;
}

ExpandableList<IOChannelIncompatibility> *IODevice::getIncompatibilities(){
	return incompatibilities;
}


bool IODevice::validateMultipleChannels(ExpandableList<IOChannel> *candidates){
	return candidates;
}



bool IODevice::deactivateChannel(std::string channel_name){ return false;}
bool IODevice::deactivateChannel(int channel_index){return false;}

bool IODevice::activateChannel(std::string channel_name){return false;}
bool IODevice::activateChannel(int channel_index){return false;}

bool IODevice::removeChannel(std::string channel_name){return false;}
bool IODevice::removeChannel(int channel_index){return false;}

bool IODevice::stopAllScheduleNodes(){
	if (VERBOSE_IO_DEVICE) mprintf("mIODevice: locking schedule nodes");
	schedule_nodes_lock.lock();
	
	for(vector <shared_ptr<ScheduleTask> >::const_iterator i = schedule_nodes.begin();
		i != schedule_nodes.end();
		++i) {
		(*i)->cancel();
	}
	
	schedule_nodes.clear();
	schedule_nodes_lock.unlock();
	if (VERBOSE_IO_DEVICE) mprintf("mIODevice: unlocked schedule nodes");
	
	return(true);
}



AsynchronousOutputNotification::AsynchronousOutputNotification( 
																 shared_ptr<IODevice> _device,
																 int _channel_index):
VariableNotification(){
	device = _device;
	channel_index = _channel_index;
}

void AsynchronousOutputNotification::notify(const Data& data, MonkeyWorksTime timeUS){
	device->updateChannel(channel_index, data);
}






