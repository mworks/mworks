//
//  EasyEyeDevice.cpp
//  EasyEye
//
//  Created by Christopher Stawarz on 7/25/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "EasyEyeDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string EasyEyeDevice::ADDRESS("address");
const std::string EasyEyeDevice::PORT("port");
const std::string EasyEyeDevice::EYE_X("eye_x");
const std::string EasyEyeDevice::EYE_Y("eye_y");


void EasyEyeDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/easy_eye");
    
    info.addParameter(ADDRESS);
    info.addParameter(PORT);
    info.addParameter(EYE_X);
    info.addParameter(EYE_Y);
}


EasyEyeDevice::EasyEyeDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    socketEndpoint(zeromq::formatTCPEndpoint(variableOrText(parameters[ADDRESS])->getValue().getString(),
                                             int(parameters[PORT]))),
    eyeX(parameters[EYE_X]),
    eyeY(parameters[EYE_Y]),
    socket(ZMQ_STREAM),
    running(false)
{
    if (!socket.setOption(ZMQ_RCVTIMEO, 500/*ms*/)) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Cannot configure ZeroMQ socket");
    }
}


EasyEyeDevice::~EasyEyeDevice() {
    if (receiveDataThread.joinable()) {
        continueReceivingData.clear();
        receiveDataThread.join();
    }
}


bool EasyEyeDevice::initialize() {
    if (!socket.connect(socketEndpoint)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot connect to EasyEye device");
        return false;
    }
    
    continueReceivingData.test_and_set();
    receiveDataThread = std::thread([this]() {
        receiveData();
    });
    
    return true;
}


bool EasyEyeDevice::startDeviceIO() {
    running = true;
    return true;
}


bool EasyEyeDevice::stopDeviceIO() {
    running = false;
    return true;
}


void EasyEyeDevice::receiveData() {
    ZeroMQSocket::Message msg;
    std::string receivedData;
    bool foundFirstNewline = false;
    
    while (continueReceivingData.test_and_set()) {
        // ZMQ_STREAM sockets always return two-part messages.  We care only about the second part,
        // which contains the actual data from the eye tracker.
        if (ZeroMQSocket::Result::ok == msg.recv(socket) &&
            msg.isComplete())
        {
            auto first = reinterpret_cast<const char *>(msg.getData());
            auto last = first + msg.getSize();
            
            while (true) {
                auto newline = std::find(first, last, '\n');
                receivedData.append(first, newline);
                
                if (newline == last) {
                    break;
                }
                
                if (!foundFirstNewline) {
                    // Ignore all data received before the first newline, in case we started receiving
                    // mid-message
                    foundFirstNewline = true;
                } else if (running) {
                    long long x = -1;
                    long long y = -1;
                    if (2 != std::sscanf(receivedData.c_str(), "%lld %lld", &x, &y)) {
                        merror(M_IODEVICE_MESSAGE_DOMAIN, "Received invalid message from EasyEye device");
                    } else {
                        auto currentTime = Clock::instance()->getCurrentTimeUS();
                        eyeX->setValue(x, currentTime);
                        eyeY->setValue(y, currentTime);
                    }
                }
                
                receivedData.clear();
                first = newline + 1;
            }
        }
    }
}


END_NAMESPACE_MW


























