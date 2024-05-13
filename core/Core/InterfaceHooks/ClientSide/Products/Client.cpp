/**
* DefaultClient.cpp
 *
 * History:
 * paul on 1/26/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "Client.h"

#include <boost/filesystem/path.hpp>

#include "LoadingUtilities.h"
#include "StandardVariables.h"
#include "SystemEventFactory.h"
#include "ZeroMQUtilities.hpp"


BEGIN_NAMESPACE_MW


#define CLIENT_INTERNAL_KEY	"MWClientInternalCallbackKey"


Client::Client() :
    RegistryAwareEventStreamInterface(M_CLIENT_MESSAGE_DOMAIN),
    incoming_event_buffer(new EventBuffer),
    outgoing_event_buffer(new EventBuffer),
    connectedEventReceived(false)
{
    registry = boost::make_shared<VariableRegistry>(outgoing_event_buffer);
	initializeStandardVariables(registry);
}


Client::~Client() {
    disconnectClient();
    
    if (incoming_listener) {
        incoming_listener->killListener();
    }
}


void Client::handleEvent(shared_ptr<Event> evt) { 
	int code = evt->getEventCode();
    
    switch (code) {
        case RESERVED_CODEC_CODE:
            registry->updateFromCodecDatum(evt->getData());
            // Request updated values for all variables
            putEvent(SystemEventFactory::requestVariablesUpdateControl());
            break;
            
        case RESERVED_SYSTEM_EVENT_CODE:
            handleSystemEvent(evt->getData());
            break;
            
        case RESERVED_TERMINATION_CODE:
            mwarning(M_CLIENT_MESSAGE_DOMAIN, "Received termination notification from server; disconnecting");
            (void)disconnectClient();
            break;
            
        default:
            break;
    }
    
    handleCallbacks(evt);

    if(code >= N_RESERVED_CODEC_CODES && code < registry->getNVariables() + N_RESERVED_CODEC_CODES) {
        shared_ptr <Variable> var = registry->getVariable(code);
        if(var) {
            var->setSilentValue(evt->getData());
		} else {
			mwarning(M_CLIENT_MESSAGE_DOMAIN, "Invalid variable, client not processing event code: %d", code);
		}
	}
}


void Client::handleSystemEvent(const Datum &sysEvent) {
    switch (sysEvent.getElement(M_SYSTEM_PAYLOAD_TYPE).getInteger()) {
        case M_SERVER_CONNECTED_CLIENT: {
            long clientID = sysEvent.getElement(M_SYSTEM_PAYLOAD).getInteger();
            std::lock_guard<std::mutex> lock(connectedEventReceivedMutex);
            if (remoteConnection &&
                clientID == reinterpret_cast<long>(remoteConnection.get()))
            {
                // Received connection acknowledgement from server.  Notify any thread waiting
                // in connectToServer.
                connectedEventReceived = true;
                connectedEventReceivedCondition.notify_all();
            }
            break;
        }
            
        case M_REQUEST_MEDIA_FILE: {
            if (experimentPackager) {
                auto mediaFileRequestPayload = sysEvent.getElement(M_SYSTEM_PAYLOAD);
                auto packagedMediaFile = experimentPackager->createMediaFilePackage(mediaFileRequestPayload);
                if (!packagedMediaFile.isUndefined()) {
                    putEvent(boost::make_shared<Event>(RESERVED_SYSTEM_EVENT_CODE, packagedMediaFile));
                }
            }
            break;
        }
            
        default:
            break;
    }
}


void Client::startEventListener() {
    incoming_listener = boost::make_shared<EventListener>(incoming_event_buffer, shared_from_this());
    incoming_listener->startListener();
}

bool Client::connectToServer(const std::string &host, const int port) {
    std::unique_lock<std::mutex> lock(connectedEventReceivedMutex);
    
    // ZeroMQ doesn't seem to resolve mDNS (aka Bonjour) hostnames correctly, so we resolve
    // the host to a numeric address ourselves
    std::string address;
    if (!zeromq::resolveHostname(host, address)) {
        return false;
    }
    
    remoteConnection.reset(new ZeroMQClient(incoming_event_buffer,
                                            outgoing_event_buffer,
                                            zeromq::formatTCPEndpoint(address, port),
                                            zeromq::formatTCPEndpoint(address, port + 1)));
    if (!remoteConnection->connect()) {
        //TODO log the error somewhere.
        return false; 
    }
    
    // Send client connected event, using memory address of remoteConnection as the client ID
    putEvent(SystemEventFactory::clientConnectedToServerControl(reinterpret_cast<long>(remoteConnection.get())));
    
    // Wait for connection acknowledgement from server
    connectedEventReceived = false;
    if (!connectedEventReceivedCondition.wait_for(lock,
                                                  std::chrono::seconds(2),
                                                  [this]() { return connectedEventReceived; }))
    {
        (void)disconnectClient();
        return false;
    }
    
    // Request component codec, variable codec, experiment state, and protocol names
    putEvent(SystemEventFactory::requestComponentCodecControl());
    putEvent(SystemEventFactory::requestCodecControl());
    putEvent(SystemEventFactory::requestExperimentStateControl());
    putEvent(SystemEventFactory::requestProtocolsControl());
    
    return true;
}

bool Client::disconnectClient() {
    if (remoteConnection && remoteConnection->isConnected()) {
        remoteConnection->disconnect();
        // Destroy connection to ensure that ZeroMQ sockets are closed
        remoteConnection.reset();
    }
    return true;
}

bool Client::isConnected() {
    if (remoteConnection) {
        return remoteConnection->isConnected();
    }
    return false;
}

void Client::putEvent(shared_ptr<Event> event) {
    outgoing_event_buffer->putEvent(event);
}

bool Client::sendExperiment(const std::string &expPath) {
    if (!remoteConnection->isConnected()) {
        return false;
    }
    
    experimentPackager = std::make_unique<ExperimentPackager>();
    auto packaged_experiment = experimentPackager->packageExperiment(boost::filesystem::path(expPath));
    if (packaged_experiment.isUndefined()) {
        merror(M_CLIENT_MESSAGE_DOMAIN, "Failed to create a valid packaged experiment.");
        experimentPackager.reset();
        return false;
    }
    
    putEvent(boost::make_shared<Event>(RESERVED_SYSTEM_EVENT_CODE, packaged_experiment));
    return true;
}

void  Client::sendProtocolSelectedEvent(const std::string &protocolname) {
    if(protocolname.size() == 0) { return; }
    putEvent(SystemEventFactory::protocolSelectionControl(protocolname));
}

void  Client::sendRunEvent() {
    putEvent(SystemEventFactory::startExperimentControl());
}

void  Client::sendStopEvent() {
    putEvent(SystemEventFactory::stopExperimentControl());
}

void  Client::sendPauseEvent() {
    putEvent(SystemEventFactory::pauseExperimentControl());
}

void  Client::sendResumeEvent() {
    putEvent(SystemEventFactory::resumeExperimentControl());
}

void Client::sendOpenDataFileEvent(const std::string &filename, bool overwrite) {
    if(filename.size() == 0) { return; }
    putEvent(SystemEventFactory::dataFileOpenControl(filename, overwrite));
}

void  Client::sendCloseDataFileEvent(const std::string &filename) {
    putEvent(SystemEventFactory::closeDataFileControl(filename));
}

void  Client::sendCloseExperimentEvent(const std::string &expName) {
    if(expName.size() == 0) { return; }
    putEvent(SystemEventFactory::closeExperimentControl(expName));
}

void  Client::sendSaveVariablesEvent(const std::string &file, const bool overwrite) {
    putEvent(SystemEventFactory::saveVariablesControl(file, overwrite, false));
}

void  Client::sendLoadVariablesEvent(const std::string &file) {
    putEvent(SystemEventFactory::loadVariablesControl(file, false));
}

void Client::updateRegistry(const Datum &codec) {
	registry->updateFromCodecDatum(codec);
}


void Client::updateValue(const int code, const Datum &data) {
	shared_ptr<Event> event(new Event(code, data));
	putEvent(event);
}


END_NAMESPACE_MW
