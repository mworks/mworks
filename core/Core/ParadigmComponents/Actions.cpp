//
//  Actions.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/20/23.
//

#include "Actions.hpp"

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


Action::Action(const ParameterValueMap &parameters) :
    State(parameters)
{ }


bool Action::execute() {
    return false;
}


void Action::action() {
    State::action();
    execute();
}


/****************************************************************
 *                  ContainerAction Methods
 ****************************************************************/


ContainerAction::ContainerAction(const ParameterValueMap &parameters) :
    Action(parameters),
    currentActionIndex(0)
{ }


void ContainerAction::action() {
    State::action();
    // Do *not* call execute()
    currentActionIndex = 0;
}


void ContainerAction::addAction(const boost::shared_ptr<Action> &action) {
    action->setParent(component_shared_from_this<State>());
    actions.emplace_back(action);
}


boost::shared_ptr<Action> ContainerAction::getNextAction() {
    boost::shared_ptr<Action> nextAction;
    if (currentActionIndex < actions.size()) {
        nextAction = actions.at(currentActionIndex);
        currentActionIndex++;
    }
    return nextAction;
}


bool ContainerAction::execute() {
    throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "Internal error", "execute() called on ContainerAction");
}


/****************************************************************
 *              ActionVariableNotification Methods
 ****************************************************************/


ActionVariableNotification::ActionVariableNotification(const boost::shared_ptr<Action> &action) :
    weakAction(action)
{ }


void ActionVariableNotification::notify(const Datum &data, MWTime time) {
    if (auto action = weakAction.lock()) {
        action->execute();
    } else {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attached action is no longer available");
    }
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
{ }


Assignment::Assignment(const VariablePtr &var, const VariablePtr &value) :
    assignment(var),
    value(value)
{ }


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
{ }


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
{ }


bool Wait::execute() {
    expirationTime = getExpirationTime();
    
    if (StateSystem::instance()->getCurrentState().get() != this) {
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
    if (!(parameters[TIMEOUT_MESSAGE].empty())) {
        timeoutMessage = parsedText(parameters[TIMEOUT_MESSAGE]);
    } else {
        timeoutMessage = boost::make_shared<ConstantVariable>(Datum("Timeout while waiting for condition: " + parameters[CONDITION].str()));
    }
}


bool WaitForCondition::execute() {
    deadline = clock->getCurrentTimeUS() + MWTime(*timeout);
    
    if (StateSystem::instance()->getCurrentState().get() != this) {
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
 *                 StimulusAction Methods
 ****************************************************************/


const std::string StimulusAction::STIMULUS("stimulus");


void StimulusAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(STIMULUS);
}


StimulusAction::StimulusAction(const ParameterValueMap &parameters) :
    Action(parameters),
    stimNode(parameters[STIMULUS])
{ }


bool StimulusAction::execute() {
    if (auto display = stimNode->getDisplay()) {
        performAction(stimNode, display);
    } else {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Stimulus has no associated display");
    }
    return true;
}


/****************************************************************
 *                       LoadStimulus Methods
 ****************************************************************/


void LoadStimulus::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/load_stimulus");
}


LoadStimulus::LoadStimulus(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void LoadStimulus::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    //
    // Stimulus loading can fail with an exception due to legitimate runtime errors
    // (e.g. a bad image file path).  Since we don't want such an exception to halt
    // the state system (or crash the server by not getting caught on another thread),
    // catch all exceptions and convert them to error messages.
    //
    try {
        stimNode->load(display);
    } catch (const std::exception &e) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Stimulus loading failed: %s", e.what());
    } catch (...) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Stimulus loading failed");
    }
}


/****************************************************************
 *                       UnloadStimulus Methods
 ****************************************************************/


void UnloadStimulus::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/unload_stimulus");
}


UnloadStimulus::UnloadStimulus(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void UnloadStimulus::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    stimNode->unload(display);
}


/****************************************************************
 *                       QueueStimulus Methods
 ****************************************************************/


void QueueStimulus::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/queue_stimulus");
}


QueueStimulus::QueueStimulus(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void QueueStimulus::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    if (!(stimNode->isLoaded()) &&
        stimNode->getDeferred() == Stimulus::deferred_load)
    {
        stimNode->load(display);
    }
    
    // freeze the stimulus
    stimNode->freeze();
    
    stimNode->setPendingVisible(true);
    stimNode->clearPendingRemoval();
    
    stimNode->addToDisplay(display);
}


/****************************************************************
 *                       LiveQueueStimulus Methods
 ****************************************************************/


void LiveQueueStimulus::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/live_queue_stimulus");
}


LiveQueueStimulus::LiveQueueStimulus(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void LiveQueueStimulus::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    if (!(stimNode->isLoaded()) &&
        stimNode->getDeferred() == Stimulus::deferred_load)
    {
        stimNode->load(display);
    }
    
    // don't freeze the stimulus
    stimNode->thaw();
    
    stimNode->setPendingVisible(true);
    stimNode->clearPendingRemoval();
    
    stimNode->addToDisplay(display);
}


/****************************************************************
 *                       DequeueStimulus Methods
 ****************************************************************/


void DequeueStimulus::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/dequeue_stimulus");
}


DequeueStimulus::DequeueStimulus(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void DequeueStimulus::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    stimNode->setPendingVisible(false);
    
    // set a flag that this node should be removed on the
    // next "explicit" update
    stimNode->setPendingRemoval();
}


/****************************************************************
 *                       BringStimulusToFront Methods
 ****************************************************************/


void BringStimulusToFront::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/bring_stimulus_to_front");
}


BringStimulusToFront::BringStimulusToFront(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void BringStimulusToFront::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    stimNode->bringToFront();
}


/****************************************************************
 *                       SendStimulusToBack Methods
 ****************************************************************/


void SendStimulusToBack::describeComponent(ComponentInfo &info) {
    StimulusAction::describeComponent(info);
    info.setSignature("action/send_stimulus_to_back");
}


SendStimulusToBack::SendStimulusToBack(const ParameterValueMap &parameters) :
    StimulusAction(parameters)
{ }


void SendStimulusToBack::performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) {
    stimNode->sendToBack();
}


/****************************************************************
 *                 StimulusDisplayAction Methods
 ****************************************************************/


const std::string StimulusDisplayAction::DISPLAY("display");
const std::string StimulusDisplayAction::PREDICTED_OUTPUT_TIME("predicted_output_time");


void StimulusDisplayAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(DISPLAY, false);
    info.addParameter(PREDICTED_OUTPUT_TIME, false);
}


StimulusDisplayAction::StimulusDisplayAction(const ParameterValueMap &parameters) :
    Action(parameters),
    display(parameters[DISPLAY]),
    predictedOutputTime(optionalVariable(parameters[PREDICTED_OUTPUT_TIME])),
    clock(Clock::instance())
{ }


bool StimulusDisplayAction::execute() {
    startTime = clock->getCurrentTimeUS();
    updateInfo = performAction();
    
    if (StateSystem::instance()->getCurrentState().get() != this) {
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
{ }


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
{ }


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
{ }


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
{ }


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
{ }


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
{ }


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
{ }


bool StopSound::execute() {
    sound->stop();
    return true;
}


/****************************************************************
 *                 StartDeviceIO Methods
 ****************************************************************/
StartDeviceIO::StartDeviceIO(shared_ptr<IODevice> _device){ 
	device = weak_ptr<IODevice>(_device); 
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
    ContainerAction::describeComponent(info);
    info.setSignature("action/if");
    info.addParameter(CONDITION);
}


If::If(const ParameterValueMap &parameters) :
    ContainerAction(parameters),
    condition(parameters[CONDITION]),
    shouldExecute(false),
    didExecute(false)
{ }


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


void If::action() {
    ContainerAction::action();
    shouldExecute = condition->getValue().getBool();
    didExecute = false;
}


boost::weak_ptr<State> If::next() {
    if (shouldExecute) {
        if (auto nextAction = getNextAction()) {
            return nextAction;
        }
        didExecute = true;
    }
    
    return ContainerAction::next();
}


/****************************************************************
 *                 Else Methods
 ****************************************************************/


void Else::describeComponent(ComponentInfo &info) {
    ContainerAction::describeComponent(info);
    info.setSignature("action/else");
}


Else::Else(const ParameterValueMap &parameters) :
    ContainerAction(parameters)
{ }


void Else::addChild(std::map<std::string, std::string> parameters,
                    ComponentRegistry *reg,
                    boost::shared_ptr<mw::Component> child)
{
    auto action = boost::dynamic_pointer_cast<Action>(child);
    if (!action) {
        throw SimpleException("else can only contain actions");
    }
    addAction(action);
}


boost::weak_ptr<State> Else::next() {
    if (auto nextAction = getNextAction()) {
        return nextAction;
    }
    return ContainerAction::next();
}


/****************************************************************
 *                 IfElse Methods
 ****************************************************************/


void IfElse::describeComponent(ComponentInfo &info) {
    ContainerAction::describeComponent(info);
    info.setSignature("action/if_else");
}


IfElse::IfElse(const ParameterValueMap &parameters) :
    ContainerAction(parameters),
    haveElse(false)
{ }


void IfElse::addChild(std::map<std::string, std::string> parameters,
                      ComponentRegistry *reg,
                      boost::shared_ptr<Component> child)
{
    if (auto cond = boost::dynamic_pointer_cast<If>(child)) {
        if (haveElse) {
            throw SimpleException("Conditional (if) actions must be placed before "
                                  "unconditional (else) actions in if/else");
        }
        addAction(cond);
    } else if (auto uncond = boost::dynamic_pointer_cast<Else>(child)) {
        if (haveElse) {
            throw SimpleException("if/else can contain only one unconditional (else) action");
        }
        addAction(uncond);
        haveElse = true;
    } else {
        throw SimpleException("if/else can contain only conditional (if) and unconditional (else) actions");
    }
}


void IfElse::action() {
    ContainerAction::action();
    currentIf.reset();
}


boost::weak_ptr<State> IfElse::next() {
    if (!(currentIf && currentIf->getDidExecute())) {
        if (auto nextAction = getNextAction()) {
            currentIf = boost::dynamic_pointer_cast<If>(nextAction);
            return nextAction;
        }
    }
    return ContainerAction::next();
}


/****************************************************************
 *                 While Methods
 ****************************************************************/


const std::string While::CONDITION("condition");


void While::describeComponent(ComponentInfo &info) {
    ContainerAction::describeComponent(info);
    info.setSignature("action/while");
    info.addParameter(CONDITION);
}


While::While(const ParameterValueMap &parameters) :
    ContainerAction(parameters),
    condition(parameters[CONDITION]),
    shouldExecute(false)
{ }


void While::addChild(std::map<std::string, std::string> parameters,
                     ComponentRegistry *reg,
                     boost::shared_ptr<Component> child)
{
    auto action = boost::dynamic_pointer_cast<Action>(child);
    if (!action) {
        throw SimpleException("while can only contain actions");
    }
    addAction(action);
}


void While::action() {
    ContainerAction::action();
    shouldExecute = condition->getValue().getBool();
}


boost::weak_ptr<State> While::next() {
    if (shouldExecute) {
        if (auto nextAction = getNextAction()) {
            return nextAction;
        }
        // We need to re-evaluate the condition (and potentially perform another iteration),
        // so return ourselves as the next state
        return component_shared_from_this<While>();
    }
    
    return ContainerAction::next();
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
