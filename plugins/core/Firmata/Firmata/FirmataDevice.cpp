//
//  FirmataDevice.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/10/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDevice.hpp"

#include "FirmataServoChannel.hpp"


BEGIN_NAMESPACE_MW


//
// Message types
//

enum {
    DIGITAL_MESSAGE         = 0x90, // send data for a digital port (collection of 8 pins)
    ANALOG_MESSAGE          = 0xE0, // send data for an analog pin (or PWM)
    REPORT_ANALOG           = 0xC0, // enable analog input by pin #
    REPORT_DIGITAL          = 0xD0, // enable digital input by port pair
    
    SET_PIN_MODE            = 0xF4, // set a pin to INPUT/OUTPUT/PWM/etc
    SET_DIGITAL_PIN_VALUE   = 0xF5, // set value of an individual digital pin
    
    REPORT_VERSION          = 0xF9, // report protocol version
    SYSTEM_RESET            = 0xFF, // reset from MIDI
    
    START_SYSEX             = 0xF0, // start a MIDI Sysex message
    END_SYSEX               = 0xF7, // end a MIDI Sysex message
    
    SERVO_CONFIG            = 0x70, // set minPulse, maxPulse
    REPORT_FIRMWARE         = 0x79, // report name and version of the firmware
    EXTENDED_ANALOG         = 0x6F, // analog write (PWM, Servo, etc) to any pin
    CAPABILITY_QUERY        = 0x6B, // ask for supported modes and resolution of all pins
    CAPABILITY_RESPONSE     = 0x6C, // reply with supported modes and resolution
    ANALOG_MAPPING_QUERY    = 0x69, // ask for mapping of analog to pin numbers
    ANALOG_MAPPING_RESPONSE = 0x6A, // reply with mapping info
    SAMPLING_INTERVAL       = 0x7A  // set the poll rate of the main loop
};


//
// Pin modes
//

enum {
    PIN_MODE_INPUT  = 0x00, // same as INPUT defined in Arduino.h
    PIN_MODE_OUTPUT = 0x01, // same as OUTPUT defined in Arduino.h
    PIN_MODE_ANALOG = 0x02, // analog pin in analogInput mode
    PIN_MODE_PWM    = 0x03, // digital pin in PWM output mode
    PIN_MODE_SERVO  = 0x04  // digital pin in Servo output mode
};


const std::string FirmataDevice::SERIAL_PORT("serial_port");
const std::string FirmataDevice::BLUETOOTH_LOCAL_NAME("bluetooth_local_name");
const std::string FirmataDevice::DATA_INTERVAL("data_interval");


void FirmataDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/firmata");
    
    info.addParameter(SERIAL_PORT, false);
    info.addParameter(BLUETOOTH_LOCAL_NAME, false);
    info.addParameter(DATA_INTERVAL, false);
}


FirmataDevice::FirmataDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    connection(FirmataConnection::create(parameters[SERIAL_PORT], parameters[BLUETOOTH_LOCAL_NAME])),
    samplingIntervalUS(0),
    deviceProtocolVersionReceived(false),
    deviceProtocolVersionMajor(0),
    deviceProtocolVersionMinor(0),
    capabilityInfoReceived(false),
    analogMappingInfoReceived(false),
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
}


FirmataDevice::~FirmataDevice() {
    if (receiveDataThread.joinable()) {
        continueReceivingData.clear();
        receiveDataThread.join();
    }
    connection->disconnect();
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
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Configuring Firmata device \"%s\"...", getTag().c_str());
    
    if (!connection->connect()) {
        return false;
    }
    
    continueReceivingData.test_and_set();
    receiveDataThread = std::thread([this]() {
        receiveData();
    });
    
    // Wait for connection to be established
    Clock::instance()->sleepMS(3000);
    
    {
        unique_lock lock(mutex);
        
        if (!sendData({ REPORT_VERSION, SYSTEM_RESET }) ||
            !checkProtocolVersion(lock) ||
            !getDeviceInfo(lock) ||
            !processChannelRequests() ||
            !configurePins())
        {
            return false;
        }
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
        if (!(channel->resolvePinNumber(pinForAnalogChannel))) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot resolve pin number on Firmata device \"%s\"", getTag().c_str());
            return false;
        }
        
        const int pinNumber = channel->getPinNumber();
        
        const auto &pinModes = modesForPin[pinNumber];
        if (pinModes.find(channel->getPinMode()) == pinModes.end()) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Pin %d does not support the requested mode on Firmata device \"%s\"",
                   pinNumber,
                   getTag().c_str());
            return false;
        }
        
        if (channel->isAnalog() && channel->isInput() && (channel->getAnalogChannelNumber() < 0)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Pin %d is not an analog input on Firmata device \"%s\"",
                   pinNumber,
                   getTag().c_str());
            return false;
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
    
    return true;
}


bool FirmataDevice::configurePins() {
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
    
    boost::weak_ptr<FirmataDevice> weakThis(component_shared_from_this<FirmataDevice>());
    
    for (const auto &port : ports) {
        for (const auto &channel : port) {
            if (channel) {
                const auto pinNumber = channel->getPinNumber();
                
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
                
                switch (channel->getDirection()) {
                    case FirmataChannel::Direction::Input: {
                        if (channel->isAnalog()) {
                            // StandardFirmata automatically enables reporting of pins in mode PIN_MODE_ANALOG.
                            // Since we don't want reporting to begin until startIO() is invoked, we must disable
                            // reporting of these pins.
                            const auto channelNumber = channel->getAnalogChannelNumber();
                            if (!sendData({ std::uint8_t(REPORT_ANALOG | channelNumber), 0 })) {
                                merror(M_IODEVICE_MESSAGE_DOMAIN,
                                       "Cannot disable reporting of analog input channel %d on Firmata device \"%s\"",
                                       channelNumber,
                                       getTag().c_str());
                                return false;
                            }
                        }
                        break;
                    }
                        
                    case FirmataChannel::Direction::Output: {
                        boost::function<void(const Datum &, MWTime)> callback;
                        
                        switch (channel->getType()) {
                            case FirmataChannel::Type::Analog:
                                callback = [weakThis, pinNumber](const Datum &data, MWTime time) {
                                    if (auto sharedThis = weakThis.lock()) {
                                        unique_lock lock(sharedThis->mutex);
                                        if (sharedThis->running) {
                                            sharedThis->setAnalogOutput(pinNumber, data.getFloat());
                                        }
                                    }
                                };
                                break;
                                
                            case FirmataChannel::Type::Digital:
                                callback = [weakThis, pinNumber](const Datum &data, MWTime time) {
                                    if (auto sharedThis = weakThis.lock()) {
                                        unique_lock lock(sharedThis->mutex);
                                        if (sharedThis->running) {
                                            sharedThis->setDigitalOutput(pinNumber, data.getBool());
                                        }
                                    }
                                };
                                break;
                                
                            case FirmataChannel::Type::Servo:
                                callback = [weakThis, pinNumber](const Datum &data, MWTime time) {
                                    if (auto sharedThis = weakThis.lock()) {
                                        unique_lock lock(sharedThis->mutex);
                                        if (sharedThis->running) {
                                            sharedThis->setServo(pinNumber, data.getFloat());
                                        }
                                    }
                                };
                                break;
                        }
                        
                        channel->getValueVariable()->addNotification(boost::make_shared<VariableCallbackNotification>(callback));
                        break;
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
                switch (channel->getType()) {
                    case FirmataChannel::Type::Analog: {
                        switch (channel->getDirection()) {
                            case FirmataChannel::Direction::Input: {
                                const auto channelNumber = channel->getAnalogChannelNumber();
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
                                if (!setAnalogOutput(channel->getPinNumber(),
                                                     channel->getValueVariable()->getValue().getFloat()))
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
                                portState.at(bitNum / 7) |= (channel->getValueVariable()->getValue().getBool() << (bitNum % 7));
                                break;
                        }
                        break;
                    }
                        
                    case FirmataChannel::Type::Servo: {
                        if (channel->isOutput()) {
                            if (!setServo(channel->getPinNumber(),
                                          channel->getValueVariable()->getValue().getFloat()))
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
        
        for (const auto &channel : port) {
            if (channel) {
                switch (channel->getType()) {
                    case FirmataChannel::Type::Analog: {
                        switch (channel->getDirection()) {
                            case FirmataChannel::Direction::Input: {
                                const auto channelNumber = channel->getAnalogChannelNumber();
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
                                if (!setAnalogOutput(channel->getPinNumber(), 0.0)) {
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


inline bool FirmataDevice::sendData(const std::vector<std::uint8_t> &data) {
    return connection->write(data);
}


void FirmataDevice::receiveData() {
    std::array<std::uint8_t, 3> message;
    std::size_t bytesReceived = 0;
    std::size_t bytesExpected = 1;
    MWTime currentCommandTime = 0;
    std::uint8_t currentCommand = 0;
    std::uint8_t currentSysexCommand = 0;
    std::uint8_t currentPinNumber = 0;
    std::vector<std::uint8_t> currentPinModeInfo;
    
    while (continueReceivingData.test_and_set()) {
        // Since this is the only thread that reads from the serial port, we don't need a lock here
        auto result = connection->read(message.at(bytesReceived), bytesExpected - bytesReceived);
        
        if (-1 == result) {
            
            //
            // It would be nice if we tried to re-connect.  However, most Arduinos will reset the
            // running sketch upon connection, meaning we'd have to re-initialize everything.
            // Just give up.
            //
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Aborting all attempts to receive data from Firmata device \"%s\"",
                   getTag().c_str());
            return;
            
        } else if (result > 0) {
            
            bytesReceived += result;
            
            if (bytesReceived == 1) {
                
                //
                // Start processing new message
                //
                
                currentCommandTime = Clock::instance()->getCurrentTimeUS();
                
                currentCommand = message.at(0);
                if (currentCommand < 0xF0) {
                    currentCommand &= 0xF0;
                }
                
                switch (currentCommand) {
                        
                    case DIGITAL_MESSAGE:
                        bytesExpected = 3;
                        break;
                        
                    case ANALOG_MESSAGE:
                        bytesExpected = 3;
                        break;
                        
                    case REPORT_VERSION:
                        bytesExpected = 3;
                        break;
                        
                    case START_SYSEX:
                        bytesExpected = 2;
                        break;
                        
                    default:
                        mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                                 "Received unexpected command (0x%02hhX) from Firmata device \"%s\"",
                                 currentCommand,
                                 getTag().c_str());
                        bytesReceived = 0;
                        break;
                        
                }
                
            } else if (currentCommand == START_SYSEX) {
                
                //
                // Handle SysEx message
                //
                
                if (bytesExpected == 2) {
                    
                    currentSysexCommand = message.at(1);
                    
                    switch (currentSysexCommand) {
                        case REPORT_FIRMWARE:
                            // Ignored
                            break;
                            
                        case CAPABILITY_RESPONSE: {
                            unique_lock lock(mutex);
                            modesForPin.clear();
                            currentPinNumber = 0;
                            break;
                        }
                            
                        case ANALOG_MAPPING_RESPONSE: {
                            unique_lock lock(mutex);
                            pinForAnalogChannel.clear();
                            currentPinNumber = 0;
                            break;
                        }
                            
                        default:
                            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                                     "Received unexpected SysEx command (0x%02hhX) from Firmata device \"%s\"",
                                     currentSysexCommand,
                                     getTag().c_str());
                            break;
                    }
                    
                    bytesExpected = 3;
                    
                } else if (message.at(2) == END_SYSEX) {
                    
                    switch (currentSysexCommand) {
                        case CAPABILITY_RESPONSE: {
                            {
                                unique_lock lock(mutex);
                                capabilityInfoReceived = true;
                            }
                            condition.notify_all();
                            break;
                        }
                            
                        case ANALOG_MAPPING_RESPONSE: {
                            {
                                unique_lock lock(mutex);
                                analogMappingInfoReceived = true;
                            }
                            condition.notify_all();
                            break;
                        }
                    }
                    
                    bytesReceived = 0;
                    bytesExpected = 1;
                    
                } else {
                    
                    switch (currentSysexCommand) {
                        case CAPABILITY_RESPONSE: {
                            const auto modeInfo = message.at(2);
                            if (modeInfo != 0x7F) {
                                currentPinModeInfo.push_back(modeInfo);
                            } else {
                                if (!(currentPinModeInfo.empty())) {
                                    unique_lock lock(mutex);
                                    auto &currentPinModes = modesForPin[currentPinNumber];
                                    for (std::size_t i = 0; i < currentPinModeInfo.size() - 1; i += 2) {
                                        currentPinModes[currentPinModeInfo.at(i)] = currentPinModeInfo.at(i + 1);
                                    }
                                    currentPinModeInfo.clear();
                                }
                                currentPinNumber++;
                            }
                            break;
                        }
                            
                        case ANALOG_MAPPING_RESPONSE: {
                            const auto channelNumber = message.at(2);
                            if (channelNumber != 127) {
                                unique_lock lock(mutex);
                                pinForAnalogChannel[channelNumber] = currentPinNumber;
                            }
                            currentPinNumber++;
                            break;
                        }
                    }
                    
                    // Still waiting for END_SYSEX
                    bytesReceived--;
                    
                }
            
            } else if (bytesReceived == bytesExpected) {
                
                //
                // Finish processing current message
                //
                
                switch (currentCommand) {
                        
                    case DIGITAL_MESSAGE:
                        if (running) {
                            const std::size_t portNum = (message.at(0) & 0x0F);
                            const std::array<std::uint8_t, 2> portState = { message.at(1), message.at(2) };
                            const auto &port = ports.at(portNum);
                            
                            for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
                                const auto &channel = port.at(bitNum);
                                if (channel && channel->isDigital() && channel->isInput()) {
                                    const bool value = portState.at(bitNum / 7) & (1 << (bitNum % 7));
                                    if (channel->getValueVariable()->getValue().getBool() != value) {
                                        channel->getValueVariable()->setValue(value, currentCommandTime);
                                    }
                                }
                            }
                        }
                        break;
                        
                    case ANALOG_MESSAGE:
                        if (running) {
                            const auto channelNumber = (message.at(0) & 0x0F);
                            const auto iter = pinForAnalogChannel.find(channelNumber);
                            if (iter != pinForAnalogChannel.end()) {
                                const auto pinNumber = iter->second;
                                const auto &channel = getChannelForPin(pinNumber);
                                if (channel && channel->isAnalog() && channel->isInput()) {
                                    int value = 0;
                                    value |= (message.at(1) & 0x7F) << 0;  // LSB
                                    value |= (message.at(2) & 0x7F) << 7;  // MSB
                                    auto floatValue = double(value) / getMaximumValueForPinMode(pinNumber, PIN_MODE_ANALOG);
                                    // The user expects analog samples at a steady rate, so set the value unconditionally
                                    channel->getValueVariable()->setValue(floatValue, currentCommandTime);
                                }
                            }
                        }
                        break;
                        
                    case REPORT_VERSION: {
                        {
                            unique_lock lock(mutex);
                            deviceProtocolVersionMajor = message.at(1);
                            deviceProtocolVersionMinor = message.at(2);
                            deviceProtocolVersionReceived = true;
                        }
                        condition.notify_all();
                        break;
                    }
                        
                    default:
                        break;
                        
                }
                
                bytesReceived = 0;
                bytesExpected = 1;
                
            }
            
        }
    }
}


END_NAMESPACE_MW




























