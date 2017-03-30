//
//  FirmataDigitalChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalChannel_hpp
#define FirmataDigitalChannel_hpp


BEGIN_NAMESPACE_MW


class FirmataDigitalChannel : public Component {
    
public:
    enum class Direction { Input = 0, Output = 1 };
    
    static const std::string PIN_NUMBER;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FirmataDigitalChannel(const ParameterValueMap &parameters);
    
    int getPinNumber() const { return pinNumber; }
    const VariablePtr& getValueVariable() const { return value; }
    
    virtual Direction getDirection() const = 0;
    bool isInput() const { return getDirection() == Direction::Input; }
    bool isOutput() const { return getDirection() == Direction::Output; }
    
private:
    const int pinNumber;
    const VariablePtr value;
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalChannel_hpp */



























