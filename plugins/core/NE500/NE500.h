/*
 *  NE500.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_NE500_H_
#define _NE500_H_

#include <string>
#include <boost/format.hpp>
#include "MWorksCore/Utilities.h"
#include "MWorksCore/Plugin.h"
#include "MWorksCore/IODevice.h"
#include "MWorksCore/ComponentFactory.h"
#include "MWorksCore/ComponentRegistry.h"

using std::string;
using std::vector;


namespace mw {

class NE500DeviceOutputNotification;

class NE500DeviceChannel : public Component {

	protected:
	
		string pump_id;
		shared_ptr<Variable> variable;
		
		double syringe_diameter;
		shared_ptr<Variable> rate;
		
	public:
	
		NE500DeviceChannel(string _pump_id, 
							shared_ptr<Variable> _variable,
							double _syringe_diameter,
							shared_ptr<Variable> _rate){
			
			pump_id = _pump_id;
			variable = _variable;
			syringe_diameter = _syringe_diameter;
			rate = _rate;
			
			if(rate == NULL){
				throw SimpleException("Invalid rate variable in NE500 pump channel");
			}
		}
		
		
		shared_ptr<Variable> getVariable(){
			return variable;
		}
		
		string getPumpID(){
			return pump_id;
		}
		
		double getSyringeDiameter(){ return syringe_diameter; }
		double getRate(){ 
			return (double)(rate->getValue()); 
		}
		

};

class NE500DeviceChannelFactory : public ComponentFactory {

	shared_ptr<Component> createObject(std::map<std::string, std::string> parameters,
                                                    ComponentRegistry *reg) {
		
		REQUIRE_ATTRIBUTES(parameters, "variable", "capability", "syringe_diameter", "flow_rate");
		
		string capability_string = boost::algorithm::to_lower_copy(parameters["capability"]);
		
		shared_ptr<Variable> variable = reg->getVariable(parameters["variable"]);
		
		double syringe_diameter = reg->getNumber(parameters["syringe_diameter"]);
		shared_ptr<Variable> rate = reg->getVariable(parameters["flow_rate"]);
		
		shared_ptr <Component> new_channel(new NE500DeviceChannel(capability_string, variable, syringe_diameter, rate));
		return new_channel;
	}

};


class NE500PumpNetworkDevice : public IODevice {

	protected:
	
		string address;
		int port;
		
		// the socket
		int s;
		
		bool connected;
		
		vector<string> pump_ids;
		
		bool active;
		boost::mutex active_mutex;	

	public:
	
		NE500PumpNetworkDevice(string _address, int _port){
		
			address = _address;
			port = _port;
			
			// TODO: connect to serial server
			connectToDevice();
		}
		
		virtual ~NE500PumpNetworkDevice(){ 
			disconnectFromDevice();
		}
		
						
		virtual void connectToDevice();
		
		virtual void disconnectFromDevice();
		
		virtual void reconnectToDevice(){
			disconnectFromDevice();
			connectToDevice();
		}
		
		string sendMessage(string message);
		
		virtual void dispense(string pump_id, double rate, Datum data){
			
			if(getActive()){
				//initializePump(pump_id, 750.0, 20.0);
                double amount = (double)data;
                
				if(amount == 0.0){
					return;
				}

                
                string dir_str;
                if(amount >= 0){
                    dir_str = "INF"; // infuse
                } else {
                    amount *= -1.0;
                    dir_str = "WDR"; // withdraw
                }

                boost::format direction_message_format("%s DIR %s"); 
                string direction_message = (direction_message_format % pump_id % dir_str).str();
                
                sendMessage(direction_message); 
                
				boost::format rate_message_format("%s RAT %.1f MM"); 
				string rate_message = (rate_message_format % pump_id % rate).str();
				
				sendMessage(rate_message);
						
				boost::format message_format("%s VOL %.3f"); 
				string message = (message_format % pump_id % amount).str();
				
				sendMessage(message);
				
//				boost::format program_message_format("%s FUN RAT");
//				string program_message = (program_message_format % pump_id).str();
//				
//				sendMessage(program_message);
				
				boost::format run_message_format("%s RUN"); 
				string run_message = (run_message_format % pump_id).str();
				
				sendMessage(run_message);
			}
		}
	
		virtual void initializePump(string pump_id, double rate, double syringe_diameter){
			
			boost::format function_message_format("%s FUN RAT"); 
			string function_message = (function_message_format % pump_id).str();
			
			sendMessage(function_message);
			
			
			boost::format diameter_message_format("%s DIA %g"); 
			string diameter_message = (diameter_message_format % pump_id % syringe_diameter).str();
			
			sendMessage(diameter_message);
			
			
			boost::format rate_message_format("%s RAT %g"); 
			string rate_message = (rate_message_format % pump_id % rate).str();
			
			sendMessage(rate_message);
		}
		
		// specify what this device can do
		virtual bool initialize(){  return connected;  }

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



		virtual bool startDeviceIO(){  setActive(true); return true; }
		virtual bool stopDeviceIO(){  setActive(false); return true; }


};


class NE500DeviceFactory : public ComponentFactory {

	shared_ptr<Component> createObject(std::map<std::string, std::string> parameters,
													 ComponentRegistry *reg);
};


class NE500DeviceOutputNotification : public VariableNotification {

	protected:
	
		weak_ptr<NE500PumpNetworkDevice> pump_network;
		weak_ptr<NE500DeviceChannel> channel;
		//string pump_id;
	
	
	public:
	
		NE500DeviceOutputNotification(weak_ptr<NE500PumpNetworkDevice> _pump_network,
									   weak_ptr<NE500DeviceChannel> _channel){
			pump_network = _pump_network;
			channel = _channel;

		}
	
		virtual void notify(const Datum& data, MWorksTime timeUS){

			shared_ptr<NE500PumpNetworkDevice> shared_pump_network(pump_network);
			shared_ptr<NE500DeviceChannel> shared_channel(channel);
			shared_pump_network->dispense(shared_channel->getPumpID(), shared_channel->getRate(), data);
			
		}
};

} // namespace mw

#endif






