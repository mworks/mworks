//
//  FirmataDevice.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/10/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDevice.hpp"


BEGIN_NAMESPACE_MW


enum {
    DIGITAL_MESSAGE       = 0x90, // send data for a digital port (collection of 8 pins)
    ANALOG_MESSAGE        = 0xE0, // send data for an analog pin (or PWM)
    REPORT_ANALOG         = 0xC0, // enable analog input by pin #
    REPORT_DIGITAL        = 0xD0, // enable digital input by port pair
    
    SET_PIN_MODE          = 0xF4, // set a pin to INPUT/OUTPUT/PWM/etc
    SET_DIGITAL_PIN_VALUE = 0xF5, // set value of an individual digital pin
    
    REPORT_VERSION        = 0xF9, // report protocol version
    SYSTEM_RESET          = 0xFF, // reset from MIDI
    
    START_SYSEX           = 0xF0, // start a MIDI Sysex message
    END_SYSEX             = 0xF7, // end a MIDI Sysex message
    
    REPORT_FIRMWARE       = 0x79  // report name and version of the firmware
};


const std::string FirmataDevice::SERIAL_PORT("serial_port");


void FirmataDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/firmata");
    
    info.addParameter(SERIAL_PORT, false);
}


FirmataDevice::FirmataDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    path(parameters[SERIAL_PORT].str()),
    deviceProtocolVersionReceived(false),
    deviceProtocolVersionMajor(0),
    deviceProtocolVersionMinor(0),
    running(false)
{
}


FirmataDevice::~FirmataDevice() {
    if (receiveDataThread.joinable()) {
        continueReceivingData.clear();
        receiveDataThread.join();
    }
}


void FirmataDevice::addChild(std::map<std::string, std::string> parameters,
                             ComponentRegistryPtr reg,
                             boost::shared_ptr<Component> child)
{
    if (auto digitalChannel = boost::dynamic_pointer_cast<FirmataDigitalChannel>(child)) {
        auto &channel = getChannelForPin(digitalChannel->getPinNumber());
        if (channel) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Digital pin %d is already in use", digitalChannel->getPinNumber());
        } else {
            channel = digitalChannel;
        }
        return;
    }
    
    throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for Firmata device");
}


bool FirmataDevice::initialize() {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Configuring Firmata device \"%s\"...", getTag().c_str());
    
    if (!serialPort.connect(path, B57600)) {
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
            !configureDigitalPorts())
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
        if (!startDigitalIO()) {
            return false;
        }
        running = true;
    }
    
    return true;
}


bool FirmataDevice::stopDeviceIO() {
    unique_lock lock(mutex);
    
    if (running) {
        if (!stopDigitalIO()) {
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


bool FirmataDevice::configureDigitalPorts() {
    boost::weak_ptr<FirmataDevice> weakThis(component_shared_from_this<FirmataDevice>());
    
    for (const auto &port : ports) {
        for (const auto &channel : port) {
            if (channel) {
                const auto pinNumber = channel->getPinNumber();
                if (!sendData({ SET_PIN_MODE, std::uint8_t(pinNumber), std::uint8_t(channel->getDirection()) })) {
                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                           "Cannot set mode of digital pin %d on Firmata device \"%s\"",
                           pinNumber,
                           getTag().c_str());
                    return false;
                }
                if (channel->isOutput()) {
                    auto callback = [weakThis, pinNumber](const Datum &data, MWTime time) {
                        if (auto sharedThis = weakThis.lock()) {
                            sharedThis->setDigitalOutput(pinNumber, data.getBool());
                        }
                    };
                    channel->getValueVariable()->addNotification(boost::make_shared<VariableCallbackNotification>(callback));
                }
            }
        }
    }
    
    return true;
}


bool FirmataDevice::startDigitalIO() {
    for (std::size_t portNum = 0; portNum < ports.size(); portNum++) {
        const auto &port = ports.at(portNum);
        bool portHasInputs = false;
        bool portHasOutputs = false;
        std::array<std::uint8_t, 2> portState = { 0, 0 };
        
        for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
            const auto &channel = port.at(bitNum);
            if (channel) {
                switch (channel->getDirection()) {
                    case FirmataDigitalChannel::Direction::Input:
                        portHasInputs = true;
                        break;
                        
                    case FirmataDigitalChannel::Direction::Output:
                        portHasOutputs = true;
                        portState.at(bitNum / 7) |= (channel->getValueVariable()->getValue().getBool() << (bitNum % 7));
                        break;
                }
            }
        }
        
        if (portHasInputs &&
            !sendData({ std::uint8_t(REPORT_DIGITAL | portNum), 1 }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot enable reporting of digital input pins on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
        
        if (portHasOutputs &&
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


bool FirmataDevice::stopDigitalIO() {
    for (std::size_t portNum = 0; portNum < ports.size(); portNum++) {
        const auto &port = ports.at(portNum);
        bool portHasInputs = false;
        bool portHasOutputs = false;
        
        for (const auto &channel : port) {
            if (channel) {
                switch (channel->getDirection()) {
                    case FirmataDigitalChannel::Direction::Input:
                        portHasInputs = true;
                        break;
                        
                    case FirmataDigitalChannel::Direction::Output:
                        portHasOutputs = true;
                        break;
                }
            }
        }
        
        if (portHasInputs &&
            !sendData({ std::uint8_t(REPORT_DIGITAL | portNum), 0 }))
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot disable reporting of digital input pins on Firmata device \"%s\"",
                   getTag().c_str());
            return false;
        }
        
        if (portHasOutputs &&
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


void FirmataDevice::setDigitalOutput(int pinNumber, bool value) {
    unique_lock lock(mutex);
    
    if (running &&
        !sendData({ SET_DIGITAL_PIN_VALUE, std::uint8_t(pinNumber), value }))
    {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot set value of digital output pin %d on Firmata device \"%s\"",
               pinNumber,
               getTag().c_str());
    }
}


inline bool FirmataDevice::sendData(const std::vector<std::uint8_t> &data) {
    return (-1 != serialPort.write(data));
}


void FirmataDevice::receiveData() {
    std::array<std::uint8_t, 3> message;
    std::size_t bytesReceived = 0;
    std::size_t bytesExpected = 1;
    MWTime currentCommandTime = 0;
    std::uint8_t currentCommand = 0;
    
    while (continueReceivingData.test_and_set()) {
        // Since this is the only thread that reads from the serial port, we don't need a lock here
        auto result = serialPort.read(message.data() + bytesReceived, bytesExpected - bytesReceived);
        
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
                    
                    if (message.at(1) != REPORT_FIRMWARE) {
                        mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                                 "Received unexpected SysEx command (0x%02hhX) from Firmata device \"%s\"",
                                 message.at(1),
                                 getTag().c_str());
                    }
                    
                    bytesExpected = 3;
                    
                } else if (message.at(2) == END_SYSEX) {
                    
                    bytesReceived = 0;
                    bytesExpected = 1;
                    
                } else {
                    
                    // Still waiting for END_SYSEX
                    bytesReceived--;
                    
                }
            
            } else if (bytesReceived == bytesExpected) {
                
                //
                // Finish processing current message
                //
                
                switch (currentCommand) {
                        
                    case DIGITAL_MESSAGE: {
                        const std::size_t portNum = (message.at(0) & 0x0F);
                        const std::array<std::uint8_t, 2> portState = { message.at(1), message.at(2) };
                        const auto &port = ports.at(portNum);
                        
                        for (std::size_t bitNum = 0; bitNum < port.size(); bitNum++) {
                            const auto &channel = port.at(bitNum);
                            if (channel && channel->isInput()) {
                                const bool value = portState.at(bitNum / 7) & (1 << (bitNum % 7));
                                if (channel->getValueVariable()->getValue().getBool() != value) {
                                    channel->getValueVariable()->setValue(value, currentCommandTime);
                                }
                            }
                        }
                        
                        break;
                    }
                        
                    case REPORT_VERSION:
                        if (!deviceProtocolVersionReceived) {
                            {
                                unique_lock lock(mutex);
                                deviceProtocolVersionMajor = message.at(1);
                                deviceProtocolVersionMinor = message.at(2);
                                deviceProtocolVersionReceived = true;
                            }
                            condition.notify_all();
                        }
                        break;
                        
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




























