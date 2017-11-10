//
//  FirmataChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/6/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataChannel_hpp
#define FirmataChannel_hpp


BEGIN_NAMESPACE_MW


class FirmataChannel : public Component {
    
public:
    enum class Type { Analog = 3, Digital = 1, Servo = 4 };
    enum class Direction { Input = 1, Output = 0 };
    
    static const std::string PIN_NUMBER;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FirmataChannel(const ParameterValueMap &parameters);
    
    bool resolvePinNumber(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel);
    int getPinNumber() const { return pinNumber; }
    int getAnalogChannelNumber() const { return analogChannelNumber; }
    
    const VariablePtr& getValueVariable() const { return value; }
    
    virtual Type getType() const = 0;
    bool isAnalog() const { return getType() == Type::Analog; }
    bool isDigital() const { return getType() == Type::Digital; }
    
    virtual Direction getDirection() const = 0;
    bool isInput() const { return getDirection() == Direction::Input; }
    bool isOutput() const { return getDirection() == Direction::Output; }
    
    int getPinMode() const { return (int(getType()) - int(getDirection())); }
    
private:
    static constexpr int minPinNumber = 0;
    static constexpr int maxPinNumber = 127;
    
    static constexpr int minAnalogChannelNumber = 0;
    static constexpr int maxAnalogChannelNumber = 15;
    
    const std::string requestedPinNumber;
    const VariablePtr value;
    int pinNumber;
    int analogChannelNumber;
    
};


END_NAMESPACE_MW


#endif /* FirmataChannel_hpp */


























