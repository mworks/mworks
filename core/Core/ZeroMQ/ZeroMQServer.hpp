//
//  ZeroMQServer.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/1/16.
//
//

#ifndef ZeroMQServer_hpp
#define ZeroMQServer_hpp

#include "ZeroMQConnection.hpp"
#include "ZeroMQSocketPair.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQServer {
    
public:
    ZeroMQServer(const boost::shared_ptr<EventBuffer> &incomingEventBuffer,
                 const boost::shared_ptr<EventBuffer> &outgoingEventBuffer,
                 const std::string &incomingSocketEndpoint,
                 const std::string &outgoingSocketEndpoint);
    
    bool start();
    bool stop();
    
    bool isStarted() const {
        return (incomingConnection.isRunning() && outgoingConnection.isRunning());
    }
    
private:
    ZeroMQSocketPair socketPair;
    ZeroMQIncomingConnection incomingConnection;
    ZeroMQOutgoingConnection outgoingConnection;
    
};


END_NAMESPACE_MW


#endif /* ZeroMQServer_hpp */
