#include "IODevice.h"
#include "IODeviceVariableNotification.h"


BEGIN_NAMESPACE_MW


const std::string IODevice::ALT("alt");


void IODevice::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(ALT, false);
}


void IODevice::finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg)
{
	shared_ptr<IODevice> this_one = component_shared_from_this<IODevice>();
	shared_ptr<IODeviceVariableNotification> notification(new IODeviceVariableNotification(this_one));
	state_system_mode->addNotification(notification);			
	
	if(!initialize()) {
		// Initialization failed, so try to map the tag to the alt device
		if(parameters.find(ALT) == parameters.end()) {
			throw SimpleException("Can't start iodevice (" + getTag() + ") and no alt tag specified");
		} else {
			shared_ptr <IODevice> alt_io_device = reg->getObject<IODevice>(parameters[ALT]);
			reg->registerAltObject(getTag(), alt_io_device);
		}
	}
}


END_NAMESPACE_MW
