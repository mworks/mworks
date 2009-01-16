/**
 * EventFactory.h
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
namespace mw {
typedef enum {
    M_OVERWRITE         = 1000,
    M_NO_OVERWRITE      = 1001,
} DataFileOptions;

class EventFactory {
    public:  

    /*********************************************************
		*          Data Package Events
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
	//NOTES
	/**
		* Creates an event that tells the server to open up filename.
	 * data file open is a control event not to be confused with
	 * M_DATAFILE_PACKAGE which is used to send the data file back to the
	 * client. opt is just an enum  for now but could become a logical 
	 * ORing of options if we think of anything else.
	 */
	
	static shared_ptr<Event> protocolSelectionControl(std::string);
	static shared_ptr<Event> startExperimentControl();
	static shared_ptr<Event> stopExperimentControl();
	static shared_ptr<Event> pauseExperimentControl();
	static shared_ptr<Event> dataFileOpenControl(std::string  filename, 
												   DataFileOptions opt);
	static shared_ptr<Event> closeDataFileControl(std::string filename);
	static shared_ptr<Event> closeExperimentControl(std::string);
	static shared_ptr<Event> saveVariablesControl(const std::string &file,
												   const bool overwrite,
												   const bool fullPath);
	static shared_ptr<Event> loadVariablesControl(const std::string &file,
												   const bool fullPath);
	
    /*********************************************************
		*          Response Package Events
		********************************************************/ 
	static shared_ptr<Event> dataFileOpenedResponse(std::string, SystemEventResponseCode code);
	static shared_ptr<Event> dataFileClosedResponse(std::string, SystemEventResponseCode code);

	// these will eventually need to contain the client address and
	// maybe even port for stat tracking and other fun stuff
	// for now they dont
	static shared_ptr<Event> serverConnectedClientResponse();
	static shared_ptr<Event> serverDisconnectClientResponse();
	static shared_ptr<Event> clientConnectedToServerResponse();
	static shared_ptr<Event> clientDisconnectedFromServerResponse();
	
	// this event sends the current state of the world to the data stream 
	// (and therefore, clients).
	static shared_ptr<Event> currentExperimentState();
	
public:
        
        /*******************************************************
         *      Response Event Package Inspector Methods
         ******************************************************/
        // all response events must have a susscess/failure code
        // in the first payload slot. returns -1 on error or a
        // SystemEventResponseCode on success.
        static int responseEventCommandCode(ScarabDatum * payload);

        
		// Generic system event building methods
		static Data systemEventPackage(SystemEventType type, 
										SystemPayloadType,
										Data payload);
        static Data systemEventPackage(SystemEventType type, 
										SystemPayloadType);
		
    protected:

        // this class is just meant to have the factory methods called
        // and not be constructed.
        EventFactory() { }
        EventFactory(const EventFactory& ) { }
        void operator=(const EventFactory& ef) { }
};
}
#endif
