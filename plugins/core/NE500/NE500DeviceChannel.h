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


END_NAMESPACE_MW


#endif






