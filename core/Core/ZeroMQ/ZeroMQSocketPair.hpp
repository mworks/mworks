//
//  ZeroMQSocketPair.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/9/21.
//

#ifndef ZeroMQSocketPair_hpp
#define ZeroMQSocketPair_hpp

#include "ZeroMQSocket.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQSocketPair {
    
public:
    enum class Type {
        Server,
        Client
    };
    
    ZeroMQSocketPair(Type type,
                     const std::string &incomingSocketEndpoint,
                     const std::string &outgoingSocketEndpoint);
    
    ZeroMQSocket & getIncomingSocket() { return incomingSocket; }
    ZeroMQSocket & getOutgoingSocket() { return outgoingSocket; }
    
    bool bind();
    bool unbind();
    
    bool connect();
    bool disconnect();
    
    ZeroMQSocket::Result send(const boost::shared_ptr<Event> &event) { return outgoingSocket.send(event); }
    ZeroMQSocket::Result recv(boost::shared_ptr<Event> &event) { return incomingSocket.recv(event); }
    
private:
    ZeroMQSocket incomingSocket;
    ZeroMQSocket outgoingSocket;
    const std::string incomingSocketEndpoint;
    const std::string outgoingSocketEndpoint;
    
};


END_NAMESPACE_MW


#endif /* ZeroMQSocketPair_hpp */
