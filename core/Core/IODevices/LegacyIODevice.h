#ifndef _LEGACYIODEVICE_H
#define _LEGACYIODEVICE_H

#include "Lockable.h"
#include "IODevice.h"

#include "GenericVariable.h"
#include "ExpandableList.h"

#include "Experiment.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "ComponentRegistry.h"
#include <boost/algorithm/string.hpp>


BEGIN_NAMESPACE_MW
	
// set VERBOSE_IO_DEVICE to 2 to see all the gory details, setup, etc. (lots of print)
#define VERBOSE_IO_DEVICE 0     

// set this to see data
#define VERBOSE_IO_DEVICE_DATA 0	

// The IO device designer should probably NOT use this, but should be more clever.
// if the warn time is shorter than the scheduled interval, you get situations
// where we get lots of warnings.  JJD Jan 30, 2007
#define M_IODEVICE_WARN_SLOP_US	2000

enum IODataDirection{ M_INPUT_DATA=0, M_OUTPUT_DATA };

enum IODataSynchronyType{ M_HARDWARE_TIMED_SYNCHRONOUS_IO, 
	M_SOFTWARE_TIMED_SYNCHRONOUS_IO, M_ASYNCHRONOUS_IO}; 


// THESE ARE THE DATA TYPES THAT MIGHT BE VISIBLE TO THE USER.
//M_DEVICE_SPECIFIC_DATA should NOT be in a request, but might be in a capability
// in this case, the specific device object must decide if it is happy with the request. 
enum IODataType{ M_DEVICE_SPECIFIC_DATA=0, M_ANALOG_DATA, M_DIGITAL_DATA,
    M_EDGE_LOW_TO_HIGH, M_EDGE_HIGH_TO_LOW, M_EDGE_ANY,
	M_DIGITAL_PULSE_ACTIVE_HIGH, M_DIGITAL_PULSE_ACTIVE_LOW, M_ANALOG_SNIPPET_DATA, M_DIGITAL_UINT8_BIT};

// JJD added data types July 2006                
// M_TTL_PULSE_ACTIVE_HIGH is meant to be an output type that will default to low
//                          and pulse to high for the duration specified in the data
// M_TTL_PULSE_ACTIVE_LOW ...                 


// ASK_DAVE --must think more about this.  What about waveforms?  words?

// We can always add more later. words are covered by M_DIGITAL_DATA as are bits  
// Also need to think about things like IMAGE, VOLUME and OPEN_DRAIN_TTL

enum IOIncompatibilityType{M_INDIVIDUAL_REQUEST_OKAY, M_UNNAMED_CAPABILITY, 
    M_NO_MATCHING_CAPABILITY, 
    M_CAPABILITY_NOT_AVAILABLE, 
    M_INCORRECT_DATA_DIRECTION, M_INCORRECT_DATA_TYPE, 
    M_UNSUPPORTED_SYNCHRONY_TYPE,
    M_SAMPLING_RATE_TOO_HIGH, M_DATA_RANGE_TOO_LOW, 
    M_DATA_RANGE_TOO_HIGH, M_DATA_RESOLUTION_TOO_HIGH
};




// A basic description of what a "port" on the device can do (may not be a 
// physical hardware port, just something that the device knows how to do)
class IOCapability {
    
protected:
    long					identifier;
    std::string				name;
    IODataDirection		data_direction;
    IODataType				data_type;
    IODataSynchronyType	synchrony_type;
    long					min_data_interval_usec;
    double					range_min;
    double					range_max;
    int						resolution_bits;
    bool					available;
    
public:
    IOCapability(long identifier, std::string name, 
                 IODataDirection dataDirection, IODataType dataType, 
                 IODataSynchronyType syncType, 
                 long _min_data_interval_usec, double _range_min, 
                 double _range_max, int _resolution);
    
    IOCapability(IOCapability& copy);
    ~IOCapability();
    
    long				getIdentifier();
    std::string			getName();
    IODataDirection    getDataDirection();
    IODataType         getDataType();
    IODataSynchronyType	getSynchronyType();
    long				getMinDataIntervalUsec();
    double				getRangeMin();
    double				getRangeMax();
    int					getResolution();
    bool				isAvailable();
    void				setAvailable(bool _available);		
    
};



// A request to bind a parameter to a named channel
class IOChannelRequest : public Lockable, public mw::Component {
    
protected:
    std::string             channel_name;
    shared_ptr<Variable>   parameter;
    std::string				requested_capability_name;
    IODataDirection		requested_data_direction;
    IODataType				requested_data_type;
    IODataSynchronyType	requested_synchrony_type;
    long					requested_data_interval_usec;
    long					requested_update_interval_usec;
    double					requested_range_min;
    double					requested_range_max;
    int						requested_resolution_bits;
    bool					resolved;
    
public:
    static const std::string CAPABILITY;
    static const std::string DATA_INTERVAL;
    static const std::string DIRECTION;
    static const std::string RANGE_MIN;
    static const std::string RANGE_MAX;
    static const std::string RESOLUTION;
    static const std::string SYNCHRONY;
    static const std::string DATA_TYPE;
    static const std::string UPDATE_INTERVAL;
    static const std::string VARIABLE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit IOChannelRequest(const ParameterValueMap &parameters);
    
    IOChannelRequest(IOChannelRequest& copy); 
    virtual ~IOChannelRequest();
    
    std::string				getChannelName();
    shared_ptr<Variable>	getVariable();
    std::string				getRequestedCapabilityName();
    IODataDirection		getRequestedDataDirection();
    IODataType				getRequestedDataType();
    IODataSynchronyType	getRequestedSynchronyType();
    long					getRequestedDataIntervalUsec();
    long					getRequestedUpdateIntervalUsec();
    double					getRequestedRangeMin();
    double					getRequestedRangeMax();
    int						getRequestedResolution();
    
    void setRequestedDataDirection(IODataDirection _direction);
    void setRequestedDataType(IODataType _type);
    void setRequestedDataIntervalUsec(long _interval);
    void setRequestedUpdateIntervalUsec(long _interval);
    void setRequestedRangeMin(double _min);
    void setRequestedRangeMax(double _max);
    void setRequestedResolution(int _bits);
    
    bool			isResolved();
    void			setResolved(bool _resolved);  
    
    // JJD added Jan 2007
    virtual IOChannelRequest *clone(){
        return new IOChannelRequest(*this);
    } 
    
};


// A generated link between an advertised capability and a parameter (variable)
// Think about this as a capability of the DEVICE (not the core)
class IOChannel : public Lockable, public boost::enable_shared_from_this<IOChannel> {
    
protected:
    IOChannelRequest *    request;                       // (the name of the channel is the request name)
    IOCapability * 		capability;                    // advertised capability of the device that has been designated as this channel
    shared_ptr<Variable>			variable;                     // the variable (parameter) in the core that is assigned to the channel
    
    // This info is redundant with the values in request (which is the
    // request that was already deemed to be satisfiable).  Also, sample
    // rate is no more special than any other field in the request...
    
    //long				nominal_data_interval_usec;     // this should not change once set by hardware init -- it is what the hardware thinks it can provide on the channel (it may differ from the requested data interval) 
    //long				actual_data_interval_usec;      // these should contain the value currently estimated for the channel (e.g. by the scheduler -- this is useful for timing checks)
    
    bool				initialized;                    // true if the channel has been initialized at the hardware level
    bool				active;                         // true if the channel is transmitting (or receiving)
    std::string			name;
    
    // update the channel (put data into the parameter value)     
    virtual void		update(Datum data);             
    virtual void		update(Datum, MWTime timeUS);
    virtual void		update(MWTime timeUS);	  
    
public:
    IOChannel(IOChannelRequest *  _request, 
              IOCapability *  _capability);
    IOChannel(IOChannel& copy);
    virtual ~IOChannel();
    
    std::string							getName();
    void								setName(std::string _name);
    IOChannelRequest * 		getRequest();
    IOCapability * 			getCapability();
    shared_ptr<Variable>				getVariable();
    long								getActualDataIntervalUsec();
    long				getCapabilityIdentifier(); 
    virtual bool		initialize(); 
    virtual bool		isInitialized();
    virtual bool		isActive();             // use this to check if the channel is active (i.e. passing data)
    virtual bool		activate();             // use this to 
    virtual bool		deactivate();
    
};



// A class to describe requests that could not be matched up with an IO capability
// The (tentative) idea here is that a users sends IOChannelRequests to the IODevice, the IODevices does its
// best to match those up with IOCapabilities, and anything that it couldn't accomodate (either exactly, or at all), gets
// registered as an IOChannelIncompatibility
class IOChannelIncompatibility{
protected:
    IOChannelRequest *  request;            // the request that could not be satisfied perfectly
    IOChannelRequest *   modified_request;  // a modified version of the request that could actually work
    
    ExpandableList<IOIncompatibilityType> *incompatibilities; // an index of what the problem was for the request
    
public:
    
    IOChannelIncompatibility(IOChannelRequest *  _request, 
                             IOChannelRequest *  _modified_request);
    IOChannelIncompatibility(IOChannelRequest *  _request, 
                             IOChannelRequest *  _modified_request, 
                             IOIncompatibilityType _incompatabilityType);
    IOChannelIncompatibility(IOChannelRequest *  _request, 
                             IOChannelRequest *  _modified_request, 
                             ExpandableList<IOIncompatibilityType>  *_incompatabilities);
    IOChannelIncompatibility(IOChannelIncompatibility& copy);
    ~IOChannelIncompatibility();
    
    IOChannelRequest *  getRequest();
    IOChannelRequest *  getModifiedRequest();
    
    ExpandableList<IOIncompatibilityType> *getIncompatibilityTypes();
    
    void setRequest(IOChannelRequest *  _request);
    void setModifiedRequest(IOChannelRequest *  _modified_request);
    void setIncompatibilities(ExpandableList<IOIncompatibilityType> *_incompatabilities);
    void addIncompatibilityType(IOIncompatibilityType _type);
};


// a short description of an available IOdevice
// used by core to survey available devices for selection.
//   instances of this are filled by the GetAvailableIODevices method in IODevice and added to a list of available devices
class IOPhysicalDeviceReference{
protected:
    long			uniqueID;         // must be greater than 0 to be a valid device reference  
    std::string		name;
    std::string		description;
    bool			available;
    
public:
    IOPhysicalDeviceReference(long _uniqueID, std::string _name);       // constructor
    IOPhysicalDeviceReference(const IOPhysicalDeviceReference& copy);
    ~IOPhysicalDeviceReference();
    
    bool    isAvailable();
    void	setAvailable(bool _available);
    
    long    getUniqueID();
    void    setUniqueID(long _uniqueID);
    
    std::string		getName();
    void			setName(std::string _name);
    std::string		getDescription();
    void			setDescription(std::string _description);
};



// Each new IO device should ultimately inherit from this base class.  It specifies the allowable interactions between the device and the core.
// At user request, the core will instantiate an IODevice object for a requested device type (e.g. a new ITC18_IODevice object)
// All interactions with the device will occur through that object using the public calls listed below.
// Think of this instantiation as a request for an interpreter to speak with any device of this type (e.g. any ITC18)
// As part of its construction, the first instance should go out and figure out what devices of this type are avaiable (i.e. how many ITC18s are there?)
// (This should ideally be done only once at first instance).  
// This will create a single static object that is part of the plugin static memory.  It keeps track of which devices of this type are out there and where are currently
// in use, and which are currently available.
// The core can ask for this information, and request that its current IODevice object be attached to either a specific device or to the next available free device on the list
// When an IODevice object is destructed, it should update the static object to say that it is now available.
//
// Ultimately, the unit of negotiation is called a "channel".  This specifies a mapping between a device.
// Assumptions here:   for now, we assume that requests for channels will not be done after initialization
class LegacyIODevice : public Lockable, public IODevice {
    
protected:
    IOPhysicalDeviceReference*					attached_device;            // this points to an object that contains info about the specific device that is attached to this object
    ExpandableList<IOCapability>              *capabilities;              // pointer to list of all advertised capabilities of the specific device
    ExpandableList<IOChannelRequest>          *pending_channel_requests;  // pointer to list of all requests that are currently unmet
    ExpandableList<IOChannel>                 *channels;                  // pointer to list of all established channels (links between params and devices)
    std::vector<boost::shared_ptr<ScheduleTask> >   schedule_nodes;            // pointer to list of all nodes that should be run by the scheduler (e.g. get latest data)
    ExpandableList<IOChannelIncompatibility>  *incompatibilities;         // pointer to list of all alterations and failures that occured during the fulfillment of the IOChannelRequest's
    Lockable	channels_lock;
    Lockable	schedule_nodes_lock;
    
    bool    registerCapability(IOCapability *  capability);      // add a capability to the list of available capabilities of the specific device           
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<mw::Component> child);
    
    int     matchCapabilityByName(std::string name);                  // will use this to campare avaialble capability names to a requested capability name
    bool	startDeviceIOchannels();        		
    virtual bool validateIndividualChannel(
                                           IOChannel *  tentative_channel, 
                                           IOChannelIncompatibility *  incomp);
    virtual bool validateMultipleChannels(ExpandableList<IOChannel> *candidates);
    virtual IOChannel *  makeNewChannel(
                                        IOChannelRequest *  _request, 
                                        IOCapability *  _capability);
    
    
public:
    explicit LegacyIODevice(const ParameterValueMap &parameters);
    LegacyIODevice();
    LegacyIODevice(const LegacyIODevice& copy);   
    virtual ~LegacyIODevice();
    
    // the following calls are in the order that they are expected to be called from the core.
    // deviations from this calling order that are not easily handled by the device plugin should return null and throw an exception to the message system (Printf)
    
    // find and return available devices of this type
    //  -- this function can do this in any way that it likes, but it should return a list (may be of length 0)
    //  (NOTE: the memory for this list may not live with the object, but could be static in the plugin)
    virtual ExpandableList<IOPhysicalDeviceReference> *getAvailablePhysicalDevices(); 
    
    // the attach functions should do two things:
    //          1) prevent this attached device from showing up on the available list in future calls to GetAvailableIODevices 
    //                  (e.g. move the device from the available list to a busy list)
    //          2) assign the IOdevice index so that all calls to THIS object will go to the specified device
    //                  (e.g. set a pointer to the requested device)
    virtual bool attachPhysicalDevice();                                      // attach next avaialble device to this object
    virtual bool attachPhysicalDevice(
                                      IOPhysicalDeviceReference* deviceToAttach);    // attach a specific device to this object
    virtual IOPhysicalDeviceReference* getAttachedPhysicalDevice();                    // see which specific device this object is currently attached to (if any)
    virtual bool isAttached();
    
    // -----------------------------------------------------------------------------------------------------------
    // all of the following functions are meant to apply to a specific attached device
    // if a specific device is not attached (i.e. if deviceUniqueID has not been spedified), they should return nulls
    
    // Find out what a specific device can do -- build a list of capabilities
    virtual ExpandableList<IOCapability> *getCapabilities();
    
    // Set up which parts of the IO device we need
    // the idea here is that we make all our requests and then call initializeChannels (below) to try to fill those requests
    // we may want to add requests down the road.  The IO device will have to either force the user to make ALL requests again (an renegotiate), or keep a list of old requests itself
    virtual bool requestChannel(shared_ptr<IOChannelRequest>  request);
    
    
    // Attempt to match up all requests with capabilities, warn if something is not possible
    // This function should create an IOChannel object for each channel assigned on the device and an IOCahnnelIncompatability object for each unmet requests
    // This function should reset any previous initialization before trying to match and initialize capabilities
    // A false return means that at least one request could not be met.
    virtual bool mapRequestsToChannels();       // matching of requests and capabilities --> creation of new (uninitialized) channels
    
    
    virtual bool initializeChannels();          // hardware initialization of all channels that were successfully matched (created by mapRequestsToChannels)
    
    // this is a general function for turning things on or off (it may not do anything for some devices) -- e.g. turn on x-ray
    virtual bool initialize();
    
    //virtual bool assignChannel(IOChannelRequest *request, IOCapability *capability);
    
    // this functions will start actual functioning of a device (input/output from/to all negotiated channels)
    //  this activity will continue until stopDeviceIO() is called (nice) or shutdown()
    virtual bool startDeviceIO();
    virtual bool stopDeviceIO();
    
    // these calls should be used to update the channels 
    virtual bool updateChannel(int channel_index);
    // optionally update an output channel with immediate data
    // should default to updateChannel(int) on input channels
    virtual bool updateChannel(int channel_index, Datum data);
    // update everything
    virtual bool updateAllChannels();
    
    
    // Managing and querying the device
    virtual ExpandableList<IOChannel> *getChannels();                             // allow core to get list of all assigned channels 
    virtual ExpandableList<IOChannelIncompatibility> *getIncompatibilities();     // allow core to get list of all unmet cahnnel requests
    
    virtual int getChannelIndex(std::string channel_name);                                // allow core to use a channel name to get the channel index
    
    virtual IOChannel *  getChannel(std::string channel_name);                             // allow core to get a pointer to a specific channel object (ref by name)
    virtual IOChannel *  getChannel(int channel_index);                              // allow core to get a pointer to a specific channel object (ref by index)
    
    virtual bool deactivateChannel(std::string channel_name);     // Allow core to block data flow through a specific channel -- data posted after this call will simply be discarded.
    virtual bool deactivateChannel(int channel_index);      // Think of these as simply blocking the data flow (i.e. the device may still be collecting data on this channel)
    
    virtual bool activateChannel(std::string channel_name);       // Allow core to restart datadata flow through a specific channel
    virtual bool activateChannel(int channel_index);
    
    virtual bool removeChannel(std::string channel_name);         // Allow the core to halt any data flow into or out of the channel (at the hardware level).  However, some devices 
    virtual bool removeChannel(int channel_index);          // (e.g. ITC18) may have to be reinitialized to actually do this.
    
    virtual bool stopAllScheduleNodes();
    
};


/**
 *  A VariableNotification object that calls back into the IODevice to
 *  let it know that an asynchronous output channel needs update (because
 *  the variable it is bound to has changed
 */

class AsynchronousOutputNotification : public VariableNotification{
    
protected:
    
    weak_ptr<LegacyIODevice> device;
    int channel_index;
    
public:
    
    AsynchronousOutputNotification( shared_ptr<LegacyIODevice> _device, int _channel_index);
    
    virtual void notify(const Datum& data, MWTime timeUS);
    
};


// A function and typedef for the purpose of scheduling channel updates
class UpdateIOChannelArgs {
public:
    weak_ptr<LegacyIODevice> device;
    int channel_index;
};

void *update_io_channel(const shared_ptr<UpdateIOChannelArgs> &args);
void *update_all_io_channels(void *void_args);


END_NAMESPACE_MW


#endif























