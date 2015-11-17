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
    static const std::string VARIABLE;
    static const std::string SYRINGE_DIAMETER;
    static const std::string FLOW_RATE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit NE500DeviceChannel(const ParameterValueMap &parameters);
    
    const VariablePtr& getVariable() const {
        return volume;
    }
    
    bool initialize(const SendFunction &sendMessage);
    bool dispense(const SendFunction &sendMessage);
    
private:
    static std::string formatFloat(double val);
    
    const std::string pump_id;
    const double syringe_diameter;
    const VariablePtr rate;
    const VariablePtr volume;
    
};


END_NAMESPACE_MW


#endif



























