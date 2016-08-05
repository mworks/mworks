
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


#include "VariableRegistryInterface.h"
#include "EventListener.h"
#include "ZeroMQClient.hpp"


BEGIN_NAMESPACE_MW


class Client : public RegistryAwareEventStreamInterface {
    private:
    const boost::shared_ptr<EventBuffer> incoming_event_buffer;
    const boost::shared_ptr<EventBuffer> outgoing_event_buffer;
    boost::shared_ptr<EventListener> incoming_listener;
	
    // a connection with a server
    std::unique_ptr<ZeroMQClient> remoteConnection;
    
    bool connectedEventReceived;
    std::mutex connectedEventReceivedMutex;
    std::condition_variable connectedEventReceivedCondition;
    
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
        ~Client();
        

		void handleEvent(shared_ptr<Event> event) override;
		
        void putEvent(shared_ptr<Event> event) override;
    
		void startEventListener();

        bool connectToServer(const std::string &host, const int port);

        /*!
         * @function disconnectClient
         * @discussion Disconnect the client from the server.
         * @result Always returns true.
         */
        bool disconnectClient();

        bool isConnected();

        bool sendExperiment(const std::string &expPath);

        void sendProtocolSelectedEvent(const std::string &protocolname);

        /*!
         * @function sendRunEvent
         * @discussion Puts a M_START_EXPERIMENT event into the outgoing
         * event stream.
         */
        void sendRunEvent();

        /*!
         * @function sendStopEvent
         * @discussion Puts a M_STOP_EXPERIMENT event into the outgoing 
         * event stream.
         */
        void sendStopEvent();

        /*!
         * @function sendPauseEvent
         * @discussion Puts a M_PAUSE_EXPERIMENT event into the outgoing 
         * event stream.
         */
        void sendPauseEvent();

        /*!
         * @function sendResumeEvent
         * @discussion Puts a M_RESUME_EXPERIMENT event into the outgoing 
         * event stream.
         */
        void sendResumeEvent();

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
        void sendOpenDataFileEvent(const std::string &filename, 
                                           const int options = 0);

        /*!
         * @function sendCloseDataFileEvent
         * @discussion Puts a M_CLOSE_DATA_FILE event into the outoging event 
         * stream asking the server to close the data file named filename.
         *
         * @param filename The name of a data file.
         */
        void sendCloseDataFileEvent(const std::string &filename);

        /*!
         * @function sendCloseExperimentEvent
         * @discussion Puts a M_CLOSE_EXPERIMENT event into the outgoing
         * event stream.  Asks the server to close the experiment named 
         * expName
         *
         * @param expName The name of an experiment.
         */
        void sendCloseExperimentEvent(const std::string &expName);
        
        /*!
         * @function sendSaveVariablesEvent
         * @discussion Puts a M_SAVE_VARIABLES event into the outgoing 
         * event stream.
         */
        void sendSaveVariablesEvent(const std::string &saveVarName, 
											const bool overwrite);
		
        /*!
         * @function sendLoadVariablesEvent
         * @discussion Puts a M_LOAD_VARIABLES event into the outgoing 
         * event stream.
         */
        void sendLoadVariablesEvent(const std::string &saveVarName);
		

		void updateValue(const int code, const Datum &data);
		void updateRegistry(const Datum &codec);
		
		unsigned int numberOfVariablesInRegistry() {
			return registry->getNVariables();
		}
		
            
};


END_NAMESPACE_MW


#endif