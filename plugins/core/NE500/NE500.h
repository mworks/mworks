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

using std::string;
using std::vector;


BEGIN_NAMESPACE_MW


class NE500DeviceChannel : public Component {
    
private:
    const std::string pump_id;
    const boost::shared_ptr<Variable> variable;
    
    const double syringe_diameter;
    const boost::shared_ptr<Variable> rate;
    
public:
    static const std::string CAPABILITY;
    static const std::string VARIABLE;
    static const std::string SYRINGE_DIAMETER;
    static const std::string FLOW_RATE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit NE500DeviceChannel(const ParameterValueMap &parameters);
    
    const boost::shared_ptr<Variable>& getVariable() const {
        return variable;
    }
    
    const std::string& getPumpID() const {
        return pump_id;
    }
    
    double getSyringeDiameter() const {
        return syringe_diameter;
    }
    
    double getRate() const {
        return rate->getValue().getFloat();
    }
    
};


class NE500PumpNetworkDevice : public IODevice, boost::noncopyable {
    
private:
    const std::string address;
    const int port;
    
    // the socket
    int s;
    
    bool connected;
    
    vector<string> pump_ids;
    
    bool active;
    boost::mutex active_mutex;
    
public:
    static const std::string ADDRESS;
    static const std::string PORT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit NE500PumpNetworkDevice(const ParameterValueMap &parameters);
    
    ~NE500PumpNetworkDevice(){
        disconnectFromDevice();
    }
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<Component> _child) override;
    
    bool initialize() override {  return connected;  }
    bool startDeviceIO() override {  setActive(true); return true; }
    bool stopDeviceIO() override {  setActive(false); return true; }
    
private:
    void connectToDevice();
    void disconnectFromDevice();
    
    string sendMessage(string message);
    void dispense(string pump_id, double rate, Datum data);
    void initializePump(string pump_id, double rate, double syringe_diameter);
    
    void setActive(bool _active){
        boost::mutex::scoped_lock active_lock(active_mutex);
        active = _active;
    }
    
    bool getActive(){
        boost::mutex::scoped_lock active_lock(active_mutex);
        bool is_active = active;
        return is_active;
    }
    
    
    class NE500DeviceOutputNotification : public VariableNotification {
    private:
        const boost::weak_ptr<NE500PumpNetworkDevice> pump_network;
        const boost::weak_ptr<NE500DeviceChannel> channel;
        
    public:
        NE500DeviceOutputNotification(const boost::shared_ptr<NE500PumpNetworkDevice> &_pump_network,
                                      const boost::shared_ptr<NE500DeviceChannel> &_channel) :
            pump_network(_pump_network),
            channel(_channel)
        { }
        
        void notify(const Datum& data, MWorksTime timeUS) override {
            if (auto shared_pump_network = pump_network.lock()) {
                if (auto shared_channel = channel.lock()) {
                    shared_pump_network->dispense(shared_channel->getPumpID(), shared_channel->getRate(), data);
                }
            }
        }
    };
};


END_NAMESPACE_MW


#endif






