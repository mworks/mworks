/*
 *  Trigger.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 6/24/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#ifndef TRIGGER_H_
#define TRIGGER_H_

#include "GenericVariable.h"
#include "Lockable.h"
#include "Utilities.h"
#include "ExpandableList.h"
namespace mw {
class Trigger {

protected:
	
	bool active;
	bool tripped;

public:

	Trigger();
    virtual ~Trigger();
	virtual bool evaluate();
	
	bool isActive(){ return active; }
	virtual void setActive(bool _active);
};


class AsynchronousTriggerNotification; // forward declaration

class AsynchronousTrigger : public Trigger, public Lockable {

protected:

	shared_ptr<Variable> trigger_variable;
	
	ExpandableList<AsynchronousTriggerNotification> *notifications;

public:
	
	AsynchronousTrigger(shared_ptr<Variable> _trigger_var);
	virtual ~AsynchronousTrigger();
	virtual bool evaluate();
	virtual void addVariable(shared_ptr<Variable> var);
    virtual void setActive(bool _active);
    
};


class AsynchronousTriggerNotification : public VariableNotification {

protected:
	
	AsynchronousTrigger *target;

public:
	
	AsynchronousTriggerNotification(AsynchronousTrigger *_target);
	virtual ~AsynchronousTriggerNotification(){};
	
	virtual void notify(const Data& data){
        //mprintf("notifying asych trigger of a variable change ...");
		target->evaluate();
	}
    
    // TODO -- Dave, I had to add this method to make the trigger actaully be called when the
    // variable logged a change.  I am not sure how to avoid such an error.
    virtual void notify(const Data& data, MonkeyWorksTime timeUS){
        //mprintf("notifying asych trigger of a variable change ver 2...");
		target->evaluate();
	}

};


class GreaterThanThresholdTrigger : public AsynchronousTrigger {

protected:

	shared_ptr<Variable> threshold;
	shared_ptr<Variable> watchvar;

public:

	GreaterThanThresholdTrigger(shared_ptr<Variable> _threshold, shared_ptr<Variable> _watchvar,
								shared_ptr<Variable> _trigger);
								
	virtual bool evaluate(){
		if(!active) return false;
	
		bool result = false;
		
		Data watchval = *watchvar;
		Data threshval = *threshold;
	
		if(watchval.getDataType() == M_FLOAT || threshval.getDataType() == M_FLOAT){
			result = (double)watchval > (double)threshval;
		} else {
			result = (long)watchval > (long)threshval;
		}
		
		if(result){
			(*trigger_variable) = true;
		}
		
		return result;
	}
};


class LessThanThresholdTrigger : public AsynchronousTrigger {

protected:

	shared_ptr<Variable> threshold;
	shared_ptr<Variable> watchvar;

public:

	LessThanThresholdTrigger(shared_ptr<Variable> _threshold, shared_ptr<Variable> _watchvar,
														   shared_ptr<Variable> _trigger);
	virtual bool evaluate(){
		
		if(!active) return false;
		
		bool result = false;
		
		Data watchval = *watchvar;
		Data threshval = *threshold;
	
		if(watchval.getDataType() == M_FLOAT || 
			threshval.getDataType() == M_FLOAT){
			result = (double)watchval < (double)threshval;
		} else {
			result = (long)watchval < (long)threshval;
		}
		
		if(!tripped && result){
			(*trigger_variable) = true;
			tripped = true;
		}

		if(tripped && !result){
			(*trigger_variable) = false;
			tripped = false;
		}

		
		return result;
	}
};


class SquareRegionTrigger : public AsynchronousTrigger {

protected:
	shared_ptr<Variable> centerx;
	shared_ptr<Variable> centery;
	shared_ptr<Variable> width;
	shared_ptr<Variable> watchx;
	shared_ptr<Variable> watchy;
	
public:

	SquareRegionTrigger(shared_ptr<Variable> _centerx, shared_ptr<Variable> _centery, 
				shared_ptr<Variable> _width, shared_ptr<Variable> _watchx, shared_ptr<Variable> _watchy,
														shared_ptr<Variable> _trigger);
	
	virtual bool evaluate();
};

class RectangularRegionTrigger : public AsynchronousTrigger {

	shared_ptr<Variable> lowerx; 
	shared_ptr<Variable> lowery;
	shared_ptr<Variable> upperx;
	shared_ptr<Variable> uppery;

	shared_ptr<Variable> watchx;
	shared_ptr<Variable> watchy;
	
public:

	RectangularRegionTrigger(shared_ptr<Variable> _lowerx, shared_ptr<Variable> _lowery, 
				shared_ptr<Variable> _upperx, shared_ptr<Variable> _uppery, 
				shared_ptr<Variable> _watchx, shared_ptr<Variable> _watchy, shared_ptr<Variable> _trigger);
	
	virtual bool evaluate();
};


class CircularRegionTrigger : public AsynchronousTrigger {

	shared_ptr<Variable> centerx;
	shared_ptr<Variable> centery;
	shared_ptr<Variable> width;

	shared_ptr<Variable> watchx;
	shared_ptr<Variable> watchy;
	
public:

	CircularRegionTrigger(shared_ptr<Variable> _centerx, shared_ptr<Variable> _centery, 
				shared_ptr<Variable> _width, shared_ptr<Variable> _watchx, shared_ptr<Variable> _watchy, 
														  shared_ptr<Variable> _trigger);
	
	virtual bool evaluate();
};

}
#endif

