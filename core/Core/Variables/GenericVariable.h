/**
 * GenericVariable.h
 *
 * Desscription:
 * An Variableobject is a unique variable specifier.  The Variable contains no 
 * actual data, rather it is analogous to the name of a variable (like a C 
 * name token), which has an associated value which depends on context (i.e. 
 * int i declared globally means one thing, and declared locally in a 
 * function means something else. 'i' is same token used in either event).  
 * We pass Variablepointers to Experiment->getX(Variable *) type functions, and
 * get back appropriate values.
 *
 * The reason that this is important here is that we'd like to have paradigm
 * variables organized in a stack-like way, with locally defined values 
 * overriding global ones.  For instance, we might want FIXATION_POINT_LOCATION
 * to mean 0.0 by default at a global level, but might want to override this 
 * value for individual trials where we want the fixation point to move.  
 * Ideally the routine that actually draws the fixation point shouldn't know
 * anything about where we are in the paradigm hierarchy, and the fixation
 * point variable should revert back to its original value when we exit that 
 * particular trial and return to a higher (block) level in the paradigm.  
 *
 * Variable can also be added, subtracted, multiplied, etc. etc. to yield a new 
 * Variable object, and the operation is evaluated any time the Variable is 
 * accessed. From a usage standpoint, these composite values look just like
 * ordinary Variables.
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 01/27/05 - Fixed spacing and added codecCode member.
 * Paul Jankunas on 02/14/05 - Added deepCopy function to create a deep 
 *                              copy of an Param.
 * Paul Jankunas on 4/6/05 - Added Variablemembers to the scarab datum made 
 *                      when you send codec info over the network.  Moved
 *                      getScarabDatum function to be a virtual function in
 *                      base param class.
 * Paul Jankunas on 06/02/05 - Added destructors.
 * Paul Jankunas on 06/05/05 - Fixed copy constructor to take a reference
 *      instead of a pointer.
 * Dave Cox on 06/09/05  Renamed to Variable from Param.  Added 
 *      ParameterNotification class and  list to Variable.  Removed 
 *      BaseParam class, CompositeParam class and overloaded operaters of
 *      Param class.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef GENERIC_VARIABLE_H
#define GENERIC_VARIABLE_H

#include "MWorksMacros.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "GenericData.h"
#include "LinkedList.h"
#include "VariableNotification.h"
#include "Utilities.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <list>


BEGIN_NAMESPACE_MW


enum WhenType{M_NEVER = -1, M_WHEN_IDLE, M_ALWAYS, M_AT_STARTUP, M_EVERY_TRIAL, M_WHEN_CHANGED};


// Forward declarations
class EventBuffer;
class VariableProperties;


class Variable : public mw::Component, boost::noncopyable {

private:

	VariableProperties *properties;
	LinkedList<VariableNotification> notifications;

	int codec_code;		// dictionary key value in the parameter registry.

	WhenType logging;  // how often should this variable's value be placed in 
						// the event stream
						
	shared_ptr<EventReceiver> event_target;
    
    static MWTime getCurrentTimeUS();

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
	
    void performNotifications(Datum data, MWTime timeUS = getCurrentTimeUS());
	
	// Announcing a variable's value to the event stream
    void announce(MWTime when = getCurrentTimeUS());
	
	// Basic value get and set (overridden in subclasses)
	virtual Datum getValue() = 0;
	virtual void setValue(Datum value, MWTime when = getCurrentTimeUS());
    virtual void setSilentValue(Datum value, MWTime when = getCurrentTimeUS()) = 0;
	
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
    void setSilentValue(Datum _value, MWTime _when) override { }
    
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



























