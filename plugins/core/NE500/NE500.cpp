/*
 *  NE500.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500.h"


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


const std::string NE500PumpNetworkDevice::ADDRESS("address");
const std::string NE500PumpNetworkDevice::PORT("port");
const std::string NE500PumpNetworkDevice::RESPONSE_TIMEOUT("response_timeout");


void NE500PumpNetworkDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/ne500");
    
    info.addParameter(ADDRESS);
    info.addParameter(PORT);
    info.addParameter(RESPONSE_TIMEOUT, "100ms");
}


NE500PumpNetworkDevice::NE500PumpNetworkDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    address(VariablePtr(parameters[ADDRESS])->getValue().getString()),
    port(parameters[PORT]),
    response_timeout(parameters[RESPONSE_TIMEOUT]),
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
    connectToDevice();
    
    for (auto &channel : pumps) {
        initializePump(channel->getPumpID(), channel->getRate(), channel->getSyringeDiameter());
        
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


void NE500PumpNetworkDevice::connectToDevice() {
    
    struct sockaddr_in addr;
    struct hostent *hostp;
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN,
            "Connecting to NE500 Pump Network...");
    
    if(NULL == (hostp = gethostbyname(address.c_str()))) {
        throw SimpleException("Error looking up host", address);
        
    }
    
    memset(&addr,0,sizeof(addr));
    memcpy((char *)&addr.sin_addr,hostp->h_addr,hostp->h_length);
    addr.sin_family = hostp->h_addrtype;
    addr.sin_port = htons((u_short)port);
    
    
    if((s=socket(PF_INET, SOCK_STREAM, 6)) < 0) {
        throw SimpleException("Error creating socket.");
    }
    
    
    int flags;
    
    int rc;
    
    /* Set socket to non-blocking */
    
    if ((flags = fcntl(s, F_GETFL, 0)) < 0)
    {
        /* Handle error */
    }
    
    
    if (fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        /* Handle error */
    }
    
    
    while(true){
        rc = connect(s, (struct sockaddr *) &addr, sizeof(addr));
        
        int connect_errno = errno;
        
        if(rc >= 0 || connect_errno == EISCONN){
            break;
        }
        
        if(connect_errno != EALREADY && connect_errno != EINPROGRESS &&
           connect_errno != EWOULDBLOCK){
            
            string err;
            
            
            switch(connect_errno){
                    
                case EBADF:
                    err = "descriptor not valid";
                    break;
                case EIO:
                    err = "io error";
                    break;
                case ECONNREFUSED:
                    err = "connection refused";
                    break;
                case EFAULT:
                    err = "bad address";
                    break;
                case EHOSTUNREACH:
                    err="host unreachable";
                    break;
                case ENAMETOOLONG:
                    err="name too long";
                    break;
                case ENETDOWN:
                    err="network down";
                    break;
                case EINTR:
                    err ="interrupted";
                    break;
                case EINVAL:
                    err = "invalid parameter";
                    break;
                case ETIMEDOUT:
                    err = "timed out";
                    break;
                case ENOTSOCK:
                    err = "not socket";
                    break;
                    
                default:
                    err = "unknown error";
            }
            
            throw SimpleException( (boost::format("Error (%s, %d) connecting to %s") % err % errno % address).str());
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
                close(s);
                connected = false;
                throw SimpleException((boost::format("Connection to NE500 Device (%s) timed out") % address).str());
            }
        }
    }
    
    
    connected = true;
}

void NE500PumpNetworkDevice::disconnectFromDevice() {
    if(connected){
        close(s);
        connected = false;
    }
}

string NE500PumpNetworkDevice::sendMessage(string message){
    
#define PUMP_SERIAL_DELIMITER_CHAR 3 // ETX
    
    std::array<char, 512> recv_buffer;
    int rc;
    bool broken = false;
    
    string result;
    
    message = message + "\r";
    
    if(connected){
        
        // Send converted line back to client.
        if (send(s, message.c_str(), message.size(), 0) < 0){
            cerr << "Error: cannot send data";
        } else {
            mprintf("SENT: %s", message.c_str());
        }
        
        // give it a moment
        shared_ptr<Clock> clock = Clock::instance();
        MWTime tic = clock->getCurrentTimeUS();
        
        while(true){
            // Clear the buffer
            recv_buffer.fill(0);
            
            rc = recv(s,
                      recv_buffer.data(),
                      recv_buffer.size() - 1,  // -1 to ensure there's always a terminating NUL
                      0);
            
            // didn't receive data
            if(rc < 0){
                
                // any response other than EWOULDBLOCK (basically, no data yet)
                // is a sign that an error has occurred.
                if(errno != EWOULDBLOCK){
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
                            default:
                                mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                                         "An unknown response was received from the syringe pump: %c", status_char);
                        }
                    }
                    
                    if(result.size() > 4){
                        
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
                            
                            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                                     "The syringe pump returned an error: %s (%s)", err_str.c_str(), error_code.c_str());
                        }
                    }
                    
                    break;
                    
                } else if((clock->getCurrentTimeUS() - tic) > response_timeout){
                    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                             "Did not receive a complete response from the pump");
                    break;
                }
            }
            
            result += string(recv_buffer.data());
        }
        
        if(broken){
            
            disconnectFromDevice();
            
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                     "Connection lost, reconnecting..."
                     "Command may not have been sent correctly");
            connectToDevice();
            
        }
        
        
        mprintf("RETURNED: %s", result.c_str());
    }
    
    
    return result;
}


void NE500PumpNetworkDevice::dispense(string pump_id, double rate, Datum data) {
    scoped_lock active_lock(active_mutex);
    
    if (active) {
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


void NE500PumpNetworkDevice::initializePump(string pump_id, double rate, double syringe_diameter) {
    
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


void NE500PumpNetworkDevice::NE500DeviceOutputNotification::notify(const Datum &data, MWTime timeUS) {
    if (auto shared_pump_network = pump_network.lock()) {
        if (auto shared_channel = channel.lock()) {
            shared_pump_network->dispense(shared_channel->getPumpID(), shared_channel->getRate(), data);
        }
    }
}


END_NAMESPACE_MW




























