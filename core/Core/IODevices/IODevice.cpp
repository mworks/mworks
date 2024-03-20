#include "IODevice.h"


BEGIN_NAMESPACE_MW


const std::string IODevice::AUTOSTART("autostart");
const std::string IODevice::ALT("alt");


void IODevice::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(AUTOSTART, "NO");
    info.addParameter(ALT, false);
}


IODevice::IODevice(const ParameterValueMap &parameters) :
    Component(parameters),
    autoStart(parameters[AUTOSTART])
{ }


IODevice::IODevice() :
    autoStart(false)
{ }


IODevice::~IODevice() {
    if (stateSystemCallbackNotification) {
        stateSystemCallbackNotification->remove();
    }
}


void IODevice::finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg) {
    if (initialize()) {
        auto callback = [this](const Datum &data, MWorksTime time) { stateSystemCallback(data, time); };
        stateSystemCallbackNotification = boost::make_shared<VariableCallbackNotification>(callback);
        state_system_mode->addNotification(stateSystemCallbackNotification);
    } else {
        //
        // Initialization failed, so try to map the tag to the alt device
        //
        
        const auto altIter = parameters.find(ALT);
        if (altIter == parameters.end()) {
            throw SimpleException("Can't start iodevice (" + getTag() + ") and no alt tag specified");
        }
        
        if (!alt_failover->getValue().getBool()) {
            throw SimpleException("Can't start iodevice (" + getTag() + ");" +
                                  " an 'alt' object is specified, but the " + ALT_FAILOVER_TAGNAME +
                                  " system variable is set to disallow failover");
        }
        
        const auto &altTag = altIter->second;
        auto alt_io_device = reg->getObject<IODevice>(altTag);
        if (!alt_io_device) {
            throw SimpleException("Can't start iodevice (" + getTag() + "), and the specified" +
                                  " 'alt' object (" + altTag + ") does not exist or is not an iodevice");
        }
        
        if (alt_io_device.get() == this) {
            throw SimpleException("Can't start iodevice (" + getTag() + "), and the specified" +
                                  " 'alt' object is the iodevice itself");
        }
        
        reg->registerAltObject(getTag(), alt_io_device);
    }
}


void IODevice::stateSystemCallback(const Datum &data, MWorksTime time) {
    switch (data.getInteger()) {
        case IDLE:
            stopDeviceIO();
            break;
            
        case RUNNING:
            if (autoStart) {
                startDeviceIO();
            }
            break;
            
        default:
            break;
    }
}


END_NAMESPACE_MW



























