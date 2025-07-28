//
//  LSLDevice.cpp
//  LSL
//
//  Created by Christopher Stawarz on 7/29/25.
//

#include "LSLDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string LSLDevice::STREAM_INFO("stream_info");
const std::string LSLDevice::SAMPLE_DATA("sample_data");
const std::string LSLDevice::SAMPLE_TIME("sample_time");


void LSLDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/lsl");

    info.addParameter(STREAM_INFO);
    info.addParameter(SAMPLE_DATA);
    info.addParameter(SAMPLE_TIME, false);
}


LSLDevice::LSLDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    streamInfo(variableOrText(parameters[STREAM_INFO])),
    sampleData(parameters[SAMPLE_DATA]),
    sampleTime(optionalVariable(parameters[SAMPLE_TIME])),
    clock(Clock::instance()),
    running(false)
{ }


LSLDevice::~LSLDevice() {
    if (receiveDataThread.joinable()) {
        running = false;
        receiveDataThread.join();
    }
}


bool LSLDevice::initialize() {
    lock_guard lock(mutex);
    
    lsl_streaminfo _info = nullptr;
    auto numMatches = lsl_resolve_bypred(&_info, 1, streamInfo->getValue().getString().c_str(), 1, resolveTimeout);
    
    if (numMatches < 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LSL stream resolve failed: %s", lsl_last_error());
        return false;
    }
    
    if (numMatches == 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LSL resolve found no matching streams");
        return false;
    }
    
    info.reset(_info);
    
    return true;
}


bool LSLDevice::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        running = true;
        receiveDataThread = std::thread([this]() {
            receiveData();
        });
    }
    
    return true;
}


bool LSLDevice::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (running) {
        running = false;
        receiveDataThread.join();
    }
    
    return true;
}


void LSLDevice::receiveData() {
    //
    // NOTE: We should be able to create the inlet once (in initialize) and
    // open and close the stream repeatedly, but this doesn't work at present:
    // https://github.com/sccn/liblsl/issues/180
    //
    inlet.reset(lsl_create_inlet(info.get(), 360, 0, 1));
    if (!inlet) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LSL stream inlet creation failed: %s", lsl_last_error());
        return;
    }
    
    int32_t error = lsl_no_error;
    
    lsl_open_stream(inlet.get(), openTimeout, &error);
    if (lsl_no_error != error) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LSL stream open failed: %s", lsl_last_error());
        return;
    }
    
    std::vector<double> sample(lsl_get_channel_count(info.get()), 0.0);
    double time = 0.0;
    auto value = Datum(Datum::list_value_type(sample.size()));
    
    while (running) {
        time = lsl_pull_sample_d(inlet.get(), sample.data(), sample.size(), pullSampleTimeout, &error);
        if (lsl_no_error != error) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "LSL stream sample pull failed: %s", lsl_last_error());
            break;
        }
        if (0.0 != time) {
            auto eventTime = clock->getCurrentTimeUS();
            
            if (sample.size() == 1) {
                sampleData->setValue(sample.front(), eventTime);
            } else {
                for (int i = 0; i < sample.size(); i++) {
                    value.setElement(i, sample[i]);
                }
                sampleData->setValue(value, eventTime);
            }
            
            if (sampleTime) {
                sampleTime->setValue(time, eventTime);
            }
        }
    }
    
    lsl_close_stream(inlet.get());
}


END_NAMESPACE_MW
