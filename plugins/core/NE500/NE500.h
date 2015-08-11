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
    
    const std::string& getPumpID() const {
        return pump_id;
    }
    
    const boost::shared_ptr<Variable>& getVariable() const {
        return variable;
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
    const MWTime response_timeout;
    
    // the socket
    int s;
    
    bool connected;
    
    std::vector<boost::shared_ptr<NE500DeviceChannel>> pumps;
    
    bool active;
    boost::mutex active_mutex;
    using scoped_lock = boost::mutex::scoped_lock;
    
public:
    static const std::string ADDRESS;
    static const std::string PORT;
    static const std::string RESPONSE_TIMEOUT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit NE500PumpNetworkDevice(const ParameterValueMap &parameters);
    ~NE500PumpNetworkDevice();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<Component> _child) override;
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    static constexpr char PUMP_SERIAL_DELIMITER_CHAR = 3;  // ETX
    
    static std::string formatFloat(double val);
    
    bool connectToDevice();
    void disconnectFromDevice();
    
    bool sendMessage(const boost::format &fmt) {
        return sendMessage(fmt.str());
    }
    
    bool sendMessage(string message);
    bool dispense(string pump_id, double rate, Datum data);
    bool initializePump(string pump_id, double rate, double syringe_diameter);
    
    
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
        
        void notify(const Datum &data, MWTime timeUS) override;
    };
};


END_NAMESPACE_MW


#endif






