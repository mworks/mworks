/*
 *  NE500DeviceChannel.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500DeviceChannel.h"


BEGIN_NAMESPACE_MW


const std::string NE500DeviceChannel::CAPABILITY("capability");
const std::string NE500DeviceChannel::VARIABLE("variable");
const std::string NE500DeviceChannel::SYRINGE_DIAMETER("syringe_diameter");
const std::string NE500DeviceChannel::FLOW_RATE("flow_rate");


void NE500DeviceChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    
    info.setSignature("iochannel/ne500");
    
    info.addParameter(CAPABILITY);
    info.addParameter(VARIABLE);
    info.addParameter(SYRINGE_DIAMETER);
    info.addParameter(FLOW_RATE);
}


NE500DeviceChannel::NE500DeviceChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    pump_id(boost::algorithm::to_lower_copy(parameters[CAPABILITY].str())),
    variable(parameters[VARIABLE]),
    syringe_diameter(parameters[SYRINGE_DIAMETER]),
    rate(parameters[FLOW_RATE])
{
    if (pump_id.empty()) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Attempt to access invalid (empty) NE500 pump id");
    }
}


/*
std::string NE500PumpNetworkDevice::formatFloat(double val) {
    val = std::max(val, 0.0);
    val = std::min(val, 9999.0);
    
    boost::format fmt;
    
    if (val < 10.0) {
        fmt = boost::format("%.3f");
    } else if (val < 100.0) {
        fmt = boost::format("%.2f");
    } else if (val < 1000.0) {
        fmt = boost::format("%.1f");
    } else {
        fmt = boost::format("%.0f");
    }
    
    return (fmt % val).str();
}


bool NE500PumpNetworkDevice::sendMessage(const std::string &pump_id, string message){
    if (!connected) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No connection to NE500 device (%s)", address.c_str());
        return false;
    }
    
    // Send converted line back to client.
    message = pump_id + " " + message + "\r";
    if (send(s, message.c_str(), message.size(), 0) < 0){
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Cannot send data to NE500 device (%s): %s",
               address.c_str(),
               strerror(errno));
        return false;
    }
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "SENT: %s", message.c_str());
    
    // give it a moment
    shared_ptr<Clock> clock = Clock::instance();
    MWTime tic = clock->getCurrentTimeUS();
    
    std::array<char, 512> recv_buffer;
    bool broken = false;
    string result;
    bool success = true;
    bool is_alarm = false;
    
    while(true){
        // Clear the buffer
        recv_buffer.fill(0);
        
        auto rc = recv(s, recv_buffer.data(), recv_buffer.size(), 0);

        if (rc > 0) {
            
            result.append(recv_buffer.data(), rc);
            
        } else {
        
            // If rc is zero, then the other side closed the connection.  Otherwise,
            // any result other than EWOULDBLOCK (basically, no data yet)
            // is a sign that an error has occurred.
            if (rc == 0 || errno != EWOULDBLOCK) {
                broken = true;
            }
            
            // if the response is complete
            // NE500 responses are of the form: "\t01S"
            if(result.size() > 1 && result[result.size() - 1] == PUMP_SERIAL_DELIMITER_CHAR){
                
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
                
            } else if((clock->getCurrentTimeUS() - tic) > response_timeout){
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "Did not receive a complete response from the pump");
                success = false;
                break;
            }
        }
    }
    
    if (broken) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Connection lost, reconnecting..."
               "Command may not have been sent correctly");
        disconnectFromDevice();
        connectToDevice();
    }
    
    if (!result.empty()) {
        if (is_alarm) {
            mwarning(M_IODEVICE_MESSAGE_DOMAIN, "Received alarm response from NE500 device: %s", result.c_str());
        } else {
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "RETURNED: %s", result.c_str());
        }
    }
    
    return success;
}


bool NE500PumpNetworkDevice::dispense(const std::string &pump_id, double rate, Datum data) {
    scoped_lock active_lock(active_mutex);
    
    if (!active) {
        return false;
    }
    
    double amount = data.getFloat();
    
    if (amount == 0.0) {
        return true;
    }
    
    string dir_str;
    if(amount >= 0){
        dir_str = "INF"; // infuse
    } else {
        amount *= -1.0;
        dir_str = "WDR"; // withdraw
    }
    
    return (sendMessage(pump_id, "DIR " + dir_str) &&
            sendMessage(pump_id, "RAT " + formatFloat(rate) + " MM") &&
            sendMessage(pump_id, "VOL " + formatFloat(amount)) &&
            sendMessage(pump_id, "RUN"));
}


bool NE500PumpNetworkDevice::initializePump(const std::string &pump_id, double rate, double syringe_diameter) {
    return (sendMessage(pump_id, "DIA " + formatFloat(syringe_diameter)) &&
            sendMessage(pump_id, "FUN RAT") &&
            sendMessage(pump_id, "RAT " + formatFloat(rate) + " MM"));
}
 */


END_NAMESPACE_MW




























