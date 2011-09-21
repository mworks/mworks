/*
 *  ServersideConduit.cpp
 *  ServersideConduit
 *
 */
 

#include "ServersideConduit.h"

namespace mw{


ServersideConduit::ServersideConduit(const ParameterValueMap &p) :
    IODevice(p),
    resource_name( (const char *) p[RESOURCE_NAME_PARAMETER] )
{  

    shared_ptr<Server> core = Server::instance(false);
    
    // TODO: generate a unique name to avoid name collisions
    shared_ptr<mw::IPCEventTransport> transport(new mw::IPCEventTransport(mw::EventTransport::server_event_transport, 
                                                                          mw::EventTransport::bidirectional_event_transport, 
                                                                          resource_name));

    // build the conduit, attaching it to the core/client's event stream 
    conduit = shared_ptr<mw::EventStreamConduit>(new mw::EventStreamConduit(transport, dynamic_pointer_cast<EventStreamInterface, Server>(core)));
    conduit->initialize();

}
    

ServersideConduit::~ServersideConduit(){ }


}
