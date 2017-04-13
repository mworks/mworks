//
//  FirmataSerialConnection.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataSerialConnection_hpp
#define FirmataSerialConnection_hpp

#include "FirmataConnection.hpp"


BEGIN_NAMESPACE_MW


class FirmataSerialConnection : public FirmataConnection {
    
public:
    explicit FirmataSerialConnection(const std::string &path);
    ~FirmataSerialConnection();
    
    bool connect() override;
    void disconnect() override;
    
    ssize_t read(std::uint8_t &data, std::size_t size) override;
    bool write(const std::vector<std::uint8_t> &data) override;
    
private:
    const std::string path;
    SerialPort serialPort;
    
};


END_NAMESPACE_MW


#endif /* FirmataSerialConnection_hpp */
