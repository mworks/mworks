//
//  NE500SerialConnection.hpp
//  NE500Plugin
//
//  Created by Christopher Stawarz on 3/29/16.
//
//

#ifndef NE500SerialConnection_hpp
#define NE500SerialConnection_hpp

#include "NE500Connection.hpp"


BEGIN_NAMESPACE_MW


class NE500SerialConnection : public NE500Connection {
    
public:
    explicit NE500SerialConnection(const std::string &path) :
        path(path),
        serialPort(false)  // Non-blocking I/O
    { }
    
    bool connect() override;
    bool disconnect() override;
    
    bool send(const std::string &command) override;
    bool receive(std::string &response) override;
    
private:
    const std::string path;
    SerialPort serialPort;
    
};


END_NAMESPACE_MW


#endif /* NE500SerialConnection_hpp */
