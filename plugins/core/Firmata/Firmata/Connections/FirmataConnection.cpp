//
//  FirmataConnection.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataConnection.hpp"

#if TARGET_OS_OSX
#  include "FirmataSerialConnection.hpp"
#endif
#include "FirmataBluetoothLEConnection.hpp"

#include "FirmataMessageTypes.h"


BEGIN_NAMESPACE_MW


std::unique_ptr<FirmataConnection> FirmataConnection::create(FirmataConnectionClient &client,
                                                             const ParameterValue &serialPortPath,
                                                             const ParameterValue &bluetoothLocalName)
{
    if (bluetoothLocalName.empty()) {
#if TARGET_OS_OSX
        std::string path;
        if (!(serialPortPath.empty())) {
            path = variableOrText(serialPortPath)->getValue().getString();
        }
        return std::unique_ptr<FirmataConnection>(new FirmataSerialConnection(client, path));
#else
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Connection via serial port is not supported on this OS");
#endif
    }
    const auto localName = variableOrText(bluetoothLocalName)->getValue().getString();
    return std::unique_ptr<FirmataConnection>(new FirmataBluetoothLEConnection(client, localName));
}


FirmataConnection::FirmataConnection(FirmataConnectionClient &client) :
    client(client),
    clock(Clock::instance()),
    running(false)
{ }


bool FirmataConnection::initialize() {
    if (!connect()) {
        return false;
    }
    
    running = true;
    runThread = std::thread([this]() {
        run();
    });
    
    return true;
}


void FirmataConnection::finalize() {
    if (runThread.joinable()) {
        running = false;
        runThread.join();
    }
    disconnect();
}


void FirmataConnection::run() {
    while (running) {
        if (!receiveData()) {
            reconnect();
        }
    }
}


bool FirmataConnection::receiveData() {
    std::array<std::uint8_t, 3> message;
    std::size_t bytesReceived = 0;
    std::size_t bytesExpected = 1;
    MWTime currentCommandTime = 0;
    std::uint8_t currentCommand = 0;
    std::uint8_t currentSysexCommand = 0;
    FirmataConnectionClient::PinModesMap modesForPin;
    FirmataConnectionClient::AnalogChannelPinMap pinForAnalogChannel;
    std::uint8_t currentPinNumber = 0;
    std::vector<std::uint8_t> currentPinModeInfo;
    
    while (running) {
        auto result = read(message.at(bytesReceived), bytesExpected - bytesReceived);
        
        if (-1 == result) {
            
            return false;
            
        } else if (result > 0) {
            
            bytesReceived += result;
            
            if (bytesReceived == 1) {
                
                //
                // Start processing new message
                //
                
                currentCommandTime = clock->getCurrentTimeUS();
                
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
                                 client.getDeviceName().c_str());
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
                            
                        case CAPABILITY_RESPONSE:
                            modesForPin.clear();
                            currentPinNumber = 0;
                            break;
                            
                        case ANALOG_MAPPING_RESPONSE:
                            pinForAnalogChannel.clear();
                            currentPinNumber = 0;
                            break;
                            
                        default:
                            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                                     "Received unexpected SysEx command (0x%02hhX) from Firmata device \"%s\"",
                                     currentSysexCommand,
                                     client.getDeviceName().c_str());
                            break;
                    }
                    
                    bytesExpected = 3;
                    
                } else if (message.at(2) == END_SYSEX) {
                    
                    switch (currentSysexCommand) {
                        case CAPABILITY_RESPONSE:
                            client.receivedCapabilityInfo(modesForPin);
                            break;
                            
                        case ANALOG_MAPPING_RESPONSE:
                            client.receivedAnalogMappingInfo(pinForAnalogChannel);
                            break;
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
                        
                    case DIGITAL_MESSAGE: {
                        const auto portNum = (message.at(0) & 0x0F);
                        const FirmataConnectionClient::PortStateArray portState = { message.at(1), message.at(2) };
                        client.receivedDigitalMessage(portNum, portState, currentCommandTime);
                        break;
                    }
                        
                    case ANALOG_MESSAGE: {
                        const auto channelNumber = (message.at(0) & 0x0F);
                        int value = 0;
                        value |= (message.at(1) & 0x7F) << 0;  // LSB
                        value |= (message.at(2) & 0x7F) << 7;  // MSB
                        client.receivedAnalogMessage(channelNumber, value, currentCommandTime);
                        break;
                    }
                        
                    case REPORT_VERSION: {
                        const auto protocolVersionMajor = message.at(1);
                        const auto protocolVersionMinor = message.at(2);
                        client.receivedProtocolVersion(protocolVersionMajor, protocolVersionMinor);
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
    
    return true;
}


void FirmataConnection::reconnect() {
    disconnect();
    client.disconnected();
    
    const auto reconnectIntervalUS = client.getReconnectInterval();
    if (reconnectIntervalUS <= 0) {
        // Client doesn't want us to attempt reconnection.  Just abort.
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Disconnected from Firmata device \"%s\"", client.getDeviceName().c_str());
        running = false;
        return;
    }
    
    merror(M_IODEVICE_MESSAGE_DOMAIN,
           "Disconnected from Firmata device \"%s\"; will try to reconnect in %g seconds",
           client.getDeviceName().c_str(),
           double(reconnectIntervalUS) / 1e6);
    
    while (running) {
        
        // Perform an "active" wait, so that we can exit promptly if finalize() is called
        const auto waitStopTime = clock->getCurrentTimeUS() + reconnectIntervalUS;
        while (true) {
            if (!running) {
                return;
            }
            if (clock->getCurrentTimeUS() >= waitStopTime) {
                break;
            }
            clock->yield();
        }
        
        mprintf(M_IODEVICE_MESSAGE_DOMAIN,
                "Attempting to reconnect to Firmata device \"%s\"...",
                client.getDeviceName().c_str());
        
        if (connect()) {
            mprintf(M_IODEVICE_MESSAGE_DOMAIN,
                    "Successfully reconnected to Firmata device \"%s\"",
                    client.getDeviceName().c_str());
            client.reconnected();
            return;
        }
        
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Failed to reconnect to Firmata device \"%s\"; will try again in %g seconds",
               client.getDeviceName().c_str(),
               double(reconnectIntervalUS) / 1e6);
        
        // Disconnect to ensure that we start in a clean state on the next
        // reconnection attempt
        disconnect();
        
    }
}


END_NAMESPACE_MW
