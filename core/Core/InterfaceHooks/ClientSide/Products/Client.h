
/*!
 * @header Client.h
 *
 * @discussion 
 *
 * History:<BR/>
 * paul on 1/26/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/26/06
 * @version 1.0.0
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__


#include "EventStreamInterface.h"
#include "VariableRegistryInterface.h"
#include "IncomingEventListener.h"
#include "ScarabClient.h"
#include "VariableRegistry.h"
#include <boost/filesystem/path.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace mw {

class Client : public RegistryAwareEventStreamInterface {
    protected:
	shared_ptr<EventBuffer> incoming_event_buffer, outgoing_event_buffer;
	shared_ptr<EventListener> incoming_listener;
	
    // a connection with a server
	shared_ptr<ScarabClient> remoteConnection;
	// the return value of the last communication
	shared_ptr<NetworkReturn> lastNetworkReturn;
	
	shared_ptr<Variable> message_variable;
	
	std::vector <std::vector<shared_ptr<GenericEventFunctor> > > callbacks;
	
	//boost::mutex event_lock;
	boost::recursive_mutex callbacksLock;
	
    
    public:
        /*!
         * @function Client
         * @discussion Default Constructor.
         */
        Client();
        
		
        /*!
         * @function ~Client
         * @discussion Destructor.
         */
        virtual ~Client();
        

		virtual void handleEvent(shared_ptr<Event> event);
		
        virtual void putEvent(shared_ptr<Event> event);
    
		virtual void startEventListener();

        virtual bool connectToServer(const std::string &host, const int port);

        /*!
         * @function disconnectClient
         * @discussion Disconnect the client from the server.
         * @result Always returns true.
         */
        virtual bool disconnectClient();
 
	
        virtual void reset();
 

        virtual bool isConnected();

        virtual bool sendExperiment(const std::string &expPath);

        virtual void sendProtocolSelectedEvent(const std::string &protocolname);

        /*!
         * @function sendRunEvent
         * @discussion Puts a M_START_EXPERIMENT event into the outgoing
         * event stream.
         */
        virtual void sendRunEvent();

        /*!
         * @function sendStopEvent
         * @discussion Puts a M_STOP_EXPERIMENT event into the outgoing 
         * event stream.
         */
        virtual void sendStopEvent();

        /*!
         * @function sendPauseEvent
         * @discussion Puts a M_PAUSE_EXPERIMENT event into the outgoing 
         * event stream.
         */
        virtual void sendPauseEvent();

        /*!
         * @function sendResumeEvent
         * @discussion Puts a M_RESUME_EXPERIMENT event into the outgoing 
         * event stream.
         */
        virtual void sendResumeEvent();

        /*!
         * @function sendOpenDataFileEvent
         * @discussion Puts a M_OPEN_DATA_FILE event into the outgoing 
         * event stream.  Filename should be the name of a data file that
         * you want to create on the server side.  Options is currently
         * unused.
         *
         * @param filename A filename to open on the server
         * @param options Unused
         */
        virtual void sendOpenDataFileEvent(const std::string &filename, 
                                           const int options = 0);

        /*!
         * @function sendCloseDataFileEvent
         * @discussion Puts a M_CLOSE_DATA_FILE event into the outoging event 
         * stream asking the server to close the data file named filename.
         *
         * @param filename The name of a data file.
         */
        virtual void sendCloseDataFileEvent(const std::string &filename);

        /*!
         * @function sendCloseExperimentEvent
         * @discussion Puts a M_CLOSE_EXPERIMENT event into the outgoing
         * event stream.  Asks the server to close the experiment named 
         * expName
         *
         * @param expName The name of an experiment.
         */
        virtual void sendCloseExperimentEvent(const std::string &expName);
        
        /*!
         * @function sendSaveVariablesEvent
         * @discussion Puts a M_SAVE_VARIABLES event into the outgoing 
         * event stream.
         */
        virtual void sendSaveVariablesEvent(const std::string &saveVarName, 
											const bool overwrite);
		
        /*!
         * @function sendLoadVariablesEvent
         * @discussion Puts a M_LOAD_VARIABLES event into the outgoing 
         * event stream.
         */
        virtual void sendLoadVariablesEvent(const std::string &saveVarName);
		

		virtual void updateValue(const int code, const Datum &data);
		virtual void updateRegistry(const Datum &codec);
		
		virtual unsigned int numberOfVariablesInRegistry() {
			return registry->getNVariables();
		}
		
            
};
}
#endif