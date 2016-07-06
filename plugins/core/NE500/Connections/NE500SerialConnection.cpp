//
//  NE500SerialConnection.cpp
//  NE500Plugin
//
//  Created by Christopher Stawarz on 3/29/16.
//
//

#include "NE500SerialConnection.hpp"

#include <sys/ioctl.h>


BEGIN_NAMESPACE_MW


bool NE500SerialConnection::connect() {
    if (connected) {
        return true;
    }
    
    if (!serialPort.connect(path, B19200)) {
        return false;
    }
    
    connected = true;
    return true;
}


bool NE500SerialConnection::disconnect() {
    if (!connected) {
        return true;
    }
    
    serialPort.disconnect();
    
    connected = false;
    return true;
}


bool NE500SerialConnection::send(const std::string &command) {
    if (!connected) {
        return false;
    }
    
    if (-1 == serialPort.write(command)) {
        return false;
    }
    
    return true;
}


bool NE500SerialConnection::receive(std::string &response) {
    if (!connected) {
        return false;
    }
    
    std::array<char, 512> buffer;
    auto rc = serialPort.read(buffer);
    
    if (-1 == rc) {
        return false;
    }
    
    response.append(buffer.data(), rc);
    return true;
}


END_NAMESPACE_MW



























