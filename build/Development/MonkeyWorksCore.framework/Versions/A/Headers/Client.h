
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


#include "EventHandler.h"
#include "IncomingEventListener.h"
#include "ScarabClient.h"
#include "CoreEventFunctor.h"
#include "VariableRegistry.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem/path.hpp>

namespace mw {

class Client : public EventHandler {
    protected:
	shared_ptr<BufferManager> buffer_manager;
	shared_ptr<IncomingEventListener> incomingListener;
	
	shared_ptr<VariableRegistry> registry;
	// a connection with a server
	shared_ptr<ScarabClient> remoteConnection;
	// the return value of the last communication
	shared_ptr<NetworkReturn> lastNetworkReturn;
	
	shared_ptr<Variable> system_event_variable;
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
        

		virtual void handleEvent(shared_ptr<Event> &event);
		
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
         * @function sendUnpauseEvent
         * @discussion Puts a M_PAUSE_EXPERIMENT event into the outgoing 
         * event stream.
         */
        virtual void sendUnpauseEvent();

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
		
		
		virtual shared_ptr<Variable> getVariable(const std::string &tag);
		virtual shared_ptr<Variable> getVariable(const int code);
		virtual int getCode(const std::string &tag);
		std::vector<std::string>getVariableNames();

		virtual void updateValue(const int code, const Data &data);
		virtual void updateRegistry(const Data &codec);
		
		virtual unsigned int numberOfVariablesInRegistry() {
			return registry->getNVariables();
		}
		
		void registerCallback(shared_ptr<GenericEventFunctor> gef);
		void registerCallback(shared_ptr<GenericEventFunctor> gef, int code);
		void Client::registerCallback(shared_ptr<GenericEventFunctor> gef,
							   string tagname);
		void registerCallback(shared_ptr<GenericEventFunctor> gef, std::vector <int> vars);
		
		void unregisterCallbacks(const std::string &callback_id);
};
}
#endif