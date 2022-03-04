//
//  ISCANDevice.cpp
//  ISCAN
//
//  Created by Christopher Stawarz on 5/20/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "ISCANDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string ISCANDevice::SERIAL_PORT("serial_port");
const std::string ISCANDevice::START_COMMAND("start_command");
const std::string ISCANDevice::STOP_COMMAND("stop_command");


inline std::string ISCANDevice::getOutputParameterName(std::size_t outputNumber) {
    return (boost::format("output_0%d") % (outputNumber + 1)).str();
}


void ISCANDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/iscan");
    
    info.addParameter(SERIAL_PORT, false);
    for (std::size_t outputNumber = 0; outputNumber < maxNumOutputs; outputNumber++) {
        info.addParameter(getOutputParameterName(outputNumber), false);
    }
    info.addParameter(START_COMMAND, "128");
    info.addParameter(STOP_COMMAND, "129");
}


ISCANDevice::ISCANDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    path(variableOrText(parameters[SERIAL_PORT])->getValue().getString()),
    startCommand(parameters[START_COMMAND]),
    stopCommand(parameters[STOP_COMMAND]),
    running(false)
{
    for (std::size_t outputNumber = 0; outputNumber < maxNumOutputs; outputNumber++) {
        auto &outputParameter = parameters[getOutputParameterName(outputNumber)];
        if (!outputParameter.empty()) {
            outputs.at(outputNumber) = VariablePtr(outputParameter);
        }
    }
}


ISCANDevice::~ISCANDevice() {
    if (receiveDataThread.joinable()) {
        continueReceivingData.clear();
        receiveDataThread.join();
    }
}


bool ISCANDevice::initialize() {
    if (!serialPort.connect(path, baudRate)) {
        return false;
    }
    
    continueReceivingData.test_and_set();
    receiveDataThread = std::thread([this]() {
        receiveData();
    });
    
    return true;
}


bool ISCANDevice::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        if (!sendCommand(startCommand)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot start ISCAN device");
            return false;
        }
        running = true;
    }
    
    return true;
}


bool ISCANDevice::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (running) {
        if (!sendCommand(stopCommand)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot stop ISCAN device");
            return false;
        }
        running = false;
    }
    
    return true;
}


inline bool ISCANDevice::sendCommand(std::uint8_t cmd) {
    return (-1 != serialPort.write(&cmd, 1));
}


void ISCANDevice::receiveData() {
    boost::endian::little_uint16_t word = 0;
    std::size_t bytesRead = 0;
    int currentOutputNumber = -1;
    
    while (continueReceivingData.test_and_set()) {
        // Since this is the only thread that reads from the serial port, we don't need a lock here
        auto result = serialPort.read(reinterpret_cast<std::uint8_t *>(&word) + bytesRead, sizeof(word) - bytesRead);
        
        if (-1 == result) {
            
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Attempting to reconnect to ISCAN device...");
            serialPort.disconnect();
            if (!serialPort.connect(path, baudRate)) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot reconnect to ISCAN device");
                return;
            }
            
        } else if (result > 0) {
            
            bytesRead += result;
            
            if (bytesRead == sizeof(word)) {
                if (0x4444 == word) {
                    // Header received.  Restart output counter.
                    currentOutputNumber = 0;
                } else if (currentOutputNumber >= 0) {
                    if (currentOutputNumber < outputs.size()) {
                        if (auto &output = outputs.at(currentOutputNumber)) {
                            output->setValue(word / 10.0);
                        }
                    }
                    currentOutputNumber++;
                }
                
                word = 0;
                bytesRead = 0;
            }
            
        }
    }
}


END_NAMESPACE_MW




























