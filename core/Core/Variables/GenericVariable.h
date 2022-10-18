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


class Variable : public mw::Component, boost::noncopyable {

private:
	VariableProperties *properties;
	LinkedList<VariableNotification> notifications;

	int codec_code;		// dictionary key value in the parameter registry.

	WhenType logging;  // how often should this variable's value be placed in 
						// the event stream
						
	shared_ptr<EventReceiver> event_target;
    
    static MWTime getCurrentTimeUS();
    
protected:
    void performNotifications(Datum data, MWTime timeUS = getCurrentTimeUS());

public:
    // Destructor
    ~Variable();
    
	// Constructors
	explicit Variable(VariableProperties *properties = nullptr);

    
	// Accessors

    void setEventTarget(const shared_ptr<EventReceiver> &_event_target) {
		event_target = _event_target;
	}

    const VariableProperties * getProperties() const { return properties; }
	
	std::string getVariableName() const;
	
	int getCodecCode() const { return codec_code; }
	void setCodecCode(int _code) { codec_code = _code; }
	
    WhenType getLogging() const { return logging; }
    void setLogging(WhenType when) { logging = when; }
	

	// Attaching notifications to variables for asynchronous "spring-loading"
    void addNotification(const shared_ptr<VariableNotification> &note);
	
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
	void operator=(long a);
	void operator=(int a);
	void operator=(short a);
	void operator=(double a);
	void operator=(float a);
	void operator=(bool a);
	void operator=(MWTime a);
	void operator=(std::string a);
	void operator=(Datum a);
	
	// Cast operators
	operator long();
	operator int();
	operator short();
	operator double();
	operator float();
	operator bool();
	operator MWTime();
	operator Datum();
	
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<mw::Component> child) override;
    
};


typedef boost::shared_ptr<Variable> VariablePtr;


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



























