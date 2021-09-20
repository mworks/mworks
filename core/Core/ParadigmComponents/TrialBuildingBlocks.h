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
#include "SelectionVariable.h"
#include "ExpressionVariable.h"
#include "EyeCalibrators.h"
#include "Sound.h"
#include "ComponentRegistry.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"
#include "VariableAssignment.hpp"


BEGIN_NAMESPACE_MW


// base class for all actions
class Action : public State {

public:
    static void describeComponent(ComponentInfo &info);

    explicit Action(const ParameterValueMap &parameters);
    Action();
    virtual bool execute();
    
    void setName(const std::string &_name);
    
    virtual void announceEntry();
    
    // State methods
    virtual void action();

};

class ActionVariableNotification : public VariableNotification{
	
protected:
	
	shared_ptr<Action> action;
	
public:
	
	ActionVariableNotification(shared_ptr<Action> _action);
	virtual void notify(const Datum&, MWTime time);
	
};


class Assignment : public Action {
    
public:
    static const std::string VARIABLE;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Assignment(const ParameterValueMap &parameters);
    Assignment(const VariablePtr &var, const VariablePtr &value);
    
    bool execute() override;
    
private:
    const VariableAssignment assignment;
    const VariablePtr value;
    
};


class ReportString : public Action {
    
public:
    static const std::string MESSAGE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ReportString(const Map<ParameterValue> &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr message;
    const boost::shared_ptr<Clock> clock;
    
};


class AssertionAction : public Action {
    
public:
    static const std::string CONDITION;
    static const std::string STOP_ON_FAILURE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AssertionAction(const Map<ParameterValue> &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr condition;
    VariablePtr message;
    const bool stopOnFailure;
    
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
};


class TimerAction : public Action {
    
public:
    static const std::string DURATION;
    static const std::string DURATION_UNITS;
    static const std::string TIMEBASE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TimerAction(const ParameterValueMap &parameters);
    
protected:
    MWTime getExpirationTime() const;
    
private:
    VariablePtr duration;
    MWTime durationUnitsToUS;
    boost::shared_ptr<TimeBase> timebase;
    
};


class StartTimer : public TimerAction {
    
public:
    static const std::string TIMER;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StartTimer(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    boost::shared_ptr<Timer> timer;
    
};


class Wait : public TimerAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit Wait(const ParameterValueMap &parameters);
    
    bool execute() override;
    weak_ptr<State> next() override;
    
private:
    MWTime expirationTime;
    
};


class WaitForCondition : public Action {
    
public:
    static const std::string CONDITION;
    static const std::string TIMEOUT;
    static const std::string TIMEOUT_MESSAGE;
    static const std::string STOP_ON_TIMEOUT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit WaitForCondition(const ParameterValueMap &parameters);
    
    bool execute() override;
    boost::weak_ptr<State> next() override;
    
private:
    bool stillWaiting() const;
    
    const VariablePtr condition;
    const VariablePtr timeout;
    VariablePtr timeoutMessage;
    const bool stopOnTimeout;
    
    const boost::shared_ptr<Clock> clock;
    MWTime deadline;
    
};

    
class LoadStimulus : public Action {
protected:
    shared_ptr<StimulusNode> stimnode;
    shared_ptr<StimulusDisplay> display;
    
public:
    LoadStimulus(shared_ptr<StimulusNode> _stimnode, 
                  shared_ptr<StimulusDisplay> _display);
    virtual ~LoadStimulus();
    virtual bool execute();
};
    
class LoadStimulusFactory : public ComponentFactory{
    virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   ComponentRegistry *reg);
};    

class UnloadStimulus : public Action {
protected:
    shared_ptr<StimulusNode> stimnode;
    shared_ptr<StimulusDisplay> display;
        
public:
    UnloadStimulus(shared_ptr<StimulusNode> _stimnode, 
                   shared_ptr<StimulusDisplay> _display);
    virtual ~UnloadStimulus();
    virtual bool execute();
};
    
class UnloadStimulusFactory : public ComponentFactory{
    virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   ComponentRegistry *reg);
};    
    
class QueueStimulus : public Action {
protected:
	shared_ptr<StimulusNode> stimnode;
	shared_ptr<StimulusDisplay> display;
    
public:
	QueueStimulus(shared_ptr<StimulusNode> _stimnode, 
				   shared_ptr<StimulusDisplay> _display);
	virtual ~QueueStimulus();
	virtual bool execute();
};

class QueueStimulusFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
};


class StimulusDisplayAction : public Action {
    
public:
    static const std::string PREDICTED_OUTPUT_TIME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StimulusDisplayAction(const ParameterValueMap &parameters);
    
    bool execute() override;
    boost::weak_ptr<State> next() override;
    
protected:
    bool stillWaiting() const;
    
    virtual boost::shared_ptr<StimulusDisplay::UpdateInfo> performAction() = 0;
    virtual const char * getActionName() const = 0;
    
    const VariablePtr predictedOutputTime;
    const boost::shared_ptr<StimulusDisplay> display;
    const boost::shared_ptr<Clock> clock;
    
    MWTime startTime;
    boost::shared_ptr<StimulusDisplay::UpdateInfo> updateInfo;
    
};


class UpdateStimulusDisplay : public StimulusDisplayAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit UpdateStimulusDisplay(const ParameterValueMap &parameters);
    
private:
    boost::shared_ptr<StimulusDisplay::UpdateInfo> performAction() override;
    const char * getActionName() const override;
    
};


class ClearStimulusDisplay : public StimulusDisplayAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit ClearStimulusDisplay(const ParameterValueMap &parameters);
    
private:
    boost::shared_ptr<StimulusDisplay::UpdateInfo> performAction() override;
    const char * getActionName() const override;
    
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
};


class ResetSelection : public Action {
	
protected:
	boost::weak_ptr<Selectable> weakSelectable;
	
public:
	
	ResetSelection(shared_ptr<Selectable> sel){
        setName("ResetSelections");
		weakSelectable = sel;
	}
	
	
	virtual bool execute(){
		if (auto selectable = weakSelectable.lock()) {
            selectable->resetSelections();
		}
		return true;
	}
};

class ResetSelectionFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class AcceptSelections : public Action {
	
protected:
    boost::weak_ptr<Selectable> weakSelectable;
	
public:
	
	AcceptSelections(shared_ptr<Selectable> sel){
		setName("AcceptSelections");
		weakSelectable = sel;
	}
	
	
	virtual bool execute(){
		if (auto selectable = weakSelectable.lock()) {
            selectable->acceptSelections();
		}
		return true;
	}
};

class AcceptSelectionsFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class RejectSelections : public Action {
	
protected:
	boost::weak_ptr<Selectable> weakSelectable;
	
public:
	
	RejectSelections(shared_ptr<Selectable> sel){
		setName("RejectSelections");
		weakSelectable = sel;
	}
	
	virtual bool execute(){
		if (auto selectable = weakSelectable.lock()) {
            selectable->rejectSelections();
		}
		return true;
	}
};

class RejectSelectionsFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class If : public Action {
    
public:
    static const std::string CONDITION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit If(const ParameterValueMap &parameters);
    explicit If(const VariablePtr &condition);
    
    void addAction(const boost::shared_ptr<Action> &action);
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    bool execute() override;
    
private:
    const VariablePtr condition;
    std::vector<boost::shared_ptr<Action>> actions;
    
};


class Else : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit Else(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    bool execute() override;
    
private:
    std::vector<boost::shared_ptr<Action>> actions;
    
};


class IfElse : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit IfElse(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    bool execute() override;
    
private:
    std::vector<boost::shared_ptr<If>> conditionals;
    boost::shared_ptr<Else> unconditional;
    
};


class While : public Action {
    
public:
    static const std::string CONDITION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit While(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    bool execute() override;
    boost::weak_ptr<State> next() override;
    
private:
    bool performIteration();
    
    const VariablePtr condition;
    std::vector< boost::shared_ptr<Action> > actions;
    bool shouldRepeat;
    
};


class Transition : public mw::Component { };

class TransitionFactory : public ComponentFactory {
public:
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
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

class TaskSystemState : public ContainerState {
    
private:
    shared_ptr< vector< shared_ptr<TransitionCondition> > > transition_list { new vector< shared_ptr<TransitionCondition> > };
    int currentActionIndex { 0 };
    
	void addTransition(shared_ptr<TransitionCondition> trans);
    
public:
    static void describeComponent(ComponentInfo &info);
    
	TaskSystemState();
    explicit TaskSystemState(const ParameterValueMap &parameters);
	
    shared_ptr<mw::Component> createInstanceObject() override;
    weak_ptr<State> next() override;
    void reset() override;
	
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<mw::Component> comp) override;
    
};


class TaskSystem : public ContainerState {
	
public:
    static void describeComponent(ComponentInfo &info);
    
	TaskSystem();
    explicit TaskSystem(const ParameterValueMap &parameters);
	
    shared_ptr<mw::Component> createInstanceObject() override;
	void action() override;
    weak_ptr<State> next() override;
	
	void addTaskSystemState(shared_ptr<TaskSystemState> state);
	
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<mw::Component> comp) override;
    
};


class StopExperiment : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit StopExperiment(const ParameterValueMap &parameters);
    virtual ~StopExperiment() { }
    
    virtual bool execute();
    
};


class PauseExperiment : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit PauseExperiment(const ParameterValueMap &parameters);
    virtual ~PauseExperiment() { }
    
    virtual bool execute();
    
};


class ResumeExperiment : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit ResumeExperiment(const ParameterValueMap &parameters);
    virtual ~ResumeExperiment() { }
    
    virtual bool execute();
    
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
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
												ComponentRegistry *reg);
};


END_NAMESPACE_MW


#endif





















