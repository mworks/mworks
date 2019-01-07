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
    
    explicit FirmataChannel(const ParameterValueMap &parameters);
    
    virtual bool resolvePinNumbers(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                   std::set<int> &pinNumbers) = 0;
    
    virtual void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) = 0;
    virtual Datum getValueForPin(int pinNumber) = 0;
    virtual void setValueForPin(int pinNumber, const Datum &value, MWTime time) = 0;
    
    virtual Type getType() const = 0;
    bool isAnalog() const { return getType() == Type::Analog; }
    bool isDigital() const { return getType() == Type::Digital; }
    
    virtual Direction getDirection() const = 0;
    bool isInput() const { return getDirection() == Direction::Input; }
    bool isOutput() const { return getDirection() == Direction::Output; }
    
    int getPinMode() const { return (int(getType()) - int(getDirection())); }
    
protected:
    static constexpr int minPinNumber = 0;
    static constexpr int maxPinNumber = 127;
    
    static constexpr int minAnalogChannelNumber = 0;
    static constexpr int maxAnalogChannelNumber = 15;
    
    static bool resolvePinNumber(const std::string &requestedPinNumber,
                                 const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                 int &pinNumber);
    
};


END_NAMESPACE_MW


#endif /* FirmataChannel_hpp */


























