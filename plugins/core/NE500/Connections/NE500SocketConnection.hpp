//
//  NE500SocketConnection.hpp
//  NE500Plugin
//
//  Created by Christopher Stawarz on 3/29/16.
//
//

#ifndef NE500SocketConnection_hpp
#define NE500SocketConnection_hpp

#include "NE500Connection.hpp"


BEGIN_NAMESPACE_MW


class NE500SocketConnection : public NE500Connection {
    
public:
    NE500SocketConnection(const std::string &address, int port) :
        address(address),
        port(port),
        s(-1)
    { }
    
    bool connect() override;
    bool disconnect() override;
    
    bool send(const std::string &command) override;
    bool receive(std::string &response) override;
    
private:
    const std::string address;
    const int port;
    
    int s;
    
};


END_NAMESPACE_MW


#endif /* NE500SocketConnection_hpp */
