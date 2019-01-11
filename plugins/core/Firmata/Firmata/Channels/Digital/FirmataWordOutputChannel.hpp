//
//  FirmataWordOutputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 1/8/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef FirmataWordOutputChannel_hpp
#define FirmataWordOutputChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataWordOutputChannel : public FirmataChannel {
    
public:
    static const std::string PIN_NUMBERS;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FirmataWordOutputChannel(const ParameterValueMap &parameters);
    
    bool resolvePinNumbers(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                           std::vector<int> &pinNumbers) override;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) override {
        valueVar->addNotification(notification);
    }
    
    Datum getValueForPin(int pinNumber) override;
    void setValueForPin(int pinNumber, const Datum &value, MWTime time) override;
    
    Type getType() const override { return Type::Digital; }
    Direction getDirection() const override { return Direction::Output; }
    int getPinMode() const override { return PIN_MODE_OUTPUT; }
    
private:
    const std::string requestedPinNumbers;
    const VariablePtr valueVar;
    
    std::map<int, std::uint8_t> bitForPin;
    
};


END_NAMESPACE_MW


#endif /* FirmataWordOutputChannel_hpp */
