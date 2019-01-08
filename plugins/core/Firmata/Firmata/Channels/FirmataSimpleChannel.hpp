//
//  FirmataSimpleChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 1/7/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef FirmataSimpleChannel_hpp
#define FirmataSimpleChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataSimpleChannel : public FirmataChannel {
    
public:
    static const std::string PIN_NUMBER;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FirmataSimpleChannel(const ParameterValueMap &parameters);
    
    bool resolvePinNumbers(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                           std::vector<int> &pinNumbers) override;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) override {
        valueVar->addNotification(notification);
    }
    
    Datum getValueForPin(int pinNumber) override {
        return valueVar->getValue();
    }
    
    void setValueForPin(int pinNumber, const Datum &value, MWTime time) override {
        valueVar->setValue(value, time);
    }
    
private:
    const std::string requestedPinNumber;
    const VariablePtr valueVar;
    
};


END_NAMESPACE_MW


#endif /* FirmataSimpleChannel_hpp */
