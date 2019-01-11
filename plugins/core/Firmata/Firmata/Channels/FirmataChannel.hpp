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


//
// Pin modes
//

enum {
    PIN_MODE_INPUT  = 0x00, // same as INPUT defined in Arduino.h
    PIN_MODE_OUTPUT = 0x01, // same as OUTPUT defined in Arduino.h
    PIN_MODE_ANALOG = 0x02, // analog pin in analogInput mode
    PIN_MODE_PWM    = 0x03, // digital pin in PWM output mode
    PIN_MODE_SERVO  = 0x04, // digital pin in Servo output mode
    
    PIN_MODE_PULLUP = 0x0B  // enable internal pull-up resistor for pin
};


class FirmataChannel : public Component, protected stx::EmptySymbolTable {
    
public:
    enum class Type { Analog, Digital, Servo };
    enum class Direction { Input, Output };
    
    explicit FirmataChannel(const ParameterValueMap &parameters);
    
    virtual bool resolvePinNumbers(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                   std::vector<int> &pinNumbers) = 0;
    
    virtual void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) = 0;
    virtual Datum getValueForPin(int pinNumber) = 0;
    virtual void setValueForPin(int pinNumber, const Datum &value, MWTime time) = 0;
    
    virtual Type getType() const = 0;
    bool isAnalog() const { return getType() == Type::Analog; }
    bool isDigital() const { return getType() == Type::Digital; }
    
    virtual Direction getDirection() const = 0;
    bool isInput() const { return getDirection() == Direction::Input; }
    bool isOutput() const { return getDirection() == Direction::Output; }
    
    virtual int getPinMode() const = 0;
    
protected:
    static bool resolvePinNumber(const Datum &requestedPinNumber,
                                 const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                 int &pinNumber);
    
    Datum lookupVariable(const std::string &varName) const override;
    Datum processFunction(const std::string &funcName, const paramlist_type &paramList) const override;
    
private:
    static const boost::regex analogChannelIDRegex;
    
};


END_NAMESPACE_MW


#endif /* FirmataChannel_hpp */


























