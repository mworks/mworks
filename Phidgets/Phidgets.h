/*
 *  Phidgets.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/28/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <Phidget21/phidget21.h>
#include <string>
#include <boost/format.hpp>
#include "MonkeyWorksCore/Utilities.h"
#include "MonkeyWorksCore/Plugin.h"
#include "MonkeyWorksCore/IODevice.h"


using namespace std;



int AttachHandler(CPhidgetHandle IFK, void *userptr);
int DetachHandler(CPhidgetHandle IFK, void *userptr);
int ErrorHandler(CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown);
int InputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State);
int OutputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State);
int SensorChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int Value);

namespace mw{

enum PhidgetChannelType{  M_PHIDGET_DIGITAL_INPUT, M_PHIDGET_DIGITAL_OUTPUT, M_PHIDGET_ANALOG_INPUT };

class PhidgetDeviceChannel : public Component {

	protected:
	
		PhidgetChannelType channel_type;
		int index;
		
		CPhidgetInterfaceKitHandle ifKit;
		
		shared_ptr<Variable> variable;
		
	public:
	
		PhidgetDeviceChannel(PhidgetChannelType _type, int _index, shared_ptr<Variable> _variable){
			channel_type = _type;
			index = _index;
			
			ifKit = NULL;
			
			variable = _variable;
		}
		
		
		void setDevice(CPhidgetInterfaceKitHandle _ifKit){
			ifKit = _ifKit;
		}
		
		CPhidgetInterfaceKitHandle getDevice(){
			return ifKit;
		}
		
		shared_ptr<Variable> getVariable(){
			return variable;
		}
		
		int getIndex(){
			return index;
		}
		
		PhidgetChannelType getType(){
			return channel_type;
		}
		
};

class PhidgetDeviceChannelFactory : public ComponentFactory {

	shared_ptr<Component> createObject(std::map<std::string, std::string> parameters,
													 ComponentRegistry *reg) {
		
		REQUIRE_ATTRIBUTES(parameters, "variable", "capability", "index");
		
		string capability_string = to_lower_copy(parameters["capability"]);
		PhidgetChannelType type;
		int index = reg->getNumber(parameters["index"]);
		
		if(capability_string == "digital_input"){
			type = M_PHIDGET_DIGITAL_INPUT;
		} else if(capability_string == "digital_output"){
			type = M_PHIDGET_DIGITAL_OUTPUT;
		} else if(capability_string == "analog_input"){
			type = M_PHIDGET_ANALOG_INPUT;
		} else {
			throw SimpleException("Unknown phidget channel type", capability_string);
		}
		
		shared_ptr<Variable> variable = reg->getVariable(parameters["variable"]);
		
		shared_ptr <Component> new_channel(new PhidgetDeviceChannel(type, index, variable));
		return new_channel;
	}

};


class PhidgetDevice : public IODevice {

	protected:
	
		CPhidgetInterfaceKitHandle ifKit;

		bool active;
		boost::mutex active_mutex;	

		map< int, weak_ptr<PhidgetDeviceChannel> > analog_input_channels;
		map< int, weak_ptr<PhidgetDeviceChannel> > digital_input_channels;
		map< int, weak_ptr<PhidgetDeviceChannel> > digital_output_channels;

	public:
	
		PhidgetDevice(){
		
            int err_code = EPHIDGET_OK;
            
			active = false;
		
			//Declare an InterfaceKit handle
			ifKit = 0;

            
			setActive(false);
			//create the InterfaceKit object
			err_code = CPhidgetInterfaceKit_create(&ifKit);
            
            if(err_code != EPHIDGET_OK){
                throw SimpleException("Unable to connect to Phidgets device.  Is the Phidgets software installed on this machine? (www.phidgets.com)");
            }
            
			//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
			CPhidget_set_OnAttach_Handler((CPhidgetHandle)ifKit, AttachHandler, NULL);
			CPhidget_set_OnDetach_Handler((CPhidgetHandle)ifKit, DetachHandler, NULL);
			CPhidget_set_OnError_Handler((CPhidgetHandle)ifKit, ErrorHandler, NULL);

			//Registers a callback that will run if an input changes.
			//Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
			CPhidgetInterfaceKit_set_OnInputChange_Handler (ifKit, InputChangeHandler, this);

			//Registers a callback that will run if the sensor value changes by more than the OnSensorChange trig-ger.
			//Requires the handle for the IntefaceKit, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
			CPhidgetInterfaceKit_set_OnSensorChange_Handler (ifKit, SensorChangeHandler, this);

			//Registers a callback that will run if an output changes.
			//Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
			//CPhidgetInterfaceKit_set_OnOutputChange_Handler (ifKit, OutputChangeHandler, this);

			//open the interfacekit for device connections
			CPhidget_open((CPhidgetHandle)ifKit, -1);

			//get the program to wait for an accelerometer device to be attached
			mprintf("Waiting for interface kit to be attached....");
			int result;
			const char *err;
			if((result = CPhidget_waitForAttachment((CPhidgetHandle)ifKit, 5000)))
			{
				CPhidget_getErrorDescription(result, &err);
				throw SimpleException("Problem waiting for attachment", err);
			}
		
			
			
		}
		
		virtual ~PhidgetDevice();
		
						
		shared_ptr<PhidgetDeviceChannel> getAnalogInputChannel(int i){
			weak_ptr<PhidgetDeviceChannel> candidate = analog_input_channels[i];
			if(!candidate.expired()){
				shared_ptr<PhidgetDeviceChannel> channel = candidate.lock();
				return channel;
			}
			
			return shared_ptr<PhidgetDeviceChannel>();
		}
		
		shared_ptr<PhidgetDeviceChannel> getDigitalInputChannel(int i){
			weak_ptr<PhidgetDeviceChannel> candidate = digital_input_channels[i];
			if(!candidate.expired()){
				shared_ptr<PhidgetDeviceChannel> channel = candidate.lock();
				return channel;
			}
			
			return shared_ptr<PhidgetDeviceChannel>();
		}
		
		
		// specify what this device can do
		virtual bool attachPhysicalDevice(){ return true; }
		virtual ExpandableList<IOCapability> *getCapabilities(){ return NULL; }
		virtual bool mapRequestsToChannels(){  return true;  }
		virtual bool initializeChannels(){  return true;  }
		virtual bool startup(){  
			return true;  
		}

		virtual void addChild(std::map<std::string, std::string> parameters,
								ComponentRegistry *reg,
								shared_ptr<Component> _child);
		
		virtual void setActive(bool _active){
			boost::mutex::scoped_lock active_lock(active_mutex);
			active = _active;
		}

		virtual bool getActive(){
			boost::mutex::scoped_lock active_lock(active_mutex);
			bool is_active = active;
			return is_active;
		}



		virtual bool startDeviceIO();
		virtual bool stopDeviceIO();
		
		
		// this will stop anyIO behavior on a device and put the device in a shutdown state (if the device has one) -- e.g. turn off x-ray
		virtual bool shutdown(){ return true; }


};


class PhidgetDeviceFactory : public ComponentFactory {

	shared_ptr<Component> createObject(std::map<std::string, std::string> parameters,
													 ComponentRegistry *reg);
};



class PhidgetDeviceOutputNotification : public VariableNotification {

	protected:
	
		
		CPhidgetInterfaceKitHandle ifk;
		int index;

	public:
	
		PhidgetDeviceOutputNotification(shared_ptr<PhidgetDeviceChannel> channel){
			index = channel->getIndex();
			ifk = channel->getDevice();

		}
	
		virtual void notify(const Data& data, MonkeyWorksTime timeUS){
			CPhidgetInterfaceKit_setOutputState(ifk, index, (int)(data.getBool())); 
		}
};


}





