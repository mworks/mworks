/**
 * TrialBuildingBlocks.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 06/02/05 - Fixed spacing, added destructors. Removed 
 *      type() function because it is unused.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "TrialBuildingBlocks.h"
#include "StimulusDisplay.h"
#include "ExpressionVariable.h"

#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>
#include <sstream>


BEGIN_NAMESPACE_MW


#define VERBOSE_ACTION_METHODS  0


/****************************************************************
 *                       Action Methods
 ****************************************************************/
void Action::describeComponent(ComponentInfo &info) {
    State::describeComponent(info);
}

Action::Action(const ParameterValueMap &parameters) :
    State(parameters)
{
    setParent(GlobalCurrentExperiment);   // a bit of kludge for now
}

Action::Action() {
    setParent(GlobalCurrentExperiment);   // a bit of kludge for now
	setName("Action");
}	

bool Action::execute() {
    return false;
}

void Action::action(){ 
	announceEntry();
	execute(); 
}

void Action::announceEntry() {
	currentState->setValue(getCompactID());
}


void Action::setName(const std::string &_name) {
	State::setName("Action: " + _name);
}


ActionVariableNotification::ActionVariableNotification(shared_ptr<Action> _action){
	action = _action;
}

void ActionVariableNotification::notify(const Datum& data, MWTime time){
	action->execute();
}


/****************************************************************
 *                       Assignment Methods
 ****************************************************************/


const std::string Assignment::VARIABLE("variable");
const std::string Assignment::VALUE("value");


void Assignment::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/assignment");
    info.addParameter(VARIABLE);
    info.addParameter(VALUE);
}


Assignment::Assignment(const ParameterValueMap &parameters) :
    Action(parameters),
    assignment(parameters[VARIABLE]),
    value(parameters[VALUE])
{
    setName("Assignment");
}


Assignment::Assignment(const VariablePtr &var, const VariablePtr &value) :
    assignment(var),
    value(value)
{
    setName("Assignment");
}


bool Assignment::execute() {
    assignment.assign(value->getValue());
    return true;
}


/****************************************************************
 *                       NextVariableSelection Methods
 ****************************************************************/
shared_ptr<mw::Component> NextVariableSelectionFactory::createObject(std::map<std::string, std::string> parameters,
																   ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "selection");
	
	shared_ptr<Variable> variable_uncast = reg->getVariable(parameters.find("selection")->second);
	if(variable_uncast == NULL){
		throw MissingAttributeException(parameters["reference_id"], "Selection attribute not found");
	}
	
	shared_ptr<SelectionVariable> variable = boost::dynamic_pointer_cast<SelectionVariable, Variable>(variable_uncast);
	
	if(variable == NULL){
		throw SimpleException((boost::format("Invalid object for attribute \"%s\" (should be a selection variable, but isn't)") % parameters["selection"]).str());
	}
	
	shared_ptr <mw::Component> newNextVariableSelection = shared_ptr<mw::Component>(new NextVariableSelection(variable));
	return newNextVariableSelection;		
}	


/****************************************************************
 *                       ReportString Methods
 ****************************************************************/


const std::string ReportString::MESSAGE("message");


void ReportString::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/report");
    info.addParameter(MESSAGE);
}


ReportString::ReportString(const ParameterValueMap &parameters) :
    Action(parameters),
    message(parsedText(parameters[MESSAGE])),
    clock(Clock::instance())
{
    setName("Report");
}


bool ReportString::execute() {
    mprintf("%s (%lld)", message->getValue().getString().c_str(), clock->getCurrentTimeUS());
    return true;
}


/****************************************************************
 *                       AssertionAction Methods
 ****************************************************************/


const std::string AssertionAction::CONDITION("condition");
const std::string AssertionAction::STOP_ON_FAILURE("stop_on_failure");


void AssertionAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    
    info.setSignature("action/assert");
    
    info.addParameter(CONDITION);
    info.addParameter(ReportString::MESSAGE, false);
    info.addParameter(STOP_ON_FAILURE, "NO");
}


AssertionAction::AssertionAction(const ParameterValueMap &parameters) :
    Action(parameters),
    condition(parameters[CONDITION]),
    stopOnFailure(parameters[STOP_ON_FAILURE])
{
    setName("Assertion");
    
    if (!(parameters[ReportString::MESSAGE].empty())) {
        message = parsedText(parameters[ReportString::MESSAGE]);
    } else {
        message = boost::make_shared<ConstantVariable>(Datum("Condition failed: " + parameters[CONDITION].str()));
    }
}


bool AssertionAction::execute() {
    if (!(condition->getValue().getBool())) {
        auto outStr = message->getValue().getString();
        assertionFailure->setValue(outStr);
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Assertion: %s", outStr.c_str());
        if (stopOnFailure) {
            merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Stopping experiment due to failed assertion");
            StateSystem::instance()->stop();
        }
        assertionFailure->setSilentValue(0L);
    }
    
    return true;
}


/****************************************************************
 *                       SetTimeBase Methods
 ****************************************************************/

SetTimeBase::SetTimeBase(shared_ptr<TimeBase> _timebase){
	setName("SetTimeBase");
	timebase = _timebase;
}

bool SetTimeBase::execute(){
	//mprintf("Setting timebase");
	timebase->setNow();
	return true;
}

shared_ptr<mw::Component> SetTimeBaseFactory::createObject(std::map<std::string, std::string> parameters,
														 ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "timebase");
	
	shared_ptr<TimeBase> timeBase = reg->getObject<TimeBase>(parameters.find("timebase")->second);
	
	if(timeBase == 0) {
		timeBase = shared_ptr<TimeBase>(new TimeBase());
		reg->registerObject(parameters.find("timebase")->second, timeBase);
	}
	
	shared_ptr <mw::Component> newSetTimeBaseAction = shared_ptr<mw::Component>(new SetTimeBase(timeBase));
	return newSetTimeBaseAction;		
}


/****************************************************************
 *                       TimerAction Methods
 ****************************************************************/


const std::string TimerAction::DURATION("duration");
const std::string TimerAction::DURATION_UNITS("duration_units");
const std::string TimerAction::TIMEBASE("timebase");


void TimerAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    
    info.addParameter(DURATION);
    info.addParameter(DURATION_UNITS, "us");
    info.addParameter(TIMEBASE, false);
}


TimerAction::TimerAction(const ParameterValueMap &parameters) :
    Action(parameters),
    duration(parameters[DURATION])
{
    auto &units = parameters[DURATION_UNITS].str();
    if (units == "us") {
        durationUnitsToUS = 1;
    } else if (units == "ms") {
        durationUnitsToUS = 1000;
    } else if (units == "s") {
        durationUnitsToUS = 1000000;
    } else {
        throw InvalidAttributeException(DURATION_UNITS, units);
    }
    
    auto &tb = parameters[TIMEBASE];
    if (!tb.empty()) {
        timebase = tb.getRegistry()->getObject<TimeBase>(tb.str());
        if (!timebase) {
            throw MissingReferenceException(TIMEBASE, tb.str());
        }
    }
}


MWTime TimerAction::getExpirationTime() const {
    MWTime offset;
    
    if (timebase) {
        offset = timebase->getTime();
    } else {
        offset = Clock::instance()->getCurrentTimeUS();
    }
    
    return MWTime(duration->getValue() * Datum(durationUnitsToUS)) + offset;
}


/****************************************************************
 *                       StartTimer Methods
 ****************************************************************/


const std::string StartTimer::TIMER("timer");


void StartTimer::describeComponent(ComponentInfo &info) {
    TimerAction::describeComponent(info);
    
    info.setSignature("action/start_timer");
    
    info.addParameter(TIMER);
}


StartTimer::StartTimer(const ParameterValueMap &parameters) :
    TimerAction(parameters)
{
    setName("StartTimer");
    
    auto &t = parameters[TIMER];
    timer = t.getRegistry()->getObject<Timer>(t.str());
    if (!timer) {
        // Create a new timer
        VariableProperties props(Datum(0),
                                 t.str(),
                                 M_WHEN_CHANGED,
                                 false);
        timer = global_variable_registry->createTimer(props);
        t.getRegistry()->registerObject(t.str(), timer);
    }
}


bool StartTimer::execute() {
    MWTime duration = getExpirationTime() - Clock::instance()->getCurrentTimeUS();
    timer->startUS(duration);
    return true;
}


/****************************************************************
 *                       Wait Methods
 ****************************************************************/


void Wait::describeComponent(ComponentInfo &info) {
    TimerAction::describeComponent(info);
    info.setSignature("action/wait");
}


Wait::Wait(const ParameterValueMap &parameters) :
    TimerAction(parameters),
    expirationTime(0)
{
    setName("Wait");
}


bool Wait::execute() {
    expirationTime = getExpirationTime();
    
    if (StateSystem::instance()->getCurrentState().lock().get() != this) {
        // If we're executing outside of the normal state system (e.g. as part of a ScheduledActions instance),
        // then we need to do the wait ourselves, right here
        auto clock = Clock::instance();
        MWTime timeToWait = expirationTime - clock->getCurrentTimeUS();
        if (timeToWait > 0) {
            clock->sleepUS(timeToWait);
        }
    }
    
    return true;
}


weak_ptr<State> Wait::next() {
    if (Clock::instance()->getCurrentTimeUS() >= expirationTime) {
        return Action::next();
    }
    return weak_ptr<State>();
}


/****************************************************************
 *                 WaitForCondition Methods
 ****************************************************************/


const std::string WaitForCondition::CONDITION("condition");
const std::string WaitForCondition::TIMEOUT("timeout");
const std::string WaitForCondition::TIMEOUT_MESSAGE("timeout_message");
const std::string WaitForCondition::STOP_ON_TIMEOUT("stop_on_timeout");


void WaitForCondition::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    
    info.setSignature("action/wait_for_condition");
    
    info.addParameter(CONDITION);
    info.addParameter(TIMEOUT);
    info.addParameter(TIMEOUT_MESSAGE, false);
    info.addParameter(STOP_ON_TIMEOUT, "YES");
}


WaitForCondition::WaitForCondition(const ParameterValueMap &parameters) :
    Action(parameters),
    condition(parameters[CONDITION]),
    timeout(parameters[TIMEOUT]),
    stopOnTimeout(parameters[STOP_ON_TIMEOUT]),
    clock(Clock::instance())
{
    setName("WaitForCondition");
    
    if (!(parameters[TIMEOUT_MESSAGE].empty())) {
        timeoutMessage = parsedText(parameters[TIMEOUT_MESSAGE]);
    } else {
        timeoutMessage = boost::make_shared<ConstantVariable>(Datum("Timeout while waiting for condition: " + parameters[CONDITION].str()));
    }
}


bool WaitForCondition::execute() {
    deadline = clock->getCurrentTimeUS() + MWTime(*timeout);
    
    if (StateSystem::instance()->getCurrentState().lock().get() != this) {
        // If we're executing outside of the normal state system (e.g. as part of a ScheduledActions instance),
        // then we need to do the wait ourselves, right here
        while (stillWaiting()) {
            clock->yield();
        }
    }
    
    return true;
}


weak_ptr<State> WaitForCondition::next() {
    if (stillWaiting()) {
        return weak_ptr<State>();
    }
    return Action::next();
}


bool WaitForCondition::stillWaiting() const {
    if (clock->getCurrentTimeUS() >= deadline) {
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "%s", timeoutMessage->getValue().getString().c_str());
        if (stopOnTimeout) {
            merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Stopping experiment due to wait for condition timeout");
            StateSystem::instance()->stop();
        }
        return false;
    }
    
    if (condition->getValue().getBool()) {
        return false;
    }
    
    return true;
}


/****************************************************************
 *                       LoadStimulus Methods
 ****************************************************************/
LoadStimulus::LoadStimulus(shared_ptr<StimulusNode> _stimnode,
                             shared_ptr<StimulusDisplay> _display) : 
Action() {
	stimnode = _stimnode;
	display = _display;
	setName("LoadStimulus");
}

LoadStimulus::~LoadStimulus() { }

bool LoadStimulus::execute() {
    //
    // Stimulus loading can fail with an exception due to legitimate runtime errors
    // (e.g. a bad image file path).  Since we don't want such an exception to halt
    // the state system (or crash the server by not getting caught on another thread),
    // catch all exceptions and convert them to error messages.
    //
    try {
        stimnode->load(display);
    } catch (const std::exception &e) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Stimulus loading failed: %s", e.what());
    } catch (...) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Stimulus loading failed");
    }
    
    return true;
}
    
shared_ptr<mw::Component> LoadStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                             ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	if(GlobalCurrentExperiment == 0) {
		throw SimpleException("GlobalCurrentExperiment is not defined");
	}
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	shared_ptr<StimulusDisplay> stimDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	if(stimDisplay == 0) {
		throw SimpleException("GlobalCurrentExperiment->getStimulusDisplay() is not defined");
	}
	
	shared_ptr <mw::Component> newLoadStimulusAction = shared_ptr<mw::Component>(new LoadStimulus(stimulus, stimDisplay));
	return newLoadStimulusAction;	
}


/****************************************************************
 *                       UnloadStimulus Methods
 ****************************************************************/
UnloadStimulus::UnloadStimulus(shared_ptr<StimulusNode> _stimnode,
                               shared_ptr<StimulusDisplay> _display) : 
Action() {
	stimnode = _stimnode;
	display = _display;
	setName("UnloadStimulus");
}

UnloadStimulus::~UnloadStimulus() { }

bool UnloadStimulus::execute() {	
    stimnode->unload(display);
    return true;
}

shared_ptr<mw::Component> UnloadStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                              ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	if(GlobalCurrentExperiment == 0) {
		throw SimpleException("GlobalCurrentExperiment is not defined");
	}
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	shared_ptr<StimulusDisplay> stimDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	if(stimDisplay == 0) {
		throw SimpleException("GlobalCurrentExperiment->getStimulusDisplay() is not defined");
	}
	
	shared_ptr <mw::Component> newUnloadStimulusAction = shared_ptr<mw::Component>(new UnloadStimulus(stimulus, stimDisplay));
	return newUnloadStimulusAction;	
}



/****************************************************************
 *                       QueueStimulus Methods
 ****************************************************************/
QueueStimulus::QueueStimulus(shared_ptr<StimulusNode> _stimnode,
						       shared_ptr<StimulusDisplay> _display) : 
Action() {
	stimnode = _stimnode;
	display = _display;
	setName("QueueStimulus");
}

QueueStimulus::~QueueStimulus() { }

bool QueueStimulus::execute() {
    if (!(stimnode->isLoaded()) &&
        stimnode->getDeferred() == Stimulus::deferred_load)
    {
        stimnode->load(display);
    }
    
    // freeze the stimulus
    stimnode->freeze();
    
    stimnode->setPendingVisible(true);
    stimnode->clearPendingRemoval();
    
    stimnode->addToDisplay(display);
    
    return true;
}	

shared_ptr<mw::Component> QueueStimulusFactory::createObject(std::map<std::string, std::string> parameters,
														   ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	if(GlobalCurrentExperiment == 0) {
		throw SimpleException("GlobalCurrentExperiment is not defined");
	}
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	shared_ptr<StimulusDisplay> stimDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	if(stimDisplay == 0) {
		throw SimpleException("GlobalCurrentExperiment->getStimulusDisplay() is not defined");
	}
	
	shared_ptr <mw::Component> newQueueStimulusAction = shared_ptr<mw::Component>(new QueueStimulus(stimulus, stimDisplay));
	return newQueueStimulusAction;	
}

/****************************************************************
 *                       LiveQueueStimulus Methods
 ****************************************************************/
LiveQueueStimulus::LiveQueueStimulus(
									   shared_ptr<StimulusNode>  _stimnode,
									   shared_ptr<StimulusDisplay> _display) : 
Action() {
	stimnode = _stimnode;
	display = _display;
	setName("LiveQueueStimulus");
}

LiveQueueStimulus::~LiveQueueStimulus() {
	
}

bool LiveQueueStimulus::execute() {
    if (!(stimnode->isLoaded()) &&
        stimnode->getDeferred() == Stimulus::deferred_load)
    {
        stimnode->load(display);
    }
    
    // don't freeze the stimulus
    stimnode->thaw();
    
    stimnode->setPendingVisible(true);
    stimnode->clearPendingRemoval();
    
    stimnode->addToDisplay(display);
    
    return true;
}	

shared_ptr<mw::Component> LiveQueueStimulusFactory::createObject(std::map<std::string, std::string> parameters,
															   ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	if(GlobalCurrentExperiment == 0) {
		throw SimpleException("GlobalCurrentExperiment is not defined");
	}
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	shared_ptr<StimulusDisplay> stimDisplay = GlobalCurrentExperiment->getStimulusDisplay();
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	if(stimDisplay == 0) {
		throw SimpleException("GlobalCurrentExperiment->getStimulusDisplay() is not defined");
	}
	
	shared_ptr <mw::Component> newLiveQueueStimulusAction = shared_ptr<mw::Component>(new LiveQueueStimulus(stimulus, stimDisplay));
	return newLiveQueueStimulusAction;	
}

/****************************************************************
 *                       DequeueStimulus Methods
 ****************************************************************/
DequeueStimulus::DequeueStimulus(shared_ptr<StimulusNode> _stimnode) : 
Action() {
	stimnode = _stimnode;
	setName("DequeueStimulus");
}

DequeueStimulus::~DequeueStimulus() { }

bool DequeueStimulus::execute() {
    
    stimnode->setPendingVisible(false);
		
    // set a flag that this node should be removed on the 
    // next "explicit" update
	stimnode->setPendingRemoval();
    
	return true;
}

shared_ptr<mw::Component> DequeueStimulusFactory::createObject(std::map<std::string, std::string> parameters,
															 ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	shared_ptr <mw::Component> newDequeueStimulusAction = shared_ptr<mw::Component>(new DequeueStimulus(stimulus));
	return newDequeueStimulusAction;	
}

/****************************************************************
 *                       BringStimulusToFront Methods
 ****************************************************************/
BringStimulusToFront::BringStimulusToFront(
											 shared_ptr<StimulusNode> _lnode) : 
Action() {
	list_node = _lnode;
	setName("BringStimulusToFront");
}

BringStimulusToFront::~BringStimulusToFront() { }

bool BringStimulusToFront::execute() {
    list_node->bringToFront();
	return true;
}

shared_ptr<StimulusNode> BringStimulusToFront::getStimulusNode() {
    return list_node;
}

shared_ptr<mw::Component> BringStimulusToFrontFactory::createObject(std::map<std::string, std::string> parameters,
																  ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	shared_ptr <mw::Component> newBringStimulusToFrontAction = shared_ptr<mw::Component>(new BringStimulusToFront(stimulus));
	return newBringStimulusToFrontAction;	
}

/****************************************************************
 *                       SendStimulusToBack Methods
 ****************************************************************/
SendStimulusToBack::SendStimulusToBack(
										 shared_ptr<StimulusNode> _lnode) :
Action() {
	list_node = _lnode;
	setName("SendStimulusToBack");
}

SendStimulusToBack::~SendStimulusToBack() { }

bool SendStimulusToBack::execute() {
    list_node->sendToBack();
	return true;
}

shared_ptr<StimulusNode> SendStimulusToBack::getStimulusNode() {
    return list_node;
}

shared_ptr<mw::Component> SendStimulusToBackFactory::createObject(std::map<std::string, std::string> parameters,
																ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "stimulus");
	
	shared_ptr<StimulusNode> stimulus = reg->getStimulus(parameters.find("stimulus")->second);
	
	checkAttribute(stimulus, parameters["reference_id"], "stimulus", parameters.find("stimulus")->second);		
	
	
	shared_ptr <mw::Component> newSendStimulusToBackAction = shared_ptr<mw::Component>(new SendStimulusToBack(stimulus));
	return newSendStimulusToBackAction;	
}


/****************************************************************
 *                 StimulusDisplayAction Methods
 ****************************************************************/


const std::string StimulusDisplayAction::PREDICTED_OUTPUT_TIME("predicted_output_time");


void StimulusDisplayAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(PREDICTED_OUTPUT_TIME, false);
}


StimulusDisplayAction::StimulusDisplayAction(const ParameterValueMap &parameters) :
    Action(parameters),
    predictedOutputTime(optionalVariable(parameters[PREDICTED_OUTPUT_TIME])),
    display(StimulusDisplay::getCurrentStimulusDisplay()),
    clock(Clock::instance())
{ }


bool StimulusDisplayAction::execute() {
    startTime = clock->getCurrentTimeUS();
    updateInfo = performAction();
    
    if (StateSystem::instance()->getCurrentState().lock().get() != this) {
        // If we're executing outside of the normal state system (e.g. as part of a ScheduledActions instance),
        // then we need to do the wait ourselves, right here
        while (stillWaiting()) {
            clock->yield();
        }
    }
    
    return true;
}


weak_ptr<State> StimulusDisplayAction::next() {
    if (stillWaiting()) {
        return weak_ptr<State>();
    }
    return Action::next();
}


bool StimulusDisplayAction::stillWaiting() const {
    if (updateInfo->isPending()) {
        return true;
    }
    
    if (predictedOutputTime) {
        predictedOutputTime->setValue(updateInfo->getPredictedOutputTime());
    }
    
    auto stopTime = clock->getCurrentTimeUS();
    auto elapsedMS = double(stopTime - startTime) / 1000.0;
    auto refreshPeriodMS = 1000.0 / display->getMainDisplayRefreshRate();
    if (elapsedMS > 2.2 * refreshPeriodMS) {  // Add a fudge factor of 20% to avoid reporting near misses
        mwarning(M_PARADIGM_MESSAGE_DOMAIN,
                 "%s action took more than two display refresh cycles (%g ms) to complete",
                 getActionName(),
                 elapsedMS);
    }
    
    return false;
}


/****************************************************************
 *                 UpdateStimulusDisplay Methods
 ****************************************************************/


void UpdateStimulusDisplay::describeComponent(ComponentInfo &info) {
    StimulusDisplayAction::describeComponent(info);
    info.setSignature("action/update_stimulus_display");
}


UpdateStimulusDisplay::UpdateStimulusDisplay(const ParameterValueMap &parameters) :
    StimulusDisplayAction(parameters)
{
    setName("UpdateStimulusDisplay");
}


boost::shared_ptr<StimulusDisplay::UpdateInfo> UpdateStimulusDisplay::performAction() {
    return display->updateDisplay();
}


const char * UpdateStimulusDisplay::getActionName() const {
    return "update_stimulus_display";
}


/****************************************************************
 *                 ClearStimulusDisplay Methods
 ****************************************************************/


void ClearStimulusDisplay::describeComponent(ComponentInfo &info) {
    StimulusDisplayAction::describeComponent(info);
    info.setSignature("action/clear_stimulus_display");
}


ClearStimulusDisplay::ClearStimulusDisplay(const ParameterValueMap &parameters) :
    StimulusDisplayAction(parameters)
{
    setName("ClearStimulusDisplay");
}


boost::shared_ptr<StimulusDisplay::UpdateInfo> ClearStimulusDisplay::performAction() {
    return display->clearDisplay();
}


const char * ClearStimulusDisplay::getActionName() const {
    return "clear_stimulus_display";
}


/****************************************************************
 *                 SoundAction Methods
 ****************************************************************/


const std::string SoundAction::SOUND("sound");


void SoundAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(SOUND);
}


template<>
boost::shared_ptr<Sound> ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    auto sound = reg->getObject<Sound>(s);
    if (!sound) {
        throw SimpleException("Unknown sound", s);
    }
    return sound;
}


SoundAction::SoundAction(const ParameterValueMap &parameters) :
    Action(parameters),
    sound(parameters[SOUND])
{ }


/****************************************************************
 *                 LoadSound Methods
 ****************************************************************/


void LoadSound::describeComponent(ComponentInfo &info) {
    SoundAction::describeComponent(info);
    info.setSignature("action/load_sound");
}


LoadSound::LoadSound(const ParameterValueMap &parameters) :
    SoundAction(parameters)
{
    setName("LoadSound");
}


bool LoadSound::execute() {
    //
    // Sound loading can fail with an exception due to legitimate runtime errors
    // (e.g. a bad audio file path).  Since we don't want such an exception to halt
    // the state system (or crash the server by not getting caught on another thread),
    // catch all exceptions and convert them to error messages.
    //
    try {
        sound->load();
    } catch (const std::exception &e) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Sound loading failed: %s", e.what());
    } catch (...) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Sound loading failed");
    }
    
    return true;
}


/****************************************************************
 *                 UnloadSound Methods
 ****************************************************************/


void UnloadSound::describeComponent(ComponentInfo &info) {
    SoundAction::describeComponent(info);
    info.setSignature("action/unload_sound");
}


UnloadSound::UnloadSound(const ParameterValueMap &parameters) :
    SoundAction(parameters)
{
    setName("UnloadSound");
}


bool UnloadSound::execute() {
    sound->unload();
    return true;
}


/****************************************************************
 *                 PlaySound Methods
 ****************************************************************/


const std::string PlaySound::START_TIME("start_time");


void PlaySound::describeComponent(ComponentInfo &info) {
    SoundAction::describeComponent(info);
    info.setSignature("action/play_sound");
    info.addParameter(START_TIME, false);
}


PlaySound::PlaySound(const ParameterValueMap &parameters) :
    SoundAction(parameters),
    startTime(optionalVariable(parameters[START_TIME]))
{
    setName("PlaySound");
}


bool PlaySound::execute() {
    if (!startTime) {
        sound->play();
    } else {
        const auto currentStartTime = startTime->getValue().getInteger();
        if (currentStartTime < Clock::instance()->getCurrentTimeUS()) {
            mwarning(M_PARADIGM_MESSAGE_DOMAIN,
                     "Requested sound start time is in the past; sound will play immediately");
            sound->play();
        } else {
            sound->play(currentStartTime);
        }
    }
    return true;
}


/****************************************************************
 *                 PauseSound Methods
 ****************************************************************/


void PauseSound::describeComponent(ComponentInfo &info) {
    SoundAction::describeComponent(info);
    info.setSignature("action/pause_sound");
}


PauseSound::PauseSound(const ParameterValueMap &parameters) :
    SoundAction(parameters)
{
    setName("PauseSound");
}


bool PauseSound::execute() {
    sound->pause();
    return true;
}


/****************************************************************
 *                 StopSound Methods
 ****************************************************************/


void StopSound::describeComponent(ComponentInfo &info) {
    SoundAction::describeComponent(info);
    info.setSignature("action/stop_sound");
}


StopSound::StopSound(const ParameterValueMap &parameters) :
    SoundAction(parameters)
{
    setName("StopSound");
}


bool StopSound::execute() {
    sound->stop();
    return true;
}


/****************************************************************
 *                 StartDeviceIO Methods
 ****************************************************************/
StartDeviceIO::StartDeviceIO(shared_ptr<IODevice> _device){ 
	device = weak_ptr<IODevice>(_device); 
	setName("StartDeviceIO");
}

StartDeviceIO::~StartDeviceIO(){/* do nothing */ };

bool StartDeviceIO::execute(){	
	shared_ptr<IODevice> _device(device);
	
	if(_device == NULL){
		merror(M_IODEVICE_MESSAGE_DOMAIN,
			   "Attempt to start IO on a non-existent device");
		return false;
	}
	
	//	mprintf("Starting IO device");
	_device->startDeviceIO();
	return true;
}

shared_ptr<mw::Component> StartDeviceIOFactory::createObject(std::map<std::string, std::string> parameters,
														   ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "device");
	
	shared_ptr<IODevice> theIODevice = reg->getObject<IODevice>(parameters.find("device")->second);
	
	checkAttribute(theIODevice, parameters["reference_id"], "device", parameters.find("device")->second);		
	
	
	shared_ptr <mw::Component> newStartDeviceIOAction = shared_ptr<mw::Component>(new StartDeviceIO(theIODevice));
	return newStartDeviceIOAction;		
}


/****************************************************************
 *                 StopDeviceIO Methods
 ****************************************************************/
StopDeviceIO::StopDeviceIO(shared_ptr<IODevice> _device){ 
	device = weak_ptr<IODevice>(_device); 
}

StopDeviceIO::~StopDeviceIO(){ /* do nothing */ } 

bool StopDeviceIO::execute(){
	shared_ptr<IODevice> _device(device);
	if(_device == NULL){
		merror(M_IODEVICE_MESSAGE_DOMAIN,
			   "Attempt to stop IO on a non-existent device");
		return false;
	}
	
	//	mprintf("Stopping IO device");
	_device->stopDeviceIO();
	return true;
}

shared_ptr<mw::Component> StopDeviceIOFactory::createObject(std::map<std::string, std::string> parameters,
														  ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "device");
	
	shared_ptr<IODevice> theIODevice = reg->getObject<IODevice>(parameters.find("device")->second);
	
	checkAttribute(theIODevice, parameters["reference_id"], "device", parameters.find("device")->second);		
	
	
	shared_ptr <mw::Component> newStopDeviceIOAction = shared_ptr<mw::Component>(new StopDeviceIO(theIODevice));
	return newStopDeviceIOAction;		
}


/****************************************************************
 *                 ResetSelection Methods
 ****************************************************************/
shared_ptr<mw::Component> ResetSelectionFactory::createObject(std::map<std::string, std::string> parameters,
															ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "selection");
	
    shared_ptr<Selectable> sel;
        
    sel = reg->getObject<Selectable>(parameters["selection"], parameters["parent_scope"]);
    
	// TODO something funny happens with Selectable's
	if(sel == NULL) {
		throw MissingReferenceException(parameters["reference_id"], "selection", parameters.find("selection")->second);		
	}
	
	
	shared_ptr <mw::Component> newResetSelectionAction = shared_ptr<mw::Component>(new ResetSelection(sel));
	return newResetSelectionAction;		
}

/****************************************************************
 *                 AcceptSelections Methods
 ****************************************************************/
shared_ptr<mw::Component> AcceptSelectionsFactory::createObject(std::map<std::string, std::string> parameters,
															  ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "selection");
	
    shared_ptr<Selectable> sel = reg->getObject<Selectable>(parameters["selection"], parameters["parent_scope"]);
    //shared_ptr<Selectable> sel = reg->getObject<Selectable>(parameters.find("selection")->second);
	
	// TODO something funny happens with Selectable's
	//	checkAttribute(sel, parameters["reference_id"], "selection", parameters.find("selection")->second);		
	if(sel == 0) {
		throw MissingReferenceException(parameters["reference_id"], "selection", parameters.find("selection")->second);		
	}
	
	
	shared_ptr <mw::Component> newAcceptSelectionsAction = shared_ptr<mw::Component>(new AcceptSelections(sel));
	return newAcceptSelectionsAction;		
}

/****************************************************************
 *                 RejectSelections Methods
 ****************************************************************/
shared_ptr<mw::Component> RejectSelectionsFactory::createObject(std::map<std::string, std::string> parameters,
															  ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "selection");
	
    //shared_ptr<Selectable> sel = reg->getObject<Selectable>(parameters.find("selection")->second);
    shared_ptr<Selectable> sel = reg->getObject<Selectable>(parameters["selection"], parameters["parent_scope"]);

    
	// TODO something funny happens with Selectable's
	//	checkAttribute(sel, parameters["reference_id"], "selection", parameters.find("selection")->second);		
	if(sel == 0) {
		throw MissingReferenceException(parameters["reference_id"], "selection", parameters.find("selection")->second);		
	}
	
	
	shared_ptr <mw::Component> newRejectSelectionsAction = shared_ptr<mw::Component>(new RejectSelections(sel));
	return newRejectSelectionsAction;		
}

/****************************************************************
 *                 If Methods
 ****************************************************************/


const std::string If::CONDITION("condition");


void If::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/if");
    info.addParameter(CONDITION);
}


If::If(const ParameterValueMap &parameters) :
    Action(parameters),
    condition(parameters[CONDITION])
{ }


If::If(const VariablePtr &condition) :
    condition(condition)
{ }


void If::addAction(const boost::shared_ptr<Action> &action) {
    action->setParent(getParent());
    actions.push_back(action);
}


void If::addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<mw::Component> child)
{
    auto action = boost::dynamic_pointer_cast<Action>(child);
    if (!action) {
        throw SimpleException("if can only contain actions");
    }
    addAction(action);
}


bool If::execute() {
    bool shouldExecute = condition->getValue().getBool();

    if (shouldExecute) {
        for (auto &action : actions) {
            action->execute();
        }
    }
    
    return shouldExecute;
}


/****************************************************************
 *                 Else Methods
 ****************************************************************/


void Else::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/else");
}


Else::Else(const ParameterValueMap &parameters) :
    Action(parameters)
{ }


void Else::addChild(std::map<std::string, std::string> parameters,
                    ComponentRegistry *reg,
                    boost::shared_ptr<mw::Component> child)
{
    auto action = boost::dynamic_pointer_cast<Action>(child);
    if (!action) {
        throw SimpleException("else can only contain actions");
    }
    action->setParent(getParent());
    actions.push_back(action);
}


bool Else::execute() {
    for (auto &action : actions) {
        action->execute();
    }
    return true;
}


/****************************************************************
 *                 IfElse Methods
 ****************************************************************/


void IfElse::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/if_else");
}


IfElse::IfElse(const ParameterValueMap &parameters) :
    Action(parameters)
{ }


void IfElse::addChild(std::map<std::string, std::string> parameters,
                      ComponentRegistry *reg,
                      boost::shared_ptr<Component> child)
{
    if (auto cond = boost::dynamic_pointer_cast<If>(child)) {
        conditionals.push_back(cond);
    } else if (auto uncond = boost::dynamic_pointer_cast<Else>(child)) {
        if (unconditional) {
            throw SimpleException("if/else can contain only one unconditional (else) action");
        }
        unconditional = uncond;
    } else {
        throw SimpleException("if/else can contain only conditional (if) and unconditional (else) actions");
    }
}


bool IfElse::execute() {
    if (conditionals.empty()) {
        mwarning(M_PARADIGM_MESSAGE_DOMAIN, "if/else contains no conditional (if) actions");
    }
    
    for (auto &cond : conditionals) {
        if (cond->execute()) {
            return true;
        }
    }
    
    if (unconditional) {
        unconditional->execute();
    }
    
    return false;
}


/****************************************************************
 *                 While Methods
 ****************************************************************/


const std::string While::CONDITION("condition");


void While::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/while");
    info.addParameter(CONDITION);
}


While::While(const ParameterValueMap &parameters) :
    Action(parameters),
    condition(parameters[CONDITION]),
    shouldRepeat(false)
{ }


void While::addChild(std::map<std::string, std::string> parameters,
                     ComponentRegistry *reg,
                     boost::shared_ptr<Component> child)
{
    auto action = boost::dynamic_pointer_cast<Action>(child);
    if (!action) {
        throw SimpleException("while can only contain actions");
    }
    action->setParent(getParent());
    actions.push_back(action);
}


bool While::execute() {
    if (StateSystem::instance()->getCurrentState().lock().get() == this) {
        shouldRepeat = performIteration();
    } else {
        // If we're executing outside of the normal state system (e.g. as part of a ScheduledActions instance),
        // we need to perform all our iterations right here
        while (performIteration()) /*repeat*/ ;
    }
    return true;
}


boost::weak_ptr<State> While::next() {
    if (shouldRepeat) {
        // We need to perform another iteration, so return ourselves as the next state
        return component_shared_from_this<While>();
    }
    return Action::next();
}


bool While::performIteration() {
    bool shouldExecute = condition->getValue().getBool();
    
    if (shouldExecute) {
        for (auto &action : actions) {
            action->execute();
        }
    }
    
    return shouldExecute;
}


/****************************************************************
 *                 TransitionCondition Methods
 ****************************************************************/	
TransitionCondition::TransitionCondition(shared_ptr<Variable> v1, 
										   weak_ptr<State> trans) {
	//    mprintf("Instantiating conditional transitions");
	condition = v1;
	always_go = false;
	if(trans.expired()) {
		throw SimpleException("An invalid transition has been created that, if used, will likely result in an infinite loop");
	} else {
		transition = trans;
	}
	name = "Transition: TransitionCondition";
}		

TransitionCondition::TransitionCondition(weak_ptr<State> trans) { 
	//condition = NULL;
	always_go = true;
	if(trans.expired()) {
		throw SimpleException("An invalid transition has been created that, if used, will likely result in an infinite loop");
	} else {
		transition = trans;
	}
}

TransitionCondition::~TransitionCondition() {
    //transition = NULL;
    //owner = NULL;
}

weak_ptr<State> TransitionCondition::execute() {
    
	
	if(always_go){
		
		if(!transition.expired()){
			currentState->setValue(getCompactID());
            return transition;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to advance to an invalid transition state");
		}
	} else if((bool)(*condition)) {
		//		fprintf(stderr, "===> Going because the condition evaluated true\n");
		//		fflush(stderr); 
		
		if(!transition.expired()){
			currentState->setValue(getCompactID());
			return transition;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to advance to an invalid transition state");
		}
    } else {
		return weak_ptr<State>();
	}
}

void TransitionCondition::setOwner(weak_ptr<State> _owner) {
	owner = _owner;
}

weak_ptr<State> TransitionCondition::getOwner() {
	return owner;
}

weak_ptr<State>  TransitionCondition::getTransition() {
    return transition;
}

shared_ptr<mw::Component> TransitionFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "type");
	
	shared_ptr<mw::Component> newTransition;
	    
	if(boost::algorithm::to_lower_copy(parameters.find("type")->second) == "yield") {
		newTransition = shared_ptr<mw::Component>(new YieldToParent());	 
	} else {
		// if it doesn't yield it needs a target
		REQUIRE_ATTRIBUTES(parameters, "type", "target");
		shared_ptr<State> target = reg->getObject<State>(parameters.find("target")->second, parameters["parent_scope"]);
		
		checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
		
		string type = boost::algorithm::to_lower_copy(parameters.find("type")->second);
		if(type == "conditional") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target", "condition");
			shared_ptr<Variable> condition = reg->getVariable(parameters.find("condition")->second);			
			
			checkAttribute(condition, parameters["reference_id"], "condition", parameters.find("condition")->second);		
			
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			
			newTransition = shared_ptr<mw::Component>(new TransitionCondition(condition, target));
		} else if(type == "timer_expired") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target", "timer");
			shared_ptr<Timer> timer = reg->getObject<Timer>(parameters.find("timer")->second);
			
			checkAttribute(timer, parameters["reference_id"], "timer", parameters.find("timer")->second);		
			
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			
			newTransition = shared_ptr<mw::Component>(new TransitionIfTimerExpired(timer, target));
		} else if(type == "direct" || type == "") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target");
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			newTransition = shared_ptr<mw::Component>(new TransitionCondition(target));
        } else if (type == "goto") {
            if (parameters.find("when") == parameters.end()) {
                newTransition = boost::make_shared<TransitionCondition>(target);
            } else {
                auto when = reg->getVariable(parameters.find("when")->second);
                checkAttribute(when, parameters["reference_id"], "when", parameters.find("when")->second);
                newTransition = boost::make_shared<TransitionCondition>(when, target);
            }
		} else {
			throw SimpleException("Illegal type for transition");
		}
	}		
	return newTransition;
}

/****************************************************************
 *                 TransitionIfTimerExpired Methods
 ****************************************************************/	
TransitionIfTimerExpired::TransitionIfTimerExpired(shared_ptr<Timer> _timer, 
													 weak_ptr<State> trans)
: TransitionCondition(trans) {
	timer = _timer;
    always_go = false;
	name = "Transition: TransitionIfTimerExpired";
}


weak_ptr<State> TransitionIfTimerExpired::execute() {
    
	if(timer->hasExpired()) {
		//fprintf(stderr, "===> Going because timer expired\n");
		//fflush(stderr);
		if(!transition.expired()){
			currentState->setValue(getCompactID());
            return transition;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to advance to an invalid transition state");
		}
    } else {
        return weak_ptr<State>();
    }
}



///****************************************************************
// *                 TransitionIfTimerExpired Methods
// ****************************************************************/	
//mTransitionIfEggTimerExpired::TransitionIfEggTimerExpired(
//														   shared_ptr<State> trans)
//: TransitionCondition(trans) {
//    always_go = false;
//}
//
//
//mState *TransitionIfEggTimerExpired::execute() {
//    if(hasTimerExpired()) {
//        return transition.get();
//    } else {
//        return NULL;
//    }
//}


//mState *TransitionIfEggTimerExpired::execute() {
//    if(hasTimerExpired()) {
//		currentState->setValue(name + " to " + transition->getName());
//        return transition.get();
//    } else {
//        return NULL;
//    }
//}

/****************************************************************
 *                 YieldToParent Methods
 ****************************************************************/
YieldToParent::YieldToParent() 
: TransitionCondition(shared_ptr<State>(GlobalCurrentExperiment)) {
	// surely not the right place to go, but might avoid a crash    
    // mprintf("YieldToParent Constructor called");
	name = "Transition: YieldToParent";
}

YieldToParent::~YieldToParent() {
	
}

weak_ptr<State> YieldToParent::execute() {
	
	currentState->setValue(getCompactID());
	//currentState->setValue(name);
	//	fprintf(stderr, "===> Yielding to parent\n");
	//	fflush(stderr);
	
	shared_ptr<State> owner_shared(owner);
	
  	if(owner_shared == NULL) {
		merror(M_PARADIGM_MESSAGE_DOMAIN, 
			   "Attempting to yield in an owner-less transition");
		
		// TODO: is this implicit cast kosher?
		return weak_ptr<State>(GlobalCurrentExperiment);
	}
	
	shared_ptr<State> parent = owner_shared->getParent();
	if (!parent) {
		merror(M_PARADIGM_MESSAGE_DOMAIN, 
			   "Attempting to yield in an owner-less transition");
		return GlobalCurrentExperiment;
	}
	return parent;
}

/****************************************************************
 *                 TaskSystemState Methods
 ****************************************************************/


void TaskSystemState::describeComponent(ComponentInfo &info) {
    ContainerState::describeComponent(info);
    info.setSignature("task_system_state");
}


TaskSystemState::TaskSystemState() {
	setTag("TaskSystemState");
}


TaskSystemState::TaskSystemState(const ParameterValueMap &parameters) :
    ContainerState(parameters)
{ }


shared_ptr<mw::Component> TaskSystemState::createInstanceObject(){
    shared_ptr<TaskSystemState> new_state = clone<TaskSystemState>();
    new_state->transition_list = transition_list;
	return new_state;
}


weak_ptr<State> TaskSystemState::next() {
    if (currentActionIndex < getList().size()) {
        shared_ptr<State> action = getList()[currentActionIndex++];
        
        shared_ptr<State> actionParent(action->getParent());
        if (actionParent.get() != this) {
            action->setParent(component_shared_from_this<State>());
            action->updateHierarchy();
        }
        
        action->updateCurrentScopedVariableContext();
        
        return action;
    }
    
    weak_ptr<State> trans;
	if(transition_list->size() == 0) {
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "No valid transitions.  Ending experiment.");
		trans = weak_ptr<State>(GlobalCurrentExperiment);
		return trans;
	}			
	
    BOOST_FOREACH( shared_ptr<TransitionCondition> condition, *transition_list ) {
		trans = condition->execute();
        shared_ptr<State> trans_shared = trans.lock();
		if(trans_shared) {
			shared_ptr<State> parent_shared(getParent());
			if(trans_shared.get() != parent_shared.get()){
				trans_shared->setParent(parent_shared); // TODO: this gets set WAY too many times
                trans_shared->updateHierarchy();
			}
			
			trans_shared->updateCurrentScopedVariableContext();
            reset();
			return trans;
		}
	}	
	return weak_ptr<State>();
}


void TaskSystemState::reset() {
    ContainerState::reset();
    currentActionIndex = 0;
}


void TaskSystemState::addChild(std::map<std::string, std::string> parameters,
								ComponentRegistry *reg,
								shared_ptr<mw::Component> comp){
	
	shared_ptr<Action> as_action = boost::dynamic_pointer_cast<Action, mw::Component>(comp);
	if(as_action != NULL){
        if (!transition_list->empty()) {
            throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                                  "Actions must be placed before transitions in task system state");
        }
        return ContainerState::addChild(parameters, reg, comp);
	}
	
	shared_ptr<TransitionCondition> as_transition = boost::dynamic_pointer_cast<TransitionCondition, mw::Component>(comp);
	if(as_transition != NULL){
		return addTransition(as_transition);
	}
	
	throw SimpleException("Attempting to add something (" + comp->getTag() + ") to task state (" + this->getTag() + ") that is not a transition or action");
}


void TaskSystemState::addTransition(shared_ptr<TransitionCondition> trans) {
	if(!trans) {
		mprintf("Attempt to add NULL transition");
		return;
	}
	trans->setOwner(component_shared_from_this<State>());
	transition_list->push_back(trans);
}


/****************************************************************
 *                 TaskSystem Methods
 ****************************************************************/


void TaskSystem::describeComponent(ComponentInfo &info) {
    ContainerState::describeComponent(info);
    info.setSignature("task_system");
}


TaskSystem::TaskSystem() {
	setTag("TaskSystem");
}


TaskSystem::TaskSystem(const ParameterValueMap &parameters) :
    ContainerState(parameters)
{ }


shared_ptr<mw::Component> TaskSystem::createInstanceObject(){
    shared_ptr<TaskSystem> new_state(clone<TaskSystem>());
	
    shared_ptr<ScopedVariableEnvironment> env_shared = getExperiment();
	if(env_shared){
		shared_ptr<ScopedVariableContext> con = env_shared->createNewContext();
		
		new_state->setLocalScopedVariableContext(con);
	} else {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to clone a state without an associated scoped environment");
	}
    
    return new_state;
	
}


void TaskSystem::action() {
    ContainerState::action();
	updateHierarchy();  // TODO: need to rethink how all of this is working...
}


weak_ptr<State> TaskSystem::next() {
	if (accessed) {
        return State::next();
	} else {
		if(getList().size() > 0) {
			accessed = true;
			//mprintf("Trial execution triggered");
			return getList()[0];
		} else {
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Task system contains no states");
            return State::next();
		}
	}
}


void TaskSystem::addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistry *reg,
                          shared_ptr<mw::Component> comp)
{
    ContainerState::addChild(parameters, reg, comp);
    string full_tag = parameters["parent_tag"];
    full_tag += "/";
    full_tag += parameters["child_tag"];
}


/****************************************************************
 *                 StopExperiment Methods
 ****************************************************************/


void StopExperiment::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/stop_experiment");
}


StopExperiment::StopExperiment(const ParameterValueMap &parameters) :
    Action(parameters)
{ }


bool StopExperiment::execute() {
    StateSystem::instance()->stop();
    return true;
}


/****************************************************************
 *                 PauseExperiment Methods
 ****************************************************************/


void PauseExperiment::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/pause_experiment");
}


PauseExperiment::PauseExperiment(const ParameterValueMap &parameters) :
    Action(parameters)
{ }


bool PauseExperiment::execute() {
    StateSystem::instance()->pause();
    return true;
}


/****************************************************************
 *                 ResumeExperiment Methods
 ****************************************************************/


void ResumeExperiment::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/resume_experiment");
}


ResumeExperiment::ResumeExperiment(const ParameterValueMap &parameters) :
    Action(parameters)
{ }


bool ResumeExperiment::execute() {
    StateSystem::instance()->resume();
    return true;
}


/*ExpandableList<State> * TaskSystem::getTaskSystemStates() {
 return list;
 }*/

/****************************************************************
 *                 BasicFixationElement Methods
 ****************************************************************/
/*
 BasicFixationElement::BasicFixationElement(State *parent,
 State *_aborttrial, State *_fixated,
 Variable_ITI_time, Variable _ITI_delta,
 Variable_acquire_time, Variable _eot_code,
 Variable_fix_x, Variable _fix_y, Variable _fix_frame,
 Variable_block_pause = M_PARAM_ZERO, 
 Variable_break_pause = M_PARAM_ZERO)
 : TaskSystemBox(parent) {
 aborttrial = _aborttrial;
 fixating = _fixating;
 ITI_time = _ITI_time;
 ITI_delta = _ITI_delta;
 acquire_time = _acquire_time;
 eot_code = _eot_code;
 fix_x = _fix_x;
 fix_y = _fix_y;
 fix_frame = _fix_frame;
 block_pause = _block_pause;
 break_pause = _break_pause;
 initSubElements();
 }
 
 BasicFixationElement::initSubElement() {
 intertrial = new VariableWaitTrial(this, ITI_time, ITI_delta);
 fixation = new TaskSystemState(this);
 fixation->addAction(new Display(fix_frame));
 fixation->addAction(new Wait(acquire_time));
 fixation->addTransition(new TransitionCondition(in_fix_window, 
 true, fixating);
 fixation->addTransition(new TransitionCondition(expired, true, ignored); 
 
 }		
 */
/****************************************************************
 *                 ITIAndFixationWithBlocking Methods
 ****************************************************************/
/*
 ITIAndFixationWithBlocking::ITIAndFixationWithBlock() { }
 ITIAndFixationWithBlocking::~ITIAndFixationWithBlocking() { }
 */


//#ifdef  JIM_DEVELOPMENT

/****************************************************************
 *                 eye calibration Methods
 ****************************************************************/

// action to trigger sampling of data from assigned vars.
// note that the true coords can be found in the calibration point

//mUpdateStimulusDisplay::UpdateStimulusDisplay() : Action() {
TakeCalibrationSampleNow::TakeCalibrationSampleNow(shared_ptr<Calibrator> _calibrator, 
													 shared_ptr<GoldStandard>_goldStandard) : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mTakeCalibrationSampleNow constructor has been called.");
    calibrator = _calibrator;
    goldStandard = _goldStandard;
	setName("TakeCalibrationSampleNow");
}

TakeCalibrationSampleNow::~TakeCalibrationSampleNow() {};

bool TakeCalibrationSampleNow::execute() {
    bool noErr = false;
    if (VERBOSE_ACTION_METHODS) mprintf("mTakeCalibrationSampleNow is executing...");
    if ((calibrator != NULL) && (goldStandard != NULL)) {
        noErr = calibrator->sampleNow(goldStandard);
    } else {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "mTakeCalibrationSampleNow has null pointer(s).");
        return (noErr);
    }
    return(noErr);
}

shared_ptr<mw::Component> TakeCalibrationSampleNowFactory::createObject(std::map<std::string, std::string> parameters,
																	  ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "calibrator", "calibratable_object");
	
	shared_ptr<Calibrator> cal = reg->getObject<Calibrator>(parameters.find("calibrator")->second);
	shared_ptr<GoldStandard> gs = reg->getObject<GoldStandard>(parameters.find("calibratable_object")->second);
	
	checkAttribute(cal, parameters["reference_id"], "calibrator", parameters.find("calibrator")->second);		
	
	// TODO something funny happens with GoldStandard's
	//	checkAttribute(gs, parameters["reference_id"], "calibratable_object", parameters.find("calibratable_object")->second);		
	if(gs == 0) {
		throw MissingReferenceException(parameters["reference_id"], "calibratable_object", parameters.find("calibratable_object")->second);		
	}
	
	shared_ptr <mw::Component> newTakeCalibrationSampleNowAction = shared_ptr<mw::Component>(new TakeCalibrationSampleNow(cal, gs));
	return newTakeCalibrationSampleNowAction;	
}		



StartAverageCalibrationSample::StartAverageCalibrationSample(shared_ptr<Calibrator> _calibrator)  : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mStartAverageCalibrationSample constructor has been called.");
    calibrator = _calibrator;
	setName("StartAverageCalibrationSample");
}

StartAverageCalibrationSample::~StartAverageCalibrationSample() {};
bool StartAverageCalibrationSample::execute() {
	if (VERBOSE_ACTION_METHODS) mprintf("mStartAverageCalibrationSample is executing...");
    calibrator->startAverage();
	return true;
}

shared_ptr<mw::Component> StartAverageCalibrationSampleFactory::createObject(std::map<std::string, std::string> parameters,
																		   ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "calibrator");
	
	shared_ptr<Calibrator> cal = reg->getObject<Calibrator>(parameters.find("calibrator")->second);
	
	checkAttribute(cal, parameters["reference_id"], "calibrator", parameters.find("calibrator")->second);		
	
	
	shared_ptr <mw::Component> newStartAverageCalibrationSampleAction = shared_ptr<mw::Component>(new StartAverageCalibrationSample(cal));
	return newStartAverageCalibrationSampleAction;
}		


EndAverageAndTakeCalibrationSample::EndAverageAndTakeCalibrationSample(shared_ptr<Calibrator> _calibrator, 
																		 shared_ptr<GoldStandard> _goldStandard) : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mTakeCalibrationSampleNow constructor has been called.");
    calibrator = _calibrator;
    goldStandard = _goldStandard;
	setName("EndAverageAndTakeCalibrationSample");
}

EndAverageAndTakeCalibrationSample::~EndAverageAndTakeCalibrationSample() {};
bool EndAverageAndTakeCalibrationSample::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("mTakeCalibrationSampleNow is executing...");
    return(calibrator->endAverageAndSample(goldStandard));
}

shared_ptr<mw::Component> EndAverageAndTakeCalibrationSampleFactory::createObject(std::map<std::string, std::string> parameters,
																				ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "calibrator", "calibratable_object");
	
	shared_ptr<Calibrator> cal = reg->getObject<Calibrator>(parameters.find("calibrator")->second);
	shared_ptr<GoldStandard> gs = reg->getObject<GoldStandard>(parameters.find("calibratable_object")->second);
	
	checkAttribute(cal, parameters["reference_id"], "calibrator", parameters.find("calibrator")->second);		
	
	// TODO something funny happens with GoldStandard's
	//	checkAttribute(gs, parameters["reference_id"], "calibratable_object", parameters.find("calibratable_object")->second);		
	if(gs == 0) {
		throw MissingReferenceException(parameters["reference_id"], "calibratable_object", parameters.find("calibratable_object")->second);		
	}
	
	
	
	shared_ptr <mw::Component> newEndAverageAndTakeCalibrationSampleAction = shared_ptr<mw::Component>(new EndAverageAndTakeCalibrationSample(cal, gs));
	return newEndAverageAndTakeCalibrationSampleAction;	
}		


EndAverageAndIgnore::EndAverageAndIgnore(shared_ptr<Calibrator> _calibrator)  : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mTakeCalibrationSampleNow constructor has been called.");
    calibrator = _calibrator;
	setName("EndAverageAndIgnore");
}

EndAverageAndIgnore::~EndAverageAndIgnore() {};
bool EndAverageAndIgnore::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("mEndAverageAndIgnore is executing...");
    return(calibrator->endAverageAndIgnore());
}

shared_ptr<mw::Component> EndAverageAndIgnoreFactory::createObject(std::map<std::string, std::string> parameters,
																 ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "calibrator");
	
	shared_ptr<Calibrator> cal = reg->getObject<Calibrator>(parameters.find("calibrator")->second);
	
	checkAttribute(cal, parameters["reference_id"], "calibrator", parameters.find("calibrator")->second);		
	
	
	
	shared_ptr <mw::Component> newEndAverageAndIgnoreAction = shared_ptr<mw::Component>(new EndAverageAndIgnore(cal));
	return newEndAverageAndIgnoreAction;
}		


// action to trigger updating of eye calibration parameters
CalibrateNow::CalibrateNow(shared_ptr<Calibrator> _calibrator) : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mCalibrateNow constructor has been called.");
    calibrator = _calibrator;
	setName("CalibrateNow");
}

CalibrateNow::~CalibrateNow() {};
bool CalibrateNow::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("mCalibrateNow is executing...");
    return(calibrator->calibrateNow());
}

shared_ptr<mw::Component> CalibrateNowFactory::createObject(std::map<std::string, std::string> parameters,
														  ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "calibrator");
	
	shared_ptr<Calibrator> cal = reg->getObject<Calibrator>(parameters.find("calibrator")->second);
	
	checkAttribute(cal, parameters["reference_id"], "calibrator", parameters.find("calibrator")->second);		
	
	
	shared_ptr <mw::Component> newCalibrateNowAction = shared_ptr<mw::Component>(new CalibrateNow(cal));
	return newCalibrateNowAction;
}		



// action to clear data in calibration
ClearCalibration::ClearCalibration(shared_ptr<Calibrator> _calibrator) : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mClearCalibration constructor has been called.");
    calibrator = _calibrator;
	setName("ClearCalibration");
}

ClearCalibration::~ClearCalibration() {};
bool ClearCalibration::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("mClearCalibration is executing...");
    return(calibrator->clearCalibrationData());
}

shared_ptr<mw::Component> ClearCalibrationFactory::createObject(std::map<std::string, std::string> parameters,
															  ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "calibrator");
	
	shared_ptr<Calibrator> cal = reg->getObject<Calibrator>(parameters.find("calibrator")->second);
	
	checkAttribute(cal, parameters["reference_id"], "calibrator", parameters.find("calibrator")->second);		
	
	
	shared_ptr <mw::Component> newClearCalibrationAction = shared_ptr<mw::Component>(new ClearCalibration(cal));
	return newClearCalibrationAction;
}		

// TODO -- action to allow partial clear of calibration



/****************************************************************
 *                 actions to control averagers
 ****************************************************************/


StartAverager::StartAverager(shared_ptr<AveragerUser> _averager) : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mStartAverager constructor has been called.");
    averager = _averager;
	setName("StartAverager");
}

StartAverager::~StartAverager() {};
bool StartAverager::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("mStartAverager is executing...");
    averager->start();
    return true;
}

shared_ptr<mw::Component> StartAveragerFactory::createObject(std::map<std::string, std::string> parameters,
														   ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "averager");
	
	shared_ptr<AveragerUser> avg = reg->getObject<AveragerUser>(parameters.find("averager")->second);
	
	checkAttribute(avg, parameters["reference_id"], "averager", parameters.find("averager")->second);		
	
	
	shared_ptr <mw::Component> newStartAveragerAction = shared_ptr<mw::Component>(new StartAverager(avg));
	return newStartAveragerAction;
}	

StopAverager::StopAverager(shared_ptr<AveragerUser> _averager) : Action()  {
    if (VERBOSE_ACTION_METHODS) mprintf("StopAverager constructor has been called.");
    averager = _averager;
	setName("StopAverager");
}

StopAverager::~StopAverager() {};
bool StopAverager::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("StopAverager is executing...");
    averager->stop();
    return true;
}

shared_ptr<mw::Component> StopAveragerFactory::createObject(std::map<std::string, std::string> parameters,
														  ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "averager");
	
	shared_ptr<AveragerUser> avg = reg->getObject<AveragerUser>(parameters.find("averager")->second);
	
	checkAttribute(avg, parameters["reference_id"], "averager", parameters.find("averager")->second);		
	
	
	shared_ptr <mw::Component> newStopAveragerAction = shared_ptr<mw::Component>(new StopAverager(avg));
	return newStopAveragerAction;
}	


ClearAverager::ClearAverager(shared_ptr<AveragerUser> _averager) : Action() {
    if (VERBOSE_ACTION_METHODS) mprintf("mClearAverager constructor has been called.");
    averager = _averager;
	setName("ClearAverager");
}

ClearAverager::~ClearAverager() {};
bool ClearAverager::execute() {
    if (VERBOSE_ACTION_METHODS) mprintf("mClearAverager is executing...");
    averager->reset();
    return true;
}

shared_ptr<mw::Component> ClearAveragerFactory::createObject(std::map<std::string, std::string> parameters,
														   ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "averager");
	
	shared_ptr<AveragerUser> avg = reg->getObject<AveragerUser>(parameters.find("averager")->second);
	
	checkAttribute(avg, parameters["reference_id"], "averager", parameters.find("averager")->second);		
	
	
	shared_ptr <mw::Component> newClearAveragerAction = shared_ptr<mw::Component>(new ClearAverager(avg));
	return newClearAveragerAction;
}	


// action to flush calibration samples up to some time?? (variable controlled?)

// action to load params into calibration??  (where should this happen?)


END_NAMESPACE_MW























