//
//  FirmataDevice.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/10/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDevice.hpp"

#include "FirmataMessageTypes.h"
#include "FirmataServoChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string FirmataDevice::SERIAL_PORT("serial_port");
const std::string FirmataDevice::BLUETOOTH_LOCAL_NAME("bluetooth_local_name");
const std::string FirmataDevice::DATA_INTERVAL("data_interval");
const std::string FirmataDevice::CONNECTED("connected");
const std::string FirmataDevice::RECONNECT_INTERVAL("reconnect_interval");


void FirmataDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/firmata");
    
    info.addParameter(SERIAL_PORT, false);
    info.addParameter(BLUETOOTH_LOCAL_NAME, false);
    info.addParameter(DATA_INTERVAL, false);
    info.addParameter(CONNECTED, false);
    info.addParameter(RECONNECT_INTERVAL, "0");
}


FirmataDevice::FirmataDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    connection(FirmataConnection::create(*this, parameters[SERIAL_PORT], parameters[BLUETOOTH_LOCAL_NAME])),
    samplingIntervalUS(0),
    connectedVar(optionalVariable(parameters[CONNECTED])),
    reconnectIntervalUS(parameters[RECONNECT_INTERVAL]),
    deviceProtocolVersionReceived(false),
    deviceProtocolVersionMajor(0),
    deviceProtocolVersionMinor(0),
    capabilityInfoReceived(false),
    analogMappingInfoReceived(false),
    connected(false),
    running(false)
{
    if (!(parameters[DATA_INTERVAL].empty())) {
        samplingIntervalUS = MWTime(parameters[DATA_INTERVAL]);
        if (samplingIntervalUS < 1000) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Data interval must be >=1ms");
        } else if (samplingIntervalUS % 1000 != 0) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Data interval must be a whole number of milliseconds");
        }
    }
    if (reconnectIntervalUS < 0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Reconnection interval must be >=0");
    }
}


FirmataDevice::~FirmataDevice() {
    connection->finalize();
}


void FirmataDevice::addChild(std::map<std::string, std::string> parameters,
                             ComponentRegistryPtr reg,
                             boost::shared_ptr<Component> child)
{
    auto channel = boost::dynamic_pointer_cast<FirmataChannel>(child);
    if (!channel) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for Firmata device");
    }
    requestedChannels.push_back(channel);
}


bool FirmataDevice::initialize() {
    unique_lock lock(mutex);
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Configuring Firmata device \"%s\"...", getTag().c_str());
    
    if (!connection->initialize()) {
        return false;
    }
    
    setConnected(true);

    if (!checkProtocolVersion(lock) ||
        !getDeviceInfo(lock) ||
        !processChannelRequests() ||
        !configurePins())
    {
        connection->finalize();
        setConnected(false);
        return false;
    }
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Firmata device \"%s\" is ready", getTag().c_str());
    
    return true;
}


bool FirmataDevice::startDeviceIO() {
    unique_lock lock(mutex);
    
    if (!running) {
        if (!startIO()) {
            return false;
        }
        running = true;
    }
    
    return true;
}


bool FirmataDevice::stopDeviceIO() {
    unique_lock lock(mutex);
    
    if (running) {
        if (!stopIO()) {
            return false;
        }
        running = false;
    }
    
    return true;
}


bool FirmataDevice::checkProtocolVersion(unique_lock &lock) {
    if (!sendData({ REPORT_VERSION })) {
        return false;
    }
    
    // Wait for receipt of protocol version
    if (!condition.wait_for(lock, std::chrono::seconds(2), [this]() { return deviceProtocolVersionReceived; })) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Firmata device \"%s\" did not respond to request for protocol version",
               getTag().c_str());
        return false;
    }
    
    // Confirm that device uses a compatible protocol version
    if (deviceProtocolVersionMajor != protocolVersionMajor ||
        deviceProtocolVersionMinor < protocolVersionMinor)
    {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Firmata device \"%s\" uses protocol version %hhu.%hhu, which is not compatible with the version "
               "used by MWorks (%hhu.%hhu)",
               getTag().c_str(),
               deviceProtocolVersionMajor,
               deviceProtocolVersionMinor,
               protocolVersionMajor,
               protocolVersionMinor);
        return false;
    }
    
    return true;
}


bool FirmataDevice::getDeviceInfo(unique_lock &lock) {
    if (!sendData({ START_SYSEX, CAPABILITY_QUERY, END_SYSEX,
                    START_SYSEX, ANALOG_MAPPING_QUERY, END_SYSEX }))
    {
        return false;
    }
    
    // Wait for receipt of capability and analog mapping info
    if (!condition.wait_for(lock, std::chrono::seconds(2), [this]() {
        return (capabilityInfoReceived && analogMappingInfoReceived);
    }))
    {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Firmata device \"%s\" did not respond to capability requests",
               getTag().c_str());
        return false;
    }
    
    return true;
}


bool FirmataDevice::processChannelRequests() {
    for (auto &channel : requestedChannels) {
        std::vector<int> pinNumbers;
        if (!(channel->resolvePinNumbers(pinForAnalogChannel, pinNumbers))) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot resolve pin numbers on Firmata device \"%s\"", getTag().c_str());
            return false;
        }
        
        for (const auto pinNumber : pinNumbers) {
            if (pinNumber < minPinNumber || pinNumber > maxPinNumber) {
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "Pin numbers must be between %d and %d; %d is not valid",
                       minPinNumber,
                       maxPinNumber,
                       pinNumber);
                return false;
            }
            
            const auto &pinModes = modesForPin[pinNumber];
            if (pinModes.find(channel->getPinMode()) == pinModes.end()) {
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "Pin %d does not support the requested mode on Firmata device \"%s\"",
                       pinNumber,
                       getTag().c_str());
                return false;
            }
            
            if (channel->isAnalog() && channel->isInput()) {
                const auto analogChannelNumber = getAnalogChannelNumber(pinNumber);
                if (analogChannelNumber < minAnalogChannelNumber || analogChannelNumber > maxAnalogChannelNumber) {
                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                           "Pin %d is not an analog input on Firmata device \"%s\"",
                           pinNumber,
                           getTag().c_str());
                    return false;
                }
            }
            
            auto &slot = getChannelForPin(pinNumber);
            if (slot) {
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "Multiple channels map to pin %d on Firmata device \"%s\"",
                       pinNumber,
                       getTag().c_str());
                return false;
            }
            slot = channel;
        }
        
        if (channel->isOutput()) {
            boost::weak_ptr<FirmataDevice> weakThis(component_shared_from_this<FirmataDevice>());
            
            // It's OK to capture channel by reference, because it will remain alive (in
            // requestedChannels) for as long as the device is alive
            auto callback = [weakThis, &channel, pinNumbers](const Datum &data, MWTime time) {
                if (auto sharedThis = weakThis.lock()) {
                    unique_lock lock(sharedThis->mutex);
                    if (sharedThis->running) {
                        for (auto pinNumber : pinNumbers) {
                            switch (channel->getType()) {
                                case FirmataChannel::Type::Analog:
                                    sharedThis->setAnalogOutput(pinNumber,
                                                                channel->getValueForPin(pinNumber).getFloat());
                                    break;
                                    
                                case FirmataChannel::Type::Digital:
                                    sharedThis->setDigitalOutput(pinNumber,
                                                                 channel->getValueForPin(pinNumber).getBool());
                                    break;
                                    
                                case FirmataChannel::Type::Servo:
                                    sharedThis->setServo(pinNumber,
                                                         channel->getValueForPin(pinNumber).getFloat());
                                    break;
                            }
                        }
                    }
                }
            };
            
            channel->addNewValueNotification(boost::make_shared<VariableCallbackNotification>(callback));
        }
    }
    
    return true;
}


bool FirmataDevice::configurePins() {
    if (!sendData({ SYSTEM_RESET })) {
        return false;
    }
    
    if (samplingIntervalUS) {
        const auto samplingIntervalMS = samplingIntervalUS / 1000;
        if (!sendData({ START_SYSEX,
                        SAMPLING_INTERVAL,
                        std::uint8_t((samplingIntervalMS >> 0) & 0x7F),  // LSB
                        std::uint8_t((samplingIntervalMS >> 7) & 0x7F),  // MSB
                        END_SYSEX }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot set sampling interval on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
    }
    
    for (std::size_t portNum = 0; portNum < ports.size(); portNum++) {
        const auto &port = ports.at(portNum);
        for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
            const auto &channel = port.at(bitNum);
            if (channel) {
                const auto pinNumber = getPinNumber(portNum, bitNum);
                
                if (auto servoChannel = boost::dynamic_pointer_cast<FirmataServoChannel>(channel)) {
                    const auto minPulseWidth = servoChannel->getMinPulseWidth();
                    const auto maxPulseWidth = servoChannel->getMaxPulseWidth();
                    if (!sendData({ START_SYSEX,
                                    SERVO_CONFIG,
                                    std::uint8_t(pinNumber),
                                    std::uint8_t((minPulseWidth >> 0) & 0x7F),  // minPulseWidth LSB
                                    std::uint8_t((minPulseWidth >> 7) & 0x7F),  // minPulseWidth MSB
                                    std::uint8_t((maxPulseWidth >> 0) & 0x7F),  // maxPulseWidth LSB
                                    std::uint8_t((maxPulseWidth >> 7) & 0x7F),  // maxPulseWidth MSB
                                    END_SYSEX }))
                    {
                        merror(M_IODEVICE_MESSAGE_DOMAIN,
                               "Cannot set servo configuration on pin %d of Firmata device \"%s\"",
                               pinNumber,
                               getTag().c_str());
                        return false;
                    }
                }
                
                if (!sendData({ SET_PIN_MODE, std::uint8_t(pinNumber), std::uint8_t(channel->getPinMode()) })) {
                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                           "Cannot set mode of pin %d on Firmata device \"%s\"",
                           pinNumber,
                           getTag().c_str());
                    return false;
                }
                
                if (channel->isAnalog() && channel->isInput()) {
                    // StandardFirmata automatically enables reporting of pins in mode PIN_MODE_ANALOG.
                    // Since we don't want reporting to begin until startIO() is invoked, we must disable
                    // reporting of these pins.
                    const auto channelNumber = getAnalogChannelNumber(pinNumber);
                    if (!sendData({ std::uint8_t(REPORT_ANALOG | channelNumber), 0 })) {
                        merror(M_IODEVICE_MESSAGE_DOMAIN,
                               "Cannot disable reporting of analog input channel %d on Firmata device \"%s\"",
                               channelNumber,
                               getTag().c_str());
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}


bool FirmataDevice::startIO() {
    for (std::size_t portNum = 0; portNum < ports.size(); portNum++) {
        const auto &port = ports.at(portNum);
        bool portHasDigitalInputs = false;
        bool portHasDigitalOutputs = false;
        std::array<std::uint8_t, 2> portState = { 0, 0 };
        
        for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
            const auto &channel = port.at(bitNum);
            if (channel) {
                const auto pinNumber = getPinNumber(portNum, bitNum);
                switch (channel->getType()) {
                    case FirmataChannel::Type::Analog: {
                        switch (channel->getDirection()) {
                            case FirmataChannel::Direction::Input: {
                                const auto channelNumber = getAnalogChannelNumber(pinNumber);
                                if (!sendData({ std::uint8_t(REPORT_ANALOG | channelNumber), 1 })) {
                                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                                           "Cannot enable reporting of analog input channel %d on Firmata device \"%s\"",
                                           channelNumber,
                                           getTag().c_str());
                                    return false;
                                }
                                break;
                            }
                                
                            case FirmataChannel::Direction::Output:
                                if (!setAnalogOutput(pinNumber,
                                                     channel->getValueForPin(pinNumber).getFloat()))
                                {
                                    return false;
                                }
                                break;
                        }
                        break;
                    }
                        
                    case FirmataChannel::Type::Digital: {
                        switch (channel->getDirection()) {
                            case FirmataChannel::Direction::Input:
                                portHasDigitalInputs = true;
                                break;
                                
                            case FirmataChannel::Direction::Output:
                                portHasDigitalOutputs = true;
                                portState.at(bitNum / 7) |= (channel->getValueForPin(pinNumber).getBool() << (bitNum % 7));
                                break;
                        }
                        break;
                    }
                        
                    case FirmataChannel::Type::Servo: {
                        if (channel->isOutput()) {
                            if (!setServo(pinNumber,
                                          channel->getValueForPin(pinNumber).getFloat()))
                            {
                                return false;
                            }
                        }
                        break;
                    }
                }
            }
        }
        
        if (portHasDigitalInputs &&
            !sendData({ std::uint8_t(REPORT_DIGITAL | portNum), 1 }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot enable reporting of digital input pins on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
        
        if (portHasDigitalOutputs &&
            !sendData({ std::uint8_t(DIGITAL_MESSAGE | portNum), portState.at(0), portState.at(1) }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot set initial values of digital output pins on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
    }
    
    return true;
}


bool FirmataDevice::stopIO() {
    for (std::size_t portNum = 0; portNum < ports.size(); portNum++) {
        const auto &port = ports.at(portNum);
        bool portHasDigitalInputs = false;
        bool portHasDigitalOutputs = false;
        
        for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
            const auto &channel = port.at(bitNum);
            if (channel) {
                const auto pinNumber = getPinNumber(portNum, bitNum);
                switch (channel->getType()) {
                    case FirmataChannel::Type::Analog: {
                        switch (channel->getDirection()) {
                            case FirmataChannel::Direction::Input: {
                                const auto channelNumber = getAnalogChannelNumber(pinNumber);
                                if (!sendData({ std::uint8_t(REPORT_ANALOG | channelNumber), 0 })) {
                                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                                           "Cannot disable reporting of analog input channel %d on Firmata device \"%s\"",
                                           channelNumber,
                                           getTag().c_str());
                                    return false;
                                }
                                break;
                            }
                                
                            case FirmataChannel::Direction::Output:
                                if (!setAnalogOutput(pinNumber, 0.0)) {
                                    return false;
                                }
                                break;
                        }
                        break;
                    }
                        
                    case FirmataChannel::Type::Digital: {
                        switch (channel->getDirection()) {
                            case FirmataChannel::Direction::Input:
                                portHasDigitalInputs = true;
                                break;
                                
                            case FirmataChannel::Direction::Output:
                                portHasDigitalOutputs = true;
                                break;
                        }
                        break;
                    }
                        
                    case FirmataChannel::Type::Servo: {
                        // We don't really know what the "ground" state is for a servo, so just
                        // leave it where it is
                        break;
                    }
                }
            }
        }
        
        if (portHasDigitalInputs &&
            !sendData({ std::uint8_t(REPORT_DIGITAL | portNum), 0 }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot disable reporting of digital input pins on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
        
        if (portHasDigitalOutputs &&
            !sendData({ std::uint8_t(DIGITAL_MESSAGE | portNum), 0, 0 }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot reset values of digital output pins on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
    }
    
    return true;
}


inline int FirmataDevice::getResolutionForPinMode(int pinNumber, int pinMode) const {
    return modesForPin.at(pinNumber).at(pinMode);
}


inline double FirmataDevice::getMaximumValueForPinMode(int pinNumber, int pinMode) const {
    return double((1 << getResolutionForPinMode(pinNumber, pinMode)) - 1);
}


bool FirmataDevice::setAnalogOutput(int pinNumber, double value) {
    if (value < 0.0 || value > 1.0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Firmata analog output value must be between 0 and 1; %g is not valid",
               value);
        return false;
    }
    
    int intValue = std::round(value * getMaximumValueForPinMode(pinNumber, PIN_MODE_PWM));
    
    if (!sendExtendedAnalogMessage(pinNumber, PIN_MODE_PWM, intValue)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot set analog output value on pin %d of Firmata device \"%s\"",
               pinNumber,
               getTag().c_str());
        return false;
    }
    
    return true;
}


bool FirmataDevice::setDigitalOutput(int pinNumber, bool value) {
    if (!sendData({ SET_DIGITAL_PIN_VALUE, std::uint8_t(pinNumber), value })) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot set digital output value on pin %d of Firmata device \"%s\"",
               pinNumber,
               getTag().c_str());
        return false;
    }
    
    return true;
}


bool FirmataDevice::setServo(int pinNumber, double value) {
    if (value < 0.0 || value > 180.0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Firmata servo value must be between 0 and 180; %g is not valid",
               value);
        return false;
    }
    
    if (!sendExtendedAnalogMessage(pinNumber, PIN_MODE_SERVO, std::round(value))) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot set servo value on pin %d of Firmata device \"%s\"",
               pinNumber,
               getTag().c_str());
        return false;
    }
    
    return true;
}


bool FirmataDevice::sendExtendedAnalogMessage(int pinNumber, int pinMode, int value) {
    const int numBits = getResolutionForPinMode(pinNumber, pinMode);
    std::vector<std::uint8_t> data = { START_SYSEX, EXTENDED_ANALOG, std::uint8_t(pinNumber) };
    
    for (std::size_t bitNum = 0; bitNum < numBits; bitNum++) {
        if (bitNum % 7 == 0) {
            data.push_back(0);
        }
        data.back() |= (value & 1) << (bitNum % 7);
        value >>= 1;
    }
    
    data.push_back(END_SYSEX);
    
    if (!sendData(data)) {
        return false;
    }
    
    return true;
}


bool FirmataDevice::sendData(const std::vector<std::uint8_t> &data) {
    if (connected) {
        return connection->sendData(data);
    }
    merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot send data to Firmata device \"%s\": Not connected", getTag().c_str());
    return false;
}


void FirmataDevice::receivedProtocolVersion(std::uint8_t protocolVersionMajor, std::uint8_t protocolVersionMinor) {
    {
        unique_lock lock(mutex);
        deviceProtocolVersionMajor = protocolVersionMajor;
        deviceProtocolVersionMinor = protocolVersionMinor;
        deviceProtocolVersionReceived = true;
    }
    condition.notify_all();
}


void FirmataDevice::receivedCapabilityInfo(const PinModesMap &modesForPin) {
    {
        unique_lock lock(mutex);
        this->modesForPin = modesForPin;
        capabilityInfoReceived = true;
    }
    condition.notify_all();
}


void FirmataDevice::receivedAnalogMappingInfo(const AnalogChannelPinMap &pinForAnalogChannel) {
    {
        unique_lock lock(mutex);
        this->pinForAnalogChannel = pinForAnalogChannel;
        analogMappingInfoReceived = true;
    }
    condition.notify_all();
}


void FirmataDevice::receivedDigitalMessage(std::uint8_t portNum, const PortStateArray &portState, MWTime time) {
    unique_lock lock(mutex);
    if (running) {
        const auto &port = ports.at(portNum);
        for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
            const auto &channel = port.at(bitNum);
            if (channel && channel->isDigital() && channel->isInput()) {
                const auto pinNumber = getPinNumber(portNum, bitNum);
                const bool value = portState.at(bitNum / 7) & (1 << (bitNum % 7));
                if (channel->getValueForPin(pinNumber).getBool() != value) {
                    channel->setValueForPin(pinNumber, value, time);
                }
            }
        }
    }
}


void FirmataDevice::receivedAnalogMessage(std::uint8_t channelNumber, int value, MWTime time) {
    unique_lock lock(mutex);
    if (running) {
        const auto iter = pinForAnalogChannel.find(channelNumber);
        if (iter != pinForAnalogChannel.end()) {
            const auto pinNumber = iter->second;
            const auto &channel = getChannelForPin(pinNumber);
            if (channel && channel->isAnalog() && channel->isInput()) {
                auto floatValue = double(value) / getMaximumValueForPinMode(pinNumber, PIN_MODE_ANALOG);
                // The user expects analog samples at a steady rate, so set the value unconditionally
                channel->setValueForPin(pinNumber, floatValue, time);
            }
        }
    }
}


void FirmataDevice::disconnected() {
    unique_lock lock(mutex);
    setConnected(false);
}


void FirmataDevice::reconnected() {
    unique_lock lock(mutex);
    
    if (!deviceProtocolVersionReceived ||
        !capabilityInfoReceived ||
        !analogMappingInfoReceived)
    {
        // We're still in initial configuration.  Don't do anything here.
        return;
    }
    
    setConnected(true);
    
    // We're going to assume that we've reconnected to the same device and not request the
    // protocol version and device info again.  However, as the connection may have failed
    // because the device lost power, we do need to reset, reconfigure, and (if running)
    // restart I/O.
    if (!configurePins() ||
        (running && !startIO()))
    {
        running = false;
    }
}


END_NAMESPACE_MW
