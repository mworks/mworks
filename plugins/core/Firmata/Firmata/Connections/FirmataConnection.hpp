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


class FirmataConnectionClient {
    
public:
    using PinModesMap = std::map<std::uint8_t, std::map<std::uint8_t, std::uint8_t>>;
    using AnalogChannelPinMap = std::map<std::uint8_t, std::uint8_t>;
    using PortStateArray = std::array<std::uint8_t, 2>;
    
    virtual ~FirmataConnectionClient() { }
    
    virtual const std::string & getDeviceName() const = 0;
    virtual MWTime getReconnectInterval() const = 0;
    
    virtual void receivedProtocolVersion(std::uint8_t protocolVersionMajor, std::uint8_t protocolVersionMinor) = 0;
    virtual void receivedCapabilityInfo(const PinModesMap &modesForPin) = 0;
    virtual void receivedAnalogMappingInfo(const AnalogChannelPinMap &pinForAnalogChannel) = 0;
    virtual void receivedDigitalMessage(std::uint8_t portNum, const PortStateArray &portState, MWTime time) = 0;
    virtual void receivedAnalogMessage(std::uint8_t channelNumber, int value, MWTime time) = 0;
    
    virtual void disconnected() = 0;
    virtual void reconnected() = 0;
    
};


class FirmataConnection : boost::noncopyable {
    
public:
    static std::unique_ptr<FirmataConnection> create(FirmataConnectionClient &client,
                                                     const ParameterValue &serialPortPath,
                                                     const ParameterValue &bluetoothLocalName);
    
    explicit FirmataConnection(FirmataConnectionClient &client);
    virtual ~FirmataConnection() { }
    
    bool initialize();
    bool sendData(const std::vector<std::uint8_t> &data) { return write(data); }
    void finalize();
    
private:
    void run();
    bool receiveData();
    void reconnect();
    
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    
    virtual ssize_t read(std::uint8_t &data, std::size_t size) = 0;
    virtual bool write(const std::vector<std::uint8_t> &data) = 0;
    
    FirmataConnectionClient &client;
    
    const boost::shared_ptr<Clock> clock;
    
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool running;
    std::thread runThread;
    
};


END_NAMESPACE_MW


#endif /* FirmataConnection_hpp */
