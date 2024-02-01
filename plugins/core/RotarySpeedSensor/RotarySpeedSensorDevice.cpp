//
//  RotarySpeedSensorDevice.cpp
//  RotarySpeedSensor
//
//  Created by Christopher Stawarz on 2/1/24.
//

#include "RotarySpeedSensorDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string RotarySpeedSensorDevice::SERIAL_PORT("serial_port");
const std::string RotarySpeedSensorDevice::DIRECTION("direction");
const std::string RotarySpeedSensorDevice::RPM("rpm");
const std::string RotarySpeedSensorDevice::RECONNECT_INTERVAL("reconnect_interval");


void RotarySpeedSensorDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/rotary_speed_sensor");
    
    info.addParameter(SERIAL_PORT, false);
    info.addParameter(DIRECTION);
    info.addParameter(RPM);
    info.addParameter(RECONNECT_INTERVAL, "0");
}


RotarySpeedSensorDevice::RotarySpeedSensorDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    serialPortPath(optionalVariableOrText(parameters[SERIAL_PORT])),
    direction(parameters[DIRECTION]),
    rpm(parameters[RPM]),
    reconnectIntervalUS(parameters[RECONNECT_INTERVAL]),
    clock(Clock::instance()),
    running(false)
{ }


RotarySpeedSensorDevice::~RotarySpeedSensorDevice() {
    if (receiveDataThread.joinable()) {
        continueReceivingData.clear();
        receiveDataThread.join();
    }
}


bool RotarySpeedSensorDevice::initialize() {
    if (serialPortPath) {
        path = serialPortPath->getValue().getString();
    }
    if (!serialPort.connect(path, baudRate)) {
        return false;
    }
    
    continueReceivingData.test_and_set();
    receiveDataThread = std::thread([this]() {
        receiveData();
    });
    
    return true;
}


bool RotarySpeedSensorDevice::startDeviceIO() {
    running = true;
    return true;
}


bool RotarySpeedSensorDevice::stopDeviceIO() {
    running = false;
    return true;
}


void RotarySpeedSensorDevice::receiveData() {
    std::uint8_t currentByte = 0;
    std::size_t currentByteIndex = START_BYTE_INDEX;
    
    MWTime currentTime = 0;
    long long currentDirection = 0;
    long long currentRPM = 0;
    
    while (continueReceivingData.test_and_set()) {
        auto result = serialPort.read(&currentByte, 1);
        
        if (-1 == result) {
            
            serialPort.disconnect();
            if (!reconnect()) {
                return;
            }
            
        } else if (result > 0) {
            
            switch (currentByteIndex) {
                case START_BYTE_INDEX:
                    if (currentByte != 255) {
                        // We expected the start byte but didn't receive it, so we must be out of sync
                        // with the device.  Discard the current byte and try again.
                        continue;
                    }
                    currentTime = clock->getCurrentTimeUS();
                    break;
                    
                case DIRECTON_BYTE_INDEX:
                    currentDirection = currentByte;
                    break;
                    
                case RPM_BYTE_INDEX:
                    currentRPM = currentByte;
                    if (running) {
                        direction->setValue(currentDirection, currentTime);
                        rpm->setValue(currentRPM, currentTime);
                    }
                    break;
            }
            
            currentByteIndex = (currentByteIndex + 1) % BYTE_INDEX_COUNT;
            
        }
    }
}


bool RotarySpeedSensorDevice::reconnect() {
    if (reconnectIntervalUS <= 0) {
        // User doesn't want us to attempt reconnection.  Just abort.
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Disconnected from rotary speed sensor");
        return false;
    }
    
    merror(M_IODEVICE_MESSAGE_DOMAIN,
           "Disconnected from rotary speed sensor; will try to reconnect in %g seconds",
           double(reconnectIntervalUS) / 1e6);
    
    while (true) {
        const auto waitStopTime = clock->getCurrentTimeUS() + reconnectIntervalUS;
        
        // Perform an "active" wait, so that we can exit promptly if the experiment is unloaded
        while (clock->getCurrentTimeUS() < waitStopTime) {
            if (!continueReceivingData.test_and_set()) {
                return false;
            }
            clock->yield();
        }
        
        mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Attempting to reconnect to rotary speed sensor");
        
        if (serialPort.connect(path, baudRate)) {
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Successfully reconnected to rotary speed sensor");
            break;
        }
        
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Failed to reconnect to rotary speed sensor; will try again in %g seconds",
               double(reconnectIntervalUS) / 1e6);
    }
    
    return true;
}


END_NAMESPACE_MW
