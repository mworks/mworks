//
//  FirmataConnection.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataConnection_hpp
#define FirmataConnection_hpp


BEGIN_NAMESPACE_MW


class FirmataConnection : boost::noncopyable {
    
public:
    static std::unique_ptr<FirmataConnection> create(const ParameterValue &serialPortPath,
                                                     const ParameterValue &bluetoothLocalName);
    
    virtual ~FirmataConnection() { }
    
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    
    virtual ssize_t read(std::uint8_t &data, std::size_t size) = 0;
    virtual bool write(const std::vector<std::uint8_t> &data) = 0;
    
};


END_NAMESPACE_MW


#endif /* FirmataConnection_hpp */
