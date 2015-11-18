/*
 *  NE500DeviceChannel.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_NE500DEVICECHANNEL_H_
#define _NE500DEVICECHANNEL_H_


BEGIN_NAMESPACE_MW


class NE500DeviceChannel : public Component {
    
public:
    using SendFunction = std::function<bool(const std::string &, std::string)>;
    
    static const std::string CAPABILITY;
    static const std::string SYRINGE_DIAMETER;
    static const std::string FLOW_RATE;
    static const std::string RATE_UNIT;
    static const std::string VARIABLE;
    static const std::string VOLUME_UNIT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit NE500DeviceChannel(const ParameterValueMap &parameters);
    
    const VariablePtr& getVariable() const {
        return volume;
    }
    
    bool initialize(const SendFunction &sendMessage);
    bool dispense(const SendFunction &sendMessage);
    
private:
    enum class Direction {
        Infuse,
        Withdraw
    };
    
    static std::string formatFloat(double val);
    
    double getCurrentRate() const {
        return rate->getValue().getFloat();
    }
    
    double getCurrentVolume() const {
        return volume->getValue().getFloat();
    }
    
    Direction getCurrentDirection(double currentVolume) const {
        return (currentVolume >= 0.0 ? Direction::Infuse : Direction::Withdraw);
    }
    
    bool setRate(const SendFunction &sendMessage, double currentRate);
    bool setVolume(const SendFunction &sendMessage, double currentVolume);
    bool setDirection(const SendFunction &sendMessage, Direction currentDirection);
    
    const std::string pump_id;
    const double syringe_diameter;
    const VariablePtr rate;
    const std::string rateUnit;
    const VariablePtr volume;
    const std::string volumeUnit;
    
    double previousRate;
    double previousAbsVolume;
    Direction previousDirection;
    
};


END_NAMESPACE_MW


#endif



























