//
//  FirmataBluetoothLEConnection.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataBluetoothLEConnection_hpp
#define FirmataBluetoothLEConnection_hpp

#include "FirmataConnection.hpp"


@class MWKFirmataBluetoothLEDelegate;  // Forward declaration


BEGIN_NAMESPACE_MW


class FirmataBluetoothLEConnection : public FirmataConnection {
    
    static void notify(FirmataBluetoothLEConnection *connection);
    static void dataReceived(FirmataBluetoothLEConnection *connection, const std::uint8_t *data, std::size_t size);
    
public:
    using NotifyCallback = decltype(&notify);
    using DataReceivedCallback = decltype(&dataReceived);
    
    explicit FirmataBluetoothLEConnection(const std::string &localName);
    ~FirmataBluetoothLEConnection();
    
    const std::string& getLocalName() const { return localName; }
    
    bool connect() override;
    void disconnect() override;
    
    ssize_t read(std::uint8_t &data, std::size_t size) override;
    bool write(const std::vector<std::uint8_t> &data) override;
    
private:
    using unique_lock = std::unique_lock<std::mutex>;
    
    bool wait(unique_lock &lock);
    void notify();
    void dataReceived(const std::uint8_t *data, std::size_t size);
    
    const std::string localName;
    MWKFirmataBluetoothLEDelegate *delegate;
    std::deque<std::uint8_t> receivedData;
    
    unique_lock::mutex_type mutex;
    std::condition_variable condition;
    bool wasNotified;
    
};


END_NAMESPACE_MW


#endif /* FirmataBluetoothLEConnection_hpp */



























