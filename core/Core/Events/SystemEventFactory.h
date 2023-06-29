/**
 * SystemEventFactory.h
 *
 * Description: An object whose sole purpose is to create events.
 *
 * History:
 * Paul Jankunas on 5/3/05 - Created.
 * Paul Jankunas on 05/12/05 - Added codecPackageEvent().
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef M_EVENT_FACTORY_H__
#define M_EVENT_FACTORY_H__


#include "Event.h"
#include "GenericData.h"
#include "VariableRegistry.h"
#include <string>
#include <boost/filesystem/path.hpp>


BEGIN_NAMESPACE_MW


class SystemEventFactory {
    public:  

    /*********************************************************
		*          Datum Package Events
		********************************************************/
	//NOTE experiment packages are handled by their own
	// object. 
	static shared_ptr<Event> codecPackage();
    static shared_ptr<Event> codecPackage(shared_ptr<VariableRegistry> registry);
	static shared_ptr<Event> componentCodecPackage();
	static shared_ptr<Event> protocolPackage();
	
    /*********************************************************
		*          Control Package Events
		********************************************************/        
	static shared_ptr<Event> protocolSelectionControl(std::string);
	static shared_ptr<Event> startExperimentControl();
	static shared_ptr<Event> stopExperimentControl();
	static shared_ptr<Event> pauseExperimentControl();
	static shared_ptr<Event> resumeExperimentControl();
    static shared_ptr<Event> requestCodecControl();
    static shared_ptr<Event> requestComponentCodecControl();
    static shared_ptr<Event> requestExperimentStateControl();
    static shared_ptr<Event> requestProtocolsControl();
	static shared_ptr<Event> dataFileOpenControl(const std::string &filename, bool overwrite);
	static shared_ptr<Event> closeDataFileControl(const std::string &filename);
	static shared_ptr<Event> closeExperimentControl(std::string);
	static shared_ptr<Event> saveVariablesControl(const std::string &file,
												   const bool overwrite,
												   const bool fullPath);
	static shared_ptr<Event> loadVariablesControl(const std::string &file,
												   const bool fullPath);
	
    static shared_ptr<Event> requestVariablesUpdateControl();
    
    static shared_ptr<Event> setEventForwardingControl(bool forwardAllEvents);
    static shared_ptr<Event> setEventForwardingControl(int eventCode, bool forward);
    static shared_ptr<Event> setEventForwardingControl(const std::string &eventName, bool forward);
    
    static shared_ptr<Event> clockOffsetEvent(MWTime offset_value);
    
    static shared_ptr<Event> connectedEvent();
    
    /*********************************************************
		*          Response Package Events
		********************************************************/ 
	static shared_ptr<Event> dataFileOpenedResponse(std::string, SystemEventResponseCode code);
	static shared_ptr<Event> dataFileClosedResponse(std::string, SystemEventResponseCode code);

	static shared_ptr<Event> serverConnectedClientResponse(long clientID);
    static shared_ptr<Event> clientConnectedToServerControl(long clientID);
	
	// this event sends the current state of the world to the data stream 
	// (and therefore, clients).
	static shared_ptr<Event> currentExperimentState();
	
public:
        
		// Generic system event building methods
		static Datum systemEventPackage(SystemEventType type, 
										SystemPayloadType,
									 Datum payload);
        static Datum systemEventPackage(SystemEventType type, 
										SystemPayloadType);
		
    protected:

        // this class is just meant to have the factory methods called
        // and not be constructed.
        SystemEventFactory() { }
        SystemEventFactory(const SystemEventFactory& ) { }
        void operator=(const SystemEventFactory& ef) { }
};


END_NAMESPACE_MW


#endif
