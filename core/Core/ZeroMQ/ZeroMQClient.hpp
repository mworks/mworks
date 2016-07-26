//
//  ZeroMQClient.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/26/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef ZeroMQClient_hpp
#define ZeroMQClient_hpp

#include "ZeroMQConnection.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQClient {
    
public:
    ZeroMQClient(const boost::shared_ptr<EventBuffer> &incomingEventBuffer,
                 const boost::shared_ptr<EventBuffer> &outgoingEventBuffer,
                 const std::string &incomingSocketEndpoint,
                 const std::string &outgoingSocketEndpoint);
    
    bool connect();
    bool disconnect();
    
    bool isConnected() const {
        return (incomingConnection.isRunning() && outgoingConnection.isRunning());
    }
    
private:
    ZeroMQSocket incomingSocket;
    ZeroMQSocket outgoingSocket;
    ZeroMQIncomingConnection incomingConnection;
    ZeroMQOutgoingConnection outgoingConnection;
    const std::string incomingSocketEndpoint;
    const std::string outgoingSocketEndpoint;
    
};


END_NAMESPACE_MW


#endif /* ZeroMQClient_hpp */



























