/*
 *  NE500PumpNetworkDevice.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500PumpNetworkDevice.h"


BEGIN_NAMESPACE_MW


const std::string NE500PumpNetworkDevice::ADDRESS("address");
const std::string NE500PumpNetworkDevice::PORT("port");
const std::string NE500PumpNetworkDevice::RESPONSE_TIMEOUT("response_timeout");
const std::string NE500PumpNetworkDevice::LOG_PUMP_COMMANDS("log_pump_commands");


void NE500PumpNetworkDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/ne500");
    
    info.addParameter(ADDRESS);
    info.addParameter(PORT);
    info.addParameter(RESPONSE_TIMEOUT, "100ms");
    info.addParameter(LOG_PUMP_COMMANDS, "YES");
}


NE500PumpNetworkDevice::NE500PumpNetworkDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    address(VariablePtr(parameters[ADDRESS])->getValue().getString()),
    port(parameters[PORT]),
    response_timeout(parameters[RESPONSE_TIMEOUT]),
    logPumpCommands(parameters[LOG_PUMP_COMMANDS]),
    s(-1),
    connected(false),
    active(false)
{ }


NE500PumpNetworkDevice::~NE500PumpNetworkDevice() {
    disconnectFromDevice();
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
    if (!connectToDevice()) {
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


bool NE500PumpNetworkDevice::connectToDevice() {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN,
            "Connecting to NE500 Pump Network...");
    
    struct hostent *hostp;
    if (NULL == (hostp = gethostbyname(address.c_str()))) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Host lookup failed: %s: %s",
               address.c_str(),
               hstrerror(h_errno));
        return false;
    }
    
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    memcpy((char *)&addr.sin_addr,hostp->h_addr,hostp->h_length);
    addr.sin_family = hostp->h_addrtype;
    addr.sin_port = htons((u_short)port);
    
    if ((s = socket(PF_INET, SOCK_STREAM, 6)) < 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Socket creation failed: %s", strerror(errno));
        return false;
    }
    
    /* Set socket to non-blocking */
    int flags;
    if ((flags = fcntl(s, F_GETFL, 0)) < 0 ||
        fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot set socket to non-blocking: %s", strerror(errno));
        close(s);
        return false;
    }
    
    while(true){
        int rc = connect(s, (struct sockaddr *) &addr, sizeof(addr));
        int connect_errno = errno;
        
        if(rc >= 0 || connect_errno == EISCONN){
            break;
        }
        
        if (connect_errno != EALREADY &&
            connect_errno != EINPROGRESS &&
            connect_errno != EWOULDBLOCK)
        {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot connect to %s: %s", address.c_str(), strerror(connect_errno));
            close(s);
            return false;
        }
        
        if(connect_errno == EINPROGRESS){
            timeval timeout;
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
            fd_set fd_w;
            FD_ZERO(&fd_w);
            FD_SET(s,&fd_w);
            int select_err=select(FD_SETSIZE,NULL,&fd_w,NULL,&timeout);
            
            /* 0 means it timed out out & no fds changed */
            if(select_err==0) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Connection to NE500 device (%s) timed out", address.c_str());
                close(s);
                return false;
            }
        }
    }
    
    connected = true;
    return true;
}


void NE500PumpNetworkDevice::disconnectFromDevice() {
    if (connected) {
        if (close(s) < 0) {
            mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                     "Cannot disconnect from NE500 device (%s): %s",
                     address.c_str(),
                     strerror(errno));
        }
        connected = false;
    }
}


static inline std::string removeControlChars(std::string str) {
    boost::algorithm::erase_all(str, "\r");
    boost::algorithm::erase_all(str, "\x02");
    boost::algorithm::erase_all(str, "\x03");
    return std::move(str);
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
    
    if (logPumpCommands) {
        mprintf(M_IODEVICE_MESSAGE_DOMAIN, "SENT: %s", removeControlChars(message).c_str());
    }
    
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
            if (shared_pump_network->active) {
                shared_channel->dispense(shared_pump_network->getSendFunction());
            }
        }
    }
}


END_NAMESPACE_MW




























