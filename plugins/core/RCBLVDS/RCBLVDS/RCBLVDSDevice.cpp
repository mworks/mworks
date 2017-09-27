//
//  RCBLVDSDevice.cpp
//  RCBLVDS
//
//  Created by Christopher Stawarz on 9/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "RCBLVDSDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string RCBLVDSDevice::ADDRESS("address");


void RCBLVDSDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/rcb_lvds");
    
    info.addParameter(ADDRESS);
}


RCBLVDSDevice::RCBLVDSDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    address(variableOrText(parameters[ADDRESS])->getValue().getString()),
    urlSession(nil),
    running(false)
{ }


RCBLVDSDevice::~RCBLVDSDevice() {
    @autoreleasepool {
        [urlSession release];
    }
}


void RCBLVDSDevice::addChild(std::map<std::string, std::string> parameters,
                             ComponentRegistryPtr reg,
                             boost::shared_ptr<Component> child)
{
    if (auto channel = boost::dynamic_pointer_cast<RCBLVDSDigitalInputChannel>(child)) {
        for (auto &channelNum : channel->getChannelNumbers()) {
            if (!(activeDigitalInputChannels.insert(channelNum).second)) {
                throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                      (boost::format("Channel %d is already in use on RCB-LVDS device \"%s\"")
                                       % channelNum
                                       % getTag()));
            }
        }
        digitalInputChannels.push_back(channel);
        return;
    }
    throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for RCB-LVDS device");
}


bool RCBLVDSDevice::initialize() {
    @autoreleasepool {
        NSURLSessionConfiguration *config = NSURLSessionConfiguration.ephemeralSessionConfiguration;
        config.timeoutIntervalForResource = 2;  // 2-second timeout for requests to complete
        urlSession = [[NSURLSession sessionWithConfiguration:config] retain];
        
        std::string statusInfo;
        if (!performRequest("/intan_status.html", statusInfo)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot obtain status of RCB-LVDS device");
            return false;
        }
        mprintf(M_IODEVICE_MESSAGE_DOMAIN,
                "RCB-LVDS device \"%s\" returned the following status info:\n\n%s",
                getTag().c_str(),
                statusInfo.c_str());
        
        boost::weak_ptr<RCBLVDSDevice> weakThis(component_shared_from_this<RCBLVDSDevice>());
        for (auto &channel : digitalInputChannels) {
            auto callback = [weakThis, &channel](const Datum &data, MWTime time) {
                if (auto sharedThis = weakThis.lock()) {
                    unique_lock lock(sharedThis->mutex);
                    if (sharedThis->running) {
                        auto value = data.getInteger();
                        if ((value >> channel->getChannelNumbers().size()) != 0) {
                            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                                     "RCB-LVDS digital input channel value (%lld) contains too many set bits; "
                                     "the full value cannot be sent to the device",
                                     value);
                        }
                        sharedThis->updateDigitalInputChannels();
                    }
                }
            };
            channel->getValueVariable()->addNotification(boost::make_shared<VariableCallbackNotification>(callback));
        }
        
        return true;
    }
}


bool RCBLVDSDevice::startDeviceIO() {
    unique_lock lock(mutex);
    if (!running) {
        updateDigitalInputChannels();
        running = true;
    }
    return true;
}


bool RCBLVDSDevice::stopDeviceIO() {
    unique_lock lock(mutex);
    if (running) {
        setDigitalInputChannels(0);
        running = false;
    }
    return true;
}


void RCBLVDSDevice::updateDigitalInputChannels() {
    std::uint16_t state = 0;
    
    for (auto &channel : digitalInputChannels) {
        auto value = channel->getValueVariable()->getValue().getInteger();
        for (auto &channelNum : channel->getChannelNumbers()) {
            state |= (value & 1) << channelNum;
            value >>= 1;
        }
    }
    
    setDigitalInputChannels(state);
}


void RCBLVDSDevice::setDigitalInputChannels(std::uint16_t state) {
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') << state;
    
    if (!performRequest("__SL_P_UDI=M" + oss.str())) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot set digital input channels on RCB-LVDS device");
    }
}


bool RCBLVDSDevice::performRequest(const std::string &path, std::string &responseBody) {
    return performRequest(path, "", responseBody);
}


bool RCBLVDSDevice::performRequest(const std::string &requestBody) {
    std::string responseBody;
    return performRequest("/", requestBody, responseBody);
}


bool RCBLVDSDevice::performRequest(const std::string &path, const std::string &requestBody, std::string &responseBody) {
    @autoreleasepool {
        NSString *urlString = [NSString stringWithFormat:@"http://%s%s", address.c_str(), path.c_str()];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlString]];
        NSInteger expectedStatusCode = 204;  // "No Content"
        
        if (requestBody.empty()) {
            expectedStatusCode = 200;  // "OK"
        } else {
            request.HTTPMethod = @"POST";
            [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
            [request setValue:[NSString stringWithFormat:@"%lu", requestBody.size()] forHTTPHeaderField:@"Content-Length"];
            request.HTTPBody = [NSData dataWithBytes:requestBody.c_str() length:requestBody.size()];
        }
        
        std::promise<bool> p;
        auto f = p.get_future();
        auto completionHandler = [expectedStatusCode, &responseBody, &p](NSData *data, NSURLResponse *response, NSError *error) {
            bool success = false;
            if (error) {
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "Request to RCB-LVDS device failed: %s",
                       error.localizedDescription.UTF8String);
            } else {
                auto statusCode = static_cast<NSHTTPURLResponse *>(response).statusCode;
                if (statusCode != expectedStatusCode) {
                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                           "RCB-LVDS device returned unexpected status code (%ld)",
                           statusCode);
                } else {
                    responseBody.assign(static_cast<const char *>(data.bytes), data.length);
                    success = true;
                }
            }
            p.set_value(success);
        };
        
        [[urlSession dataTaskWithRequest:request completionHandler:completionHandler] resume];
        
        return f.get();
    }
}


END_NAMESPACE_MW





















