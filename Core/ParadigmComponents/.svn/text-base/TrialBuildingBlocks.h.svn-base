/**
 * TrialBuildingBlocks.h
 *
 * Description: TrialBuildingBlocks.h is mainly concerned with defining
 * "Actions" and "Transition".  An Action packages up some executable payload
 * into a standardized "execute" method, while an Transition evalutes where
 * (i.e. what State) the state system should move to next.
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 06/02/05 - Fixed spacing, added destructors. Removed 
 *      type() function because it is unused.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef _TRIAL_BUILDING_BLOCKS_H
#define _TRIAL_BUILDING_BLOCKS_H

#include "Experiment.h"

#include "IODevice.h"
#include "Timer.h"
#include "ConstantVariable.h"
#include "SelectionVariable.h"
#include "ExpressionVariable.h"
#include "EyeCalibrators.h"
#include "Sound.h"
#include "ComponentRegistry_new.h"
namespace mw {

using namespace boost;

// base class for all actions
class Action : public State, public VariableNotification {
    protected:
       // State *parent;
		Variable *delay;
		ScheduleTask *taskRef;
	public:
		Action();
        virtual ~Action();
		virtual bool execute();
		
		// TODO: are these needed
        virtual void setOwner(weak_ptr<State> _parent);
        virtual weak_ptr<State> getOwner();
        
		virtual weak_ptr<Experiment> getExperiment();
		void setName(const std::string &_name);
		
		// Fancier features
		void setDelay(Variable *_delay){ delay = _delay; }
		Variable *getDelay(){ return delay; };
		
		virtual void announceEntry();
		virtual void announceExit();
		
		// State methods
		virtual void action();
		virtual weak_ptr<State> next();
};

class ActionVariableNotification : public VariableNotification{
	
protected:
	
	shared_ptr<Action> action;
	
public:
	
	ActionVariableNotification(shared_ptr<Action> _action);
	virtual void notify(const Data&, MonkeyWorksTime time);
	
};

class Assignment : public Action {
protected:
	shared_ptr<Variable> var;
	shared_ptr<Variable> value;
public:
	Assignment(shared_ptr<Variable> var, 
				shared_ptr<Variable> val);
	virtual ~Assignment();
	virtual bool execute();
};

class AssignmentFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
	
};


class ReportString : public Action {
protected:
	std::vector <shared_ptr<Variable> >stringFragments;
	bool error;
public:
	ReportString(const std::string &string_to_report);
	virtual ~ReportString();
	virtual bool execute();
};

class ReportStringFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class AssertionAction : public Action {
protected:
	std::vector <shared_ptr<Variable> >stringFragments;
	shared_ptr<Variable> condition;
	
public:
	AssertionAction(shared_ptr<Variable> _condition, 
					 const std::string &_assertion_message);
	virtual ~AssertionAction();		
	virtual bool execute();
};

class AssertionActionFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class NextVariableSelection : public Action {
	
protected:
	
	shared_ptr<SelectionVariable> variable;
	
public:
	
	NextVariableSelection(shared_ptr<SelectionVariable> _var){
		variable = _var;
		setName("NextVariableSelection");
	}
	
	virtual bool execute(){
		//mprintf("Executing next selection");
		if(variable == NULL){
			merror(M_PARADIGM_MESSAGE_DOMAIN, 
				   "Attempt to update an invalid or nonexistent selection variable.");
			return false;
		}
		variable->nextValue();
		return true;
	}
};

class NextVariableSelectionFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class SetTimeBase : public Action {
	
protected:
	
	shared_ptr<TimeBase> timebase;
	
public:
	
	SetTimeBase(shared_ptr<TimeBase> _timebase);
	virtual bool execute();
};

class SetTimeBaseFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class StartTimer : public Action {
	
protected:
	shared_ptr<Timer> timer;
	shared_ptr<TimeBase> timebase;
	shared_ptr<Variable> time_to_wait_us;
	
public:
	
	StartTimer(shared_ptr<Timer> timer, shared_ptr<Variable> time_to_wait);
	StartTimer(shared_ptr<Timer> timer, shared_ptr<TimeBase> timebase, 
				shared_ptr<Variable> time_to_wait);
	virtual bool execute();
};

class StartTimerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

//class StartEggTimer : public Action {
//protected:
//	shared_ptr<Variable> time_to_wait_us;
//public:
//	StartEggTimer(shared_ptr<Variable> _time_to_wait);
//	virtual ~StartEggTimer();
//	virtual bool execute();
//	shared_ptr<Variable> getTimeToWait();
//};

class Wait : public Action {
protected:
	shared_ptr<Variable> waitTime;
	shared_ptr<TimeBase> timeBase;
public:
	Wait(shared_ptr<Variable> time_us);
	Wait(shared_ptr<TimeBase> timeBase,
		  shared_ptr<Variable> time_us);
	virtual ~Wait();
	virtual bool execute();
	shared_ptr<Variable> getTimeToWait();
};

class WaitFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class QueueStimulus : public Action {
protected:
	shared_ptr<StimulusNode> stimnode;
	shared_ptr<StimulusDisplay> display;
    
    // TODO: might be nice to add an optional set of "forcing"
    // parameters, e.g. to force the object to queued at a particular location
public:
	QueueStimulus(shared_ptr<StimulusNode> _stimnode, 
				   shared_ptr<StimulusDisplay> _display);
	virtual ~QueueStimulus();
	virtual bool execute();
};

class QueueStimulusFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class LiveQueueStimulus : public Action {
protected:
	shared_ptr<StimulusNode> stimnode;
	shared_ptr<StimulusDisplay> display;
public:
	LiveQueueStimulus(shared_ptr<StimulusNode> _stimnode, 
					   shared_ptr<StimulusDisplay> _display);
	virtual ~LiveQueueStimulus();
	virtual bool execute();
};

class LiveQueueStimulusFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class DequeueStimulus : public Action {
protected:
	shared_ptr<StimulusNode> stimnode;
public:
	DequeueStimulus(shared_ptr<StimulusNode> _stimnode);
	virtual ~DequeueStimulus();
	virtual bool execute();
};

class DequeueStimulusFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class BringStimulusToFront : public Action {
protected:
	shared_ptr<StimulusNode> list_node;
public:
	BringStimulusToFront(shared_ptr<StimulusNode> _stimnode);
	virtual ~BringStimulusToFront();
	virtual bool execute();
	shared_ptr<StimulusNode> getStimulusNode();
};

class BringStimulusToFrontFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class SendStimulusToBack : public Action {
protected:
	shared_ptr<StimulusNode> list_node;
public:
	SendStimulusToBack(shared_ptr<StimulusNode> _stimnode);
	virtual ~SendStimulusToBack();
	virtual bool execute();
	shared_ptr<StimulusNode> getStimulusNode();
};

class SendStimulusToBackFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class UpdateStimulusDisplay : public Action {
protected:
	weak_ptr<Experiment> experiment; 
public: 
	UpdateStimulusDisplay();
	virtual ~UpdateStimulusDisplay();
	virtual bool execute();
};

class UpdateStimulusDisplayFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class PlaySound : public Action {
	
protected:
	shared_ptr<Sound> sound;
	
public:
	
	PlaySound(shared_ptr<Sound> _sound){
		sound = _sound;
		setName("PlaySound");
	}
	
	virtual ~PlaySound(){}
	
	virtual bool execute(){
		if(sound == NULL){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Attempt to play a NULL sound");
			return false;
		}
		sound->play();
		return true;
	}
};

class PlaySoundFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class StopSound : public Action {
	
protected:
	shared_ptr<Sound> sound;
	
public:
	
	StopSound(shared_ptr<Sound> _sound){
		sound = _sound;
		setName("StopSound");
	}
	
	virtual ~StopSound(){}
	
	virtual bool execute(){
		if(sound == NULL){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Attempt to play a NULL sound");
			return false;
		}
		sound->stop();
		return true;
	}
};

class StopSoundFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class PauseSound : public Action {
	
protected:
	shared_ptr<Sound> sound;
	
public:
	
	PauseSound(shared_ptr<Sound> _sound){
		sound = _sound;
		setName("PauseSound");
	}
	
	virtual ~PauseSound(){}
	
	virtual bool execute(){
		if(sound == NULL){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Attempt to play a NULL sound");
			return false;
		}
		sound->pause();
		return true;
	}
};

class PauseSoundFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class StartDeviceIO : public Action {
protected:
	weak_ptr<IODevice> device;
public:
	StartDeviceIO(shared_ptr<IODevice> _device);
	virtual ~StartDeviceIO();
	virtual bool execute();
};

class StartDeviceIOFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class StopDeviceIO : public Action {
protected:
	weak_ptr<IODevice> device;
public:
	StopDeviceIO(shared_ptr<IODevice> _device);
	virtual ~StopDeviceIO();
	virtual bool execute();
};

class StopDeviceIOFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class ResetSelection : public Action {
	
protected:
	shared_ptr<Selectable> selectable;
	
public:
	
	ResetSelection(shared_ptr<Selectable> sel){
		selectable = sel;
	}
	
	
	virtual bool execute(){
		if(selectable == shared_ptr<Selectable>()){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Attempt to reset a NULL selectable object.");
			return false;
			setName("ResetSelections");
		}
		selectable->resetSelections();
		return true;
	}
};

class ResetSelectionFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class AcceptSelections : public Action {
	
protected:
	shared_ptr<Selectable> selectable;
	
public:
	
	AcceptSelections(shared_ptr<Selectable> sel){
		setName("AcceptSelections");
		selectable = sel;
	}
	
	
	virtual bool execute(){
		if(selectable == shared_ptr<Selectable>()){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Attempt to accept selections on a NULL selectable object.");
			return false;
		}
		selectable->acceptSelections();
		return true;
	}
};

class AcceptSelectionsFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class RejectSelections : public Action {
	
protected:
	shared_ptr<Selectable> selectable;
	
public:
	
	RejectSelections(shared_ptr<Selectable> sel){
		setName("RejectSelections");
		selectable = sel;
	}
	
	virtual bool execute(){
		if(selectable == shared_ptr<Selectable>()){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Attempt to reject selections on a NULL selectable object.");
			return false;
		}
		selectable->rejectSelections();
		return true;
	}
};

class RejectSelectionsFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class If : public Action {
protected:
	shared_ptr<Variable> condition;
	ExpandableList<Action> actionlist;
	ExpandableList<Action> elselist;	
public:
	If(shared_ptr<Variable> v1);
	virtual ~If();
	void addAction(shared_ptr<Action> act);
	void addChild(std::map<std::string, std::string> parameters,
					mwComponentRegistry *reg,
					shared_ptr<mw::Component> child);
	virtual bool execute();
};

class IfFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class Transition : public mw::Component { };

class TransitionFactory : public ComponentFactory {
public:
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class TransitionCondition : public Transition {
protected:
	shared_ptr<Variable> condition;
	weak_ptr<State> transition;
	bool always_go;
	
	weak_ptr<State> owner;
	std::string name;
	
public:
	// if v1
	TransitionCondition(shared_ptr<Variable> v1, 
						 weak_ptr<State> trans);
	// always go
	TransitionCondition(weak_ptr<State> trans);
	virtual ~TransitionCondition();
	virtual weak_ptr<State> execute();
	void setOwner(weak_ptr<State> _owner);
	weak_ptr<State> getOwner();
	weak_ptr<State> getTransition();
};

class TransitionIfTimerExpired : public TransitionCondition {
protected:
	shared_ptr<Timer> timer;
public:
	TransitionIfTimerExpired(shared_ptr<Timer> _timer, 
							  weak_ptr<State> trans);
	virtual weak_ptr<State> execute();
};

//class TransitionIfEggTimerExpired : public TransitionCondition {
//public:
//	TransitionIfEggTimerExpired(shared_ptr<State> trans);
//	virtual State *execute();
//};

class YieldToParent : public TransitionCondition {
public:
	YieldToParent();
	virtual ~YieldToParent();
	virtual weak_ptr<State> execute();
};

class TaskSystemState : public State {
protected:
	ExpandableList<Action> *action_list;
	ExpandableList<TransitionCondition> *transition_list;
	bool done;
public:
	TaskSystemState();
	TaskSystemState(State *parent);
	virtual ~TaskSystemState();	
	
	virtual shared_ptr<mw::Component> createInstanceObject();
	virtual void action();
	virtual weak_ptr<State> next();
	
	virtual void addChild(std::map<std::string, std::string> parameters,
						  mwComponentRegistry *reg, shared_ptr<mw::Component> comp);
	virtual void addAction(shared_ptr<Action> act);
	virtual void addTransition(shared_ptr<TransitionCondition> trans);
	
	
	ExpandableList<Action> * getActionList();
	ExpandableList<TransitionCondition> * getTransitionList();
	void setActionList(ExpandableList<Action> *al){  action_list = al; }
	void setTransitionList(ExpandableList<TransitionCondition> *tl){  
		transition_list = tl;
	}  
};

class TaskSystemStateFactory : public ComponentFactory{	
	shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
											mwComponentRegistry *reg){
		
		shared_ptr<State> component(new TaskSystemState());
		component->setParameters(parameters, reg);
		return component;
	}
};



// A container for building blocks
class TaskSystem : public ContainerState {
protected:
	bool execution_triggered;
	
public:
	// execute what's in the box, leaving the transition 
	// list open to be user defined
	TaskSystem();
	TaskSystem(State *parent);	
	
	virtual shared_ptr<mw::Component> createInstanceObject();
	virtual ~TaskSystem();	
	virtual void updateHierarchy();
	virtual void action();
	virtual weak_ptr<State> next();
	
	void addTaskSystemState(shared_ptr<TaskSystemState> state);
	
	virtual void addChild(std::map<std::string, std::string> parameters,
								mwComponentRegistry *reg,
								shared_ptr<mw::Component> comp){
		ContainerState::addChild(parameters, reg, comp);
		string full_tag = parameters["parent_tag"];
		full_tag += "/";
		full_tag += parameters["child_tag"];
		
		// register an alternate name (in task_system/task_system_state style)
		reg->registerObject(full_tag, comp);
	
	}
	
	
	virtual weak_ptr<State> getStartState();
	//mExpandableList<State> * getTaskSystemStates();
	
	//void setTaskSystemStates(ExpandableList<State> *states){ list = states; }
};

class TaskSystemFactory : public ComponentFactory{	
	shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
											mwComponentRegistry *reg){
		
		shared_ptr<State> component(new TaskSystem());
		component->setParameters(parameters, reg);
		return component;
	}
};



// action to cause the calibrator object to take a calibration value immediately
// the calibrator class has methods to get values for registered varaibles and 
//  call goldStandard objects to get true values
class TakeCalibrationSampleNow : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
	shared_ptr<GoldStandard> goldStandard;     // object that contains the "true" values
public:
	TakeCalibrationSampleNow(shared_ptr<Calibrator> _calibrator, 
							  shared_ptr<GoldStandard> _goldStandard);
	virtual ~TakeCalibrationSampleNow();
	virtual bool execute();
};

class TakeCalibrationSampleNowFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class StartAverageCalibrationSample : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
public:
	StartAverageCalibrationSample(shared_ptr<Calibrator> _calibrator);
	virtual ~StartAverageCalibrationSample();
	virtual bool execute();
};

class StartAverageCalibrationSampleFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class EndAverageAndTakeCalibrationSample : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
	shared_ptr<GoldStandard> goldStandard;     // object that contains the "true" values
public:
	EndAverageAndTakeCalibrationSample(shared_ptr<Calibrator> _calibrator, 
										shared_ptr<GoldStandard> _goldStandard);
	virtual ~EndAverageAndTakeCalibrationSample();
	virtual bool execute();
};

class EndAverageAndTakeCalibrationSampleFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class EndAverageAndIgnore : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
public:
	EndAverageAndIgnore(shared_ptr<Calibrator> _calibrator);
	virtual ~EndAverageAndIgnore();
	virtual bool execute();
};

class EndAverageAndIgnoreFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


// action to compute the calibration parameters and immediately begin using them
class CalibrateNow : public Action {
protected:
	shared_ptr<Calibrator> calibrator;
public:
	CalibrateNow(shared_ptr<Calibrator> _calibrator);
	virtual ~CalibrateNow();
	virtual bool execute();     // false if calibration could not be completed
};

class CalibrateNowFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

// action to compute the calibration parameters and immediately begin using them
class ClearCalibration : public Action {
protected:
	shared_ptr<Calibrator> calibrator;
public:
	ClearCalibration(shared_ptr<Calibrator> _calibrator);
	virtual ~ClearCalibration();
	virtual bool execute();
};

class ClearCalibrationFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};



// ==============================================
// averager controler actions -- Added May 7, 2006 JJD
// ==============================================
class StartAverager : public Action {
protected:
	shared_ptr<AveragerUser> averager;
public:
	StartAverager(shared_ptr<AveragerUser> _averager);
	virtual ~StartAverager();
	virtual bool execute();
};

class StartAveragerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class StopAverager : public Action {
protected:
	shared_ptr<AveragerUser> averager;
public:
	StopAverager(shared_ptr<AveragerUser> _averager);
	virtual ~StopAverager();
	virtual bool execute();
};

class StopAveragerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};

class ClearAverager : public Action {
protected:
	shared_ptr<AveragerUser> averager;
public:
	ClearAverager(shared_ptr<AveragerUser> _averager);
	virtual ~ClearAverager();
	virtual bool execute();
};

class ClearAveragerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}

#endif


