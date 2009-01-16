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
 * Params can also be added, subtracted, multiplied, etc. etc. to yield a new 
 * Variableobject. This new object represents the appropriate operation on the
 * composite Params, and the operation is evaluated any time the Variableis 
 * accessed. From a usage standpoint, these composite values look just like
 * ordinary Params.
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

#include "Component.h"
#include "ComponentFactory.h"
#include "GenericData.h"
#include "LinkedList.h"
#include "VariableNotification.h"
#include "Utilities.h"
#include "Clonable.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <list>
namespace mw {
class BufferManager;

using namespace boost;

enum VariableScope{ M_INVALID_SCOPE = -1, 
					 M_EXPERIMENTWIDE_SCOPE, 
					 M_LOCAL_SCOPE };


enum WhenType{M_NEVER = -1, M_WHEN_IDLE, M_ALWAYS, M_AT_STARTUP, M_EVERY_TRIAL, M_WHEN_CHANGED};

#define M_TRUE_PARAM Variable::trueParam()
#define M_FALSE_PARAM Variable::falseParam()

// number of values in the dictionary that makeup this objects scarab package.
const int __VARIABLE_PACKAGE_SIZE__ = 6;
const int __VALUE_PACKAGE_SIZE__ = 1;


enum Operator {	M_PLUS,
					M_INCREMENT,
					M_MINUS,
					M_DECREMENT,
					M_TIMES,
					M_DIVIDE,
					M_MOD,
					M_IS_EQUAL,
					M_IS_NOT_EQUAL,
					M_IS_GREATER_THAN,
					M_IS_LESS_THAN,
					M_IS_GREATER_THAN_OR_EQUAL,
					M_IS_LESS_THAN_OR_EQUAL,
					M_FLOAT_CAST,
					M_INTEGER_CAST,
					M_BOOLEAN_CAST,
					M_AND,
					M_OR,
					M_NOT,
					M_UNARY_MINUS,
					M_UNARY_PLUS
				};


class VariableProperties; // defined in VariableProperties.h
class ExpressionVariable;


class Variable : public mw::Component, public ComponentFactory {

protected:

	VariableProperties *properties;
	LinkedList<VariableNotification> notifications;

	int codec_code;		// dictionary key value in the parameter registry.

	WhenType logging;  // how often should this variable's value be placed in 
						// the event stream
						
	shared_ptr<BufferManager> buffer_manager;

public:

	// Constructors & Destructors
	Variable(VariableProperties *properties=NULL);
	Variable(const Variable& tocopy);
	virtual ~Variable();

	// Polymorphic copying
	virtual Variable *clone() = 0;
	virtual Variable *frozenClone();


	// Accessors

	virtual void setBufferManager(shared_ptr<BufferManager> _buffer_manager){
		buffer_manager = _buffer_manager;
	}

	virtual VariableProperties *getProperties(){ return properties; }
	virtual void setProperties(VariableProperties *props){ properties=props; }
	
	std::string getVariableName();
	
	int getCodecCode(){ return codec_code; }
	void setCodecCode(int _code){  codec_code = _code; }
	
	WhenType getLogging();
	void setLogging(WhenType when);
	

	// Attaching notifications to variables for asynchronous "spring-loading"
	virtual void addNotification(shared_ptr<VariableNotification> note);
	void lockNotifications(){ notifications.lock(); }
	void unlockNotifications(){ notifications.unlock(); }
	
	shared_ptr<VariableNotification> getFirstNotification(){
		return notifications.getFrontmost();
	}
	
	virtual void performNotifications(Data data);
	virtual void performNotifications(Data data, MonkeyWorksTime timeUS);
	
	// Announcing a variable's value to the event stream
	virtual void announce();
	virtual void announce(MonkeyWorksTime _when);
	
	// Basic value get and set (overridden in subclasses)
	virtual Data getValue() = 0;
	virtual void setValue(Data _data) = 0;
	virtual void setValue(Data _data, MonkeyWorksTime _when) = 0;
	virtual void setSilentValue(Data _value) = 0;
	
	
	// Hopefully to be removed
	//virtual ScarabDatum * toScarabPackage();
	
	// Equals operators
	virtual void operator=(long a);
	virtual void operator=(int a);
	virtual void operator=(short a);
	virtual void operator=(double a);
	virtual void operator=(float a);
	virtual void operator=(bool a);
	virtual void operator=(MonkeyWorksTime a);
	virtual void operator=(std::string a);
	virtual void operator=(Data a);
	
	// Cast operators
	virtual operator long();
	virtual operator int();
	virtual operator short();
	virtual operator double();
	virtual operator float();
	virtual operator bool();
	virtual operator MonkeyWorksTime();
	virtual operator Data();
			
	// Arithmetic operator overloads
	virtual ExpressionVariable operator+(Variable& v);
	virtual ExpressionVariable operator-(Variable& v);
	virtual ExpressionVariable operator*(Variable& v);
	virtual ExpressionVariable operator/(Variable& v);
	virtual ExpressionVariable operator==(Variable& v);
	virtual ExpressionVariable operator!=(Variable& v);
	virtual ExpressionVariable operator%(Variable& v);
	virtual ExpressionVariable operator>(Variable& v);
	virtual ExpressionVariable operator<(Variable& v);
	virtual ExpressionVariable operator>=(Variable& v);
	virtual ExpressionVariable operator<=(Variable& v);
	virtual ExpressionVariable operator&&(Variable& v);
	virtual ExpressionVariable operator||(Variable& v);
	virtual ExpressionVariable operator++();
	virtual ExpressionVariable operator--();
	virtual ExpressionVariable operator!();
	virtual ExpressionVariable operator-();
	virtual ExpressionVariable operator+();	
	
	virtual void addChild(std::map<std::string, std::string> parameters,
						  mwComponentRegistry *reg,
						  shared_ptr<mw::Component> child);
};
		

class VariableFactory : public ComponentFactory {

	// Factory method
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);

};


class EmptyVariable : public Variable {

public:

	EmptyVariable(VariableProperties *p = NULL) : Variable(p){ };
	
	virtual void announce(){ }
	virtual void announce(MonkeyWorksTime t){ }
	
	virtual Data getValue(){  return Data(0L); }
	virtual void setValue(Data v){ }
	virtual void setValue(Data v, MonkeyWorksTime t){ }
	virtual void setSilentValue(Data _value){ return; }
	
	virtual Variable *clone(){ 
		return new EmptyVariable((const EmptyVariable&)(*this));
	}

};
}

#endif


