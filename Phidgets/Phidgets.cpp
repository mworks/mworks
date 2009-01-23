/*
 *  Phidgets.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/28/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Phidgets.h"
#include "MonkeyWorksCore/ComponentRegistry.h"

using namespace mw;


PhidgetDevice::~PhidgetDevice(){
	//since user input has been read, this is a signal to terminate the program so we will close the phidget and delete the object we created
	
	stopDeviceIO();
	
	if(ifKit){
		CPhidget_close((CPhidgetHandle)ifKit);
		CPhidget_delete((CPhidgetHandle)ifKit);
	}
}
		


bool PhidgetDevice::startDeviceIO(){
	setActive(true);
	return true;
}
	

bool PhidgetDevice::stopDeviceIO(){
	setActive(false);
	return true;
}


void PhidgetDevice::addChild(std::map<std::string, std::string> parameters,
								ComponentRegistry *reg,
								shared_ptr<Component> _child){
	shared_ptr<PhidgetDeviceChannel> child = dynamic_pointer_cast<PhidgetDeviceChannel, Component>(_child);
	
	
	if(child == NULL){
		throw SimpleException("Failed to cast phidget device channel");
	}
	
	int index = child->getIndex();
	PhidgetChannelType type = child->getType();
	
	child->setDevice(ifKit);
	
	weak_ptr<PhidgetDeviceChannel> wchild(child);
	
	if(type == M_PHIDGET_ANALOG_INPUT){
		analog_input_channels[index] = wchild;
	} else if(type == M_PHIDGET_DIGITAL_INPUT){
		digital_input_channels[index] = wchild;
	} else if(type == M_PHIDGET_DIGITAL_OUTPUT){
		digital_output_channels[index] = wchild;
		shared_ptr<Variable> var = child->getVariable();
		shared_ptr<VariableNotification> notif(new PhidgetDeviceOutputNotification(child));
		var->addNotification(notif);
	} else {
		throw SimpleException("Unknown phidget channel type");
	}
}


int AttachHandler(CPhidgetHandle IFK, void *userptr)
{
	int serialNo;
	const char *name;

	CPhidget_getDeviceName(IFK, &name);
	CPhidget_getSerialNumber(IFK, &serialNo);

	printf("%s %10d attached!\n", name, serialNo);

	return 0;
}

int DetachHandler(CPhidgetHandle IFK, void *userptr)
{
	int serialNo;
	const char *name;

	CPhidget_getDeviceName (IFK, &name);
	CPhidget_getSerialNumber(IFK, &serialNo);

	printf("%s %10d detached!\n", name, serialNo);

	return 0;
}

int ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown)
{
	printf("Error handled. %i - %s", ErrorCode, unknown);
	return 0;
}

//callback that will run if an input changes.
//Index - Index of the input that generated the event, State - boolean (0 or 1) representing the input state (on or off)
int InputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State)
{
	PhidgetDevice *device = (PhidgetDevice *)usrptr;
	
	if(device->getActive()){
	
		shared_ptr<PhidgetDeviceChannel> channel = device->getDigitalInputChannel(Index);
		
		if(channel == NULL){
			return 0;
		}
		
		printf("Digital input state: %d > Value: %d\n", Index, State);
		
		shared_ptr<Variable> var = channel->getVariable();
		if(var != NULL){
			var->setValue((long)State);
		}
	}
	
	return 0;
}

//callback that will run if an output changes.
//Index - Index of the output that generated the event, State - boolean (0 or 1) representing the output state (on or off)
int OutputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State)
{
	printf("Digital Output: %d > State: %d\n", Index, State);
	return 0;
}

//callback that will run if the sensor value changes by more than the OnSensorChange trigger.
//Index - Index of the sensor that generated the event, Value - the sensor read value
int SensorChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int Value)
{
	
	
	PhidgetDevice *device = (PhidgetDevice *)usrptr;
	
	if(device->getActive()){
	
		shared_ptr<PhidgetDeviceChannel> channel = device->getAnalogInputChannel(Index);
		
		if(channel == NULL){
			mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Invalid channel change trigger from phidgets device");
			return 0;
		}
		
		printf("Sensor: %d > Value: %d\n", Index, Value);
		
		shared_ptr<Variable> var = channel->getVariable();
		var->setValue((long)Value);
	}
	
	return 0;
}



shared_ptr<mw::Component> PhidgetDeviceFactory::createObject(std::map<std::string, std::string> parameters,
													 ComponentRegistry *reg) {
    shared_ptr <mw::Component> newDevice = shared_ptr<Component>(new PhidgetDevice());
		return newDevice;
	}