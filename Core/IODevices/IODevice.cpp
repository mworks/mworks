#include "IODeviceVariableNotification.h"
#include "IODevice.h"

using namespace mw;


void IODevice::finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg)
{
	shared_ptr<IODevice> this_one = shared_from_this();
	shared_ptr<IODeviceVariableNotification> notification(new IODeviceVariableNotification(this_one));
	state_system_mode->addNotification(notification);			
	
	if(!initialize()) {
		// Initialization failed, so try to map the tag to the alt device
		if(parameters.find("alt") == parameters.end()) {
			throw SimpleException("Can't start iodevice (" + tag + ") and no alt tag specified");
		} else {
			shared_ptr <IODevice> alt_io_device = reg->getObject<IODevice>(parameters.find("alt")->second);
			reg->registerAltObject(tag, alt_io_device);
		}
	}
}
