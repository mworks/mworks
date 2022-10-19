/**
 * GenericVariable.h
 *
 * Copyright (c) 2002-2016 MIT. All rights reserved.
 */

#ifndef GENERIC_VARIABLE_H
#define GENERIC_VARIABLE_H

#include "MWorksMacros.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "GenericData.h"
#include "LinkedList.h"
#include "VariableProperties.h"
#include "VariableNotification.h"
#include "Utilities.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <list>


BEGIN_NAMESPACE_MW


class Variable : public Component, boost::noncopyable {
    
public:
    explicit Variable(const VariableProperties &properties = VariableProperties());
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  ComponentPtr child) override;
    
    const VariableProperties & getProperties() const { return properties; }
    const std::string & getVariableName() const { return properties.getTagName(); }
    
    int getCodecCode() const { return codec_code; }
    void setCodecCode(int code) { codec_code = code; }
    
    WhenType getLogging() const { return logging; }
    void setLogging(WhenType when) { logging = when; }
    
    void setEventTarget(const boost::shared_ptr<EventReceiver> &target) { event_target = target; }
    
    // Attaching notifications to variables for asynchronous "spring-loading"
    void addNotification(const boost::shared_ptr<VariableNotification> &note);
    
    // Announcing a variable's value to the event stream
    void announce(MWTime when = getCurrentTimeUS());
    
    // Basic value get and set (overridden in subclasses)
    virtual Datum getValue() = 0;
    virtual void setValue(Datum value, MWTime when = getCurrentTimeUS());
    virtual void setValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime when = getCurrentTimeUS());
    virtual void setSilentValue(Datum value, MWTime when = getCurrentTimeUS()) = 0;
    virtual void setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime when = getCurrentTimeUS()) = 0;
    
    // Can the value be modified?
    virtual bool isWritable() const = 0;
    
    // Equals operators
    void operator=(long a) { setValue(a); }
    void operator=(int a) { setValue(a); }
    void operator=(short a) { setValue(int(a)); }
    void operator=(double a) { setValue(a); }
    void operator=(float a) { setValue(a); }
    void operator=(bool a) { setValue(a); }
    void operator=(long long a) { setValue(a); }
    void operator=(const std::string &a) { setValue(a); }
    void operator=(Datum a) { setValue(a); }
    
    // Cast operators
    operator long() { return getValue().getInteger(); }
    operator int() { return getValue().getInteger(); }
    operator short() { return getValue().getInteger(); }
    operator double() { return getValue().getFloat(); }
    operator float() { return getValue().getFloat(); }
    operator bool() { return getValue().getBool(); }
    operator long long() { return getValue().getInteger(); }
    operator Datum() { return getValue(); }
    
protected:
    void performNotifications(Datum data, MWTime timeUS = getCurrentTimeUS());
    
private:
    static MWTime getCurrentTimeUS();
    
    VariableProperties properties;
    int codec_code;
    WhenType logging;
    boost::shared_ptr<EventReceiver> event_target;
    
    LinkedList<VariableNotification> notifications;
    
};


using VariablePtr = boost::shared_ptr<Variable>;


class VariableFactory : public ComponentFactory {

	// Factory method
    shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                           ComponentRegistry *reg) override;

};


class ReadOnlyVariable : public Variable {
    
public:
    using Variable::Variable;
    
    void setValue(Datum v, MWTime t) override { }
    void setValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime when) override { }
    void setSilentValue(Datum _value, MWTime _when) override { }
    void setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime when) override { }
    
    bool isWritable() const override { return false; }
    
};


class ConstantVariable : public ReadOnlyVariable {
    
public:
    explicit ConstantVariable(const Datum &value) : value(value) { }
    
    Datum getValue() override { return value; }
    
private:
    const Datum value;
    
};


class EmptyVariable : public ConstantVariable {
    
public:
    EmptyVariable() : ConstantVariable(Datum(0)) { }
    
};


END_NAMESPACE_MW


#endif



























