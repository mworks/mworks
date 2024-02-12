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
#include "Clock.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <list>


BEGIN_NAMESPACE_MW


class ParameterValue;


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
    void announce(MWTime when = getCurrentTimeUS()) { announce(getValue(), when); }
    
    // Basic value get and set (overridden in subclasses)
    virtual Datum getValue() = 0;
    virtual void setValue(const Datum &value, MWTime when = getCurrentTimeUS(), bool silent = false) = 0;
    virtual void setValue(const std::vector<Datum> &indexOrKeyPath,
                          const Datum &value,
                          MWTime when = getCurrentTimeUS(),
                          bool silent = false) = 0;
    
    // Convenience value setters
    void setSilentValue(const Datum &value, MWTime when = getCurrentTimeUS()) {
        setValue(value, when, true);
    }
    void setSilentValue(const std::vector<Datum> &indexOrKeyPath,
                        const Datum &value,
                        MWTime when = getCurrentTimeUS())
    {
        setValue(indexOrKeyPath, value, when, true);
    }
    
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
    static MWTime getCurrentTimeUS();
    
    void performNotifications(const Datum &value, MWTime when, bool silent);
    
private:
    void announce(const Datum &value, MWTime when) const;
    
    VariableProperties properties;
    int codec_code;
    WhenType logging;
    boost::shared_ptr<EventReceiver> event_target;
    
    LinkedList<VariableNotification> notifications;
    
};


inline MWTime Variable::getCurrentTimeUS() {
    auto clock = Clock::instance(false);
    if (clock) {
        return clock->getCurrentTimeUS();
    }
    return 0;
}


using VariablePtr = boost::shared_ptr<Variable>;


class ReadWriteVariable : public Variable {
    
public:
    using Variable::Variable;
    
    bool isWritable() const override { return true; }
    
protected:
    // To prevent deadlock when a notification tries to read the variable's value,
    // use a shared_lock in getValue, and an upgrade_lock in setValue (with
    // upgrade_to_unique_lock protecting the actual value update)
    using Mutex = boost::upgrade_mutex;
    Mutex valueMutex;
    using shared_lock = boost::shared_lock<Mutex>;
    using upgrade_lock = boost::upgrade_lock<Mutex>;
    using upgrade_to_unique_lock = boost::upgrade_to_unique_lock<Mutex>;
    
};


class ReadOnlyVariable : public Variable {
    
public:
    using Variable::Variable;
    
    void setValue(const Datum &value, MWTime when, bool silent) override { }
    void setValue(const std::vector<Datum> &indexOrKeyPath, const Datum &value, MWTime when, bool silent) override { }
    
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


class BaseVariableFactory : public ComponentFactory {
    
public:
    static const std::string GROUPS;
    static const std::string DESCRIPTION;
    
    static ComponentInfo describeComponent();
    
    explicit BaseVariableFactory(const ComponentInfo &info) : ComponentFactory(info) { }
    
    ComponentPtr createObject(StdStringMap parameters, ComponentRegistry *reg) override;
    
protected:
    static std::string getGroups(const ParameterValue &paramValue);
    static std::string getDescription(const ParameterValue &paramValue);
    
private:
    virtual ComponentPtr createVariable(const Map<ParameterValue> &parameters) const = 0;
    
};


class VariableFactory : public BaseVariableFactory {
    
public:
    static const std::string DEFAULT_VALUE;
    static const std::string TYPE;
    static const std::string SCOPE;
    static const std::string LOGGING;
    static const std::string PERSISTENT;
    static const std::string EXCLUDE_FROM_DATA_FILE;
    
    static ComponentInfo describeComponent();
    
    VariableFactory() : BaseVariableFactory(describeComponent()) { }
    
private:
    enum class Scope { Global, Local };
    
    static GenericDataType getType(const ParameterValue &paramValue);
    static Datum getDefaultValue(const ParameterValue &paramValue, GenericDataType type);
    static WhenType getLogging(const ParameterValue &paramValue);
    static Scope getScope(const ParameterValue &paramValue);
    
    ComponentPtr createVariable(const Map<ParameterValue> &parameters) const override;
    
};


END_NAMESPACE_MW


#endif
