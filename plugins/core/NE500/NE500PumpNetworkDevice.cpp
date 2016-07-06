/*
 *  NE500PumpNetworkDevice.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500PumpNetworkDevice.h"

#include "NE500SerialConnection.hpp"
#include "NE500SocketConnection.hpp"


BEGIN_NAMESPACE_MW


const std::string NE500PumpNetworkDevice::ADDRESS("address");
const std::string NE500PumpNetworkDevice::PORT("port");
const std::string NE500PumpNetworkDevice::RESPONSE_TIMEOUT("response_timeout");
const std::string NE500PumpNetworkDevice::LOG_PUMP_COMMANDS("log_pump_commands");


void NE500PumpNetworkDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/ne500");
    
    info.addParameter(ADDRESS, false);
    info.addParameter(PORT, false);
    info.addParameter(RESPONSE_TIMEOUT, "100ms");
    info.addParameter(LOG_PUMP_COMMANDS, "YES");
}


NE500PumpNetworkDevice::NE500PumpNetworkDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    response_timeout(parameters[RESPONSE_TIMEOUT]),
    logPumpCommands(parameters[LOG_PUMP_COMMANDS]),
    active(false)
{
    std::string address;
    if (!parameters[ADDRESS].empty()) {
        address = VariablePtr(parameters[ADDRESS])->getValue().getString();
    }
    
    if (parameters[PORT].empty()) {
        connection.reset(new NE500SerialConnection(address));
    } else {
        if (address.empty()) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Address required for NE500 socket connection");
        }
        connection.reset(new NE500SocketConnection(address, int(parameters[PORT])));
    }
}


NE500PumpNetworkDevice::~NE500PumpNetworkDevice() {
    connection->disconnect();
}


void NE500PumpNetworkDevice::addChild(std::map<std::string, std::string> parameters,
                                      ComponentRegistry *reg,
                                      shared_ptr<Component> _child)
{
    shared_ptr<NE500DeviceChannel> channel = boost::dynamic_pointer_cast<NE500DeviceChannel, Component>(_child);
    if (!channel){
        throw SimpleException("Attempt to access an invalid NE500 channel object");
    }
    pumps.push_back(channel);
}


bool NE500PumpNetworkDevice::initialize() {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Connecting to NE500 Pump Network...");
    if (!connection->connect()) {
        return false;
    }
    
    const auto sendFunc = getSendFunction();
    
    for (auto &channel : pumps) {
        if (!channel->initialize(sendFunc)) {
            return false;
        }
        
        shared_ptr<Variable> var = channel->getVariable();
        shared_ptr<NE500PumpNetworkDevice> shared_self_ref(component_shared_from_this<NE500PumpNetworkDevice>());
        shared_ptr<VariableNotification> notif(new NE500DeviceOutputNotification(shared_self_ref, channel));
        var->addNotification(notif);
    }
    
    return true;
}


bool NE500PumpNetworkDevice::startDeviceIO() {
    scoped_lock active_lock(active_mutex);
    active = true;
    return true;
}


bool NE500PumpNetworkDevice::stopDeviceIO() {
    scoped_lock active_lock(active_mutex);
    active = false;
    return true;
}


static inline std::string removeControlChars(std::string str) {
    boost::algorithm::erase_all(str, "\r");
    boost::algorithm::erase_all(str, "\x02");
    boost::algorithm::erase_all(str, "\x03");
    return std::move(str);
}


bool NE500PumpNetworkDevice::sendMessage(const std::string &pump_id, string message) {
    if (!connection->isConnected()) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No connection to NE500 device");
        return false;
    }
    
    message = pump_id + " " + message + "\r";
    if (!connection->send(message)) {
        return false;
    }
    
    if (logPumpCommands) {
        mprintf(M_IODEVICE_MESSAGE_DOMAIN, "SENT: %s", removeControlChars(message).c_str());
    }
    
    // give it a moment
    shared_ptr<Clock> clock = Clock::instance();
    MWTime tic = clock->getCurrentTimeUS();
    
    bool broken = false;
    string result;
    bool success = true;
    bool is_alarm = false;
    
    while (true) {
        if (!connection->receive(result)) {
            broken = true;
        }
        
        // if the response is complete
        // NE500 responses are of the form: "\t01S"
        if (result.size() > 1 && result.back() == PUMP_SERIAL_DELIMITER_CHAR) {
            
            if(result.size() > 3){
                char status_char = result[3];
                
                
                switch(status_char){
                    case 'S':
                    case 'W':
                    case 'I':
                        break;
                        
                    case 'A':
                        is_alarm = true;
                        break;
                        
                    default:
                        merror(M_IODEVICE_MESSAGE_DOMAIN,
                               "An unknown response was received from the syringe pump: %c", status_char);
                        success = false;
                        break;
                }
            }
            
            if (result.size() > 4 && !is_alarm) {
                
                char error_char = result[4];
                if(error_char == '?'){
                    string error_code = result.substr(5, result.size() - 6);
                    string err_str("");
                    
                    if(error_code == ""){
                        err_str = "Unrecognized command";
                    } else if(error_code == "OOR"){
                        err_str = "Out of Range";
                    } else if(error_code == "NA"){
                        err_str = "Command currently not applicable";
                    } else if(error_code == "IGN"){
                        err_str = "Command ignored";
                    } else if(error_code == "COM"){
                        err_str = "Communications failure";
                    } else {
                        err_str = "Unspecified error";
                    }
                    
                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                           "The syringe pump returned an error: %s (%s)", err_str.c_str(), error_code.c_str());
                    success = false;
                }
            }
            
            break;
            
        } else if ((clock->getCurrentTimeUS() - tic) > response_timeout) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Did not receive a complete response from the pump");
            success = false;
            break;
        }
    }
    
    if (broken) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Connection lost, reconnecting..."
               "Command may not have been sent correctly");
        connection->disconnect();
        connection->connect();
    }
    
    if (!result.empty()) {
        if (is_alarm) {
            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                     "Received alarm response from NE500 device: %s",
                     removeControlChars(result).c_str());
        } else if (logPumpCommands) {
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "RETURNED: %s", removeControlChars(result).c_str());
        }
    }
    
    return success;
}


void NE500PumpNetworkDevice::NE500DeviceOutputNotification::notify(const Datum &data, MWTime timeUS) {
    if (auto shared_pump_network = pump_network.lock()) {
        if (auto shared_channel = channel.lock()) {
            scoped_lock active_lock(shared_pump_network->active_mutex);
            auto sendMessage = shared_pump_network->getSendFunction();
            
            if (shared_channel->update(sendMessage) &&
                shared_pump_network->active)
            {
                shared_channel->dispense(sendMessage);
            }
        }
    }
}


END_NAMESPACE_MW




























