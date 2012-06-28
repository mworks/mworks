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
#include "ConstantVariable.h"
#include "StimulusDisplay.h"
#include "ExpressionVariable.h"

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
    State(),  // TODO: pass parameters once State supports them
    delay(NULL),
    taskRef(0)
{
    setOwner(GlobalCurrentExperiment);   // a bit of kludge for now
    setName("Action");
}

Action::Action() : State() {
    setOwner(GlobalCurrentExperiment);   // a bit of kludge for now
	delay = NULL;
	taskRef = 0;
	setName("Action");
}	

Action::~Action() {
    parent = weak_ptr<State>();
	if(taskRef) { taskRef->cancel(); }
}

bool Action::execute() {
    return false;
}

void Action::action(){ 
	announceEntry();
	execute(); 
	announceExit();
}

void Action::announceEntry() {
	currentState->setValue(getCompactID());
}

void Action::announceExit() {
    // no need to do this
}


void Action::setName(const std::string &_name) {
	State::setName("Action: " + _name);
}

weak_ptr<State> Action::next(){ 
	
	shared_ptr<State> parent_shared(parent);
	parent_shared->update();
	parent_shared->updateCurrentScopedVariableContext();
	return parent; 
}

void Action::setOwner(weak_ptr<State> _parent) {
	parent = _parent;
}

weak_ptr<State> Action::getOwner() {
	return parent;
}


weak_ptr<Experiment> Action::getExperiment() {
	if(!parent.expired()) {
		shared_ptr<State> parent_shared(parent);
		return parent_shared->getExperiment();
	} 	
	//mprintf("Action has no parent experiment");
	return weak_ptr<Experiment>();
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
Assignment::Assignment(shared_ptr<Variable> _var, 
						 shared_ptr<Variable> val) : Action() {
	var = _var;
	value = val;
	setName("Assignment");
}

Assignment::~Assignment() {
	
}

bool Assignment::execute() {
	var->operator=(value->getValue());
	return true;
}

shared_ptr<mw::Component> AssignmentFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "variable", "value");
	
	
	shared_ptr<Variable> variable = reg->getVariable(parameters.find("variable")->second);
	shared_ptr<Variable> val = reg->getVariable(parameters.find("value")->second);
	
	
	checkAttribute(val, parameters["reference_id"], "value", parameters.find("value")->second);
	checkAttribute(variable, parameters["reference_id"], "variable", parameters.find("variable")->second);
	
	
	shared_ptr <mw::Component> newAssignmentAction = shared_ptr<mw::Component>(new Assignment(variable, val));
	return newAssignmentAction;		
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
	
	shared_ptr<SelectionVariable> variable = dynamic_pointer_cast<SelectionVariable, Variable>(variable_uncast);
	
	if(variable == NULL){
		throw SimpleException((boost::format("Invalid object for attribute \"%s\" (should be a selection variable, but isn't)") % parameters["selection"]).str());
	}
	
	shared_ptr <mw::Component> newNextVariableSelection = shared_ptr<mw::Component>(new NextVariableSelection(variable));
	return newNextVariableSelection;		
}	


/****************************************************************
 *                       MessageAction Methods
 ****************************************************************/


const std::string MessageAction::MESSAGE("message");


MessageAction::MessageAction(const ParameterValueMap &parameters) :
    Action(parameters)
{ }


void MessageAction::parseMessage(const std::string &outStr) {
    stringFragments.clear();
    error = false;
    
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("", "().,;:|/\\+-*&^!@=<>?$ \t", boost::drop_empty_tokens);
    tokenizer tokens(outStr, sep);
    for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter){
        std::string token = *tok_iter;
        if(token == "$"){
            // peek ahead to the next token. if it starts with a word char, this is a variable
            ++tok_iter;
            if(tok_iter != tokens.end()){
                std::string next_token = *tok_iter;
                boost::regex re("^\\w");
                if(boost::regex_search(next_token, re)){  // if it's a word
                    shared_ptr<Variable> var = global_variable_registry->getVariable(next_token);
                    if(!var) {
                        error = true;
                        var = shared_ptr<ConstantVariable>(new ConstantVariable(Datum(std::string("UNKNOWNVAR"))));
                    }
                    
                    stringFragments.push_back(var);
                    
                } else { // if it's not a word (an isolated $, apparently)
                    
                    shared_ptr<ConstantVariable> c(new ConstantVariable(Datum("$" + next_token)));
                    stringFragments.push_back(c);
                }
                
            } else {
                break; // at the end of the token stream
            }
        } else {
            shared_ptr<ConstantVariable> c(new ConstantVariable(Datum(token)));
            stringFragments.push_back(c);
        }
    }         
}


std::string MessageAction::getMessage() const {
    std::string outStr("");
    
    for (std::vector< shared_ptr<Variable> >::const_iterator i = stringFragments.begin();
         i != stringFragments.end();
         i++)
    {
        outStr.append((*i)->getValue().toString());
    }
    
    return outStr;
}


/****************************************************************
 *                       ReportString Methods
 ****************************************************************/


void ReportString::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/report");
    info.addParameter(MESSAGE);
}


ReportString::ReportString(const ParameterValueMap &parameters) :
    MessageAction(parameters)
{
    setName("Report");
    parseMessage(parameters[MESSAGE].str());
}


ReportString::ReportString(const std::string &message) {
    setName("Report");
    parseMessage(message);
}


bool ReportString::execute() {
    std::string outStr = getMessage();
    shared_ptr <Clock> clock = Clock::instance();
    
    if(!error) {
        mprintf("%s (%lld)", outStr.c_str(), clock->getCurrentTimeUS());
    } else {
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
               "%s (%lld)", outStr.c_str(), clock->getCurrentTimeUS());
    }
    
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
    info.addParameter(MESSAGE, false);
    info.addParameter(STOP_ON_FAILURE, "0");
}


AssertionAction::AssertionAction(const ParameterValueMap &parameters) :
    MessageAction(parameters),
    condition(parameters[CONDITION]),
    stopOnFailure(parameters[STOP_ON_FAILURE])
{
    setName("Assertion");
    
    if (!(parameters[MESSAGE].empty())) {
        parseMessage(parameters[MESSAGE].str());
    } else {
        parseMessage("Condition failed: " + parameters[CONDITION].str());
    }
}


AssertionAction::AssertionAction(shared_ptr<Variable> condition, const std::string &message, bool stopOnFailure) :
    condition(condition),
    stopOnFailure(stopOnFailure)
{
    setName("Assertion");
    parseMessage(message);
}


bool AssertionAction::execute() {
    if (!(condition->getValue().getBool())) {
        std::string outStr = getMessage();
        assertionFailure->setValue(outStr);
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Assertion: %s", outStr.c_str());
        if (stopOnFailure) {
            merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Stopping experiment due to failed assertion");
            StateSystem::instance()->stop();
        }
        assertionFailure->setValue(0L);
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
 *                       StartTimer Methods
 ****************************************************************/

StartTimer::StartTimer(shared_ptr<Timer> _timer,  
						 shared_ptr<Variable> _time_to_wait_us) : Action() {
	setName("StartTimer");
    timer = _timer;
	time_to_wait_us = _time_to_wait_us;
}

StartTimer::StartTimer(shared_ptr<Timer> _timer, 
						 shared_ptr<TimeBase> _timebase, 
						 shared_ptr<Variable> _time_to_wait_us) : Action() {
	setName("StartTimer");
	timer = _timer;
	timebase = _timebase;
	time_to_wait_us = _time_to_wait_us;
}

bool StartTimer::execute() {
	
	MWTime offset;
	shared_ptr <Clock> clock = Clock::instance();
	
	if(timebase){
		offset = timebase->getTime();
		//mprintf("offset = %u, now = %u", (unsigned long)offset, (unsigned long)clock->getCurrentTimeUS());
	} else {
		offset = clock->getCurrentTimeUS();
	}
	
	//MWTime test = ((MWTime)(*time_to_wait_us));
	//long test2 = ((long)(*time_to_wait_us));
	//long test3 = (static_cast<ConstantVariable *>(time_to_wait_us))->getValue();
	
	
	
	// mprintf("## time_to_wait_us = %lld", ((MWTime)(*time_to_wait_us)));
	MWTime time_to_wait_value = (MWTime)time_to_wait_us->getValue();
    MWTime thetime = (time_to_wait_value + offset) -  (clock->getCurrentTimeUS());
	
	//mprintf("the time to wait = %u", (unsigned long)thetime);
	timer->startUS(thetime);
    return true;
}

shared_ptr<mw::Component> StartTimerFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "duration", "timer");
	
    shared_ptr<Variable> _timeToWait;
    
	shared_ptr<Variable> duration = reg->getVariable(parameters.find("duration")->second);
	checkAttribute(duration, parameters["reference_id"], "duration", parameters.find("duration")->second);		
    
    string duration_units = parameters["duration_units"];
   
    
    if(duration_units.empty() || duration_units == "us"){
        _timeToWait = duration;
    } else {
    
        ExpressionVariable e;
        if(duration_units == "s"){
            ConstantVariable one_million(1000000L);
            e = duration->operator*((Variable &)one_million);
        } else if(duration_units == "ms"){
            ConstantVariable one_thousand(1000L);
            e = duration->operator*((Variable &)one_thousand);
        }
        _timeToWait = shared_ptr<Variable>(e.clone());
    }
	
	shared_ptr<Timer> _timer = reg->getObject<Timer>(parameters.find("timer")->second);
	if(_timer == NULL) { // create a new timer
	 Datum zero((long)0);
		VariableProperties props(&zero, 
								  parameters.find("timer")->second,
								  parameters.find("timer")->second,
								  parameters.find("timer")->second + " is a timer",
								  M_NEVER, 
								  M_WHEN_CHANGED,
								  true, 
								  false,
								  M_DISCRETE_BOOLEAN,
								  "");
                                  
		_timer = global_variable_registry->createTimer(&props);
		reg->registerObject(parameters.find("timer")->second, _timer);
	}
	
	
	shared_ptr <mw::Component> newStartTimerAction;
	//std::map<std::string, std::string>::const_iterator timeBaseElement = parameters.find("timebase");
	//if(timeBaseElement != parameters.end()) {
	std::string timebaseString = parameters["timebase"];
	if(!timebaseString.empty()){
		// for when there's a timebase
		shared_ptr<TimeBase> _timeBase = reg->getObject<TimeBase>(timebaseString);
		//checkAttribute(_timeBase, parameters["reference_id"], "_timeBase", parameters.find("_timeBase")->second);
		checkAttribute(_timeBase, parameters["reference_id"], "timebase", parameters.find("timebase")->second);
		
		newStartTimerAction = shared_ptr<mw::Component>(new StartTimer(_timer, _timeBase, _timeToWait));
	} else {
		// no timebase
		newStartTimerAction = shared_ptr<mw::Component>(new StartTimer(_timer, _timeToWait));
	}
	return newStartTimerAction;		
}

///****************************************************************
//*                       StartEggTimer Methods
//****************************************************************/
//mStartEggTimer::StartEggTimer(shared_ptr<Variable> _time_to_wait_us) 
//																: Action() {
//    time_to_wait_us = _time_to_wait_us;
//	//mprintf("start eggtimer action %d us", (long)*time_to_wait_us);
//}
//
///*
//mStartEggTimer::StartEggTimer(long _time_to_wait_us) : Action() {
//    Datum the_time_data(_time_to_wait_us);
//	time_to_wait_us = (Variable *)(new ConstantVariable(new Datum(the_time_data))); // TODO leaks
//}*/
//
//mStartEggTimer::~StartEggTimer() {
//
//}
//
//bool StartEggTimer::execute() {
//    //long thetime = (long)(*time_to_wait_us);
//	//mprintf("fucker");
//	startEggTimer((MWTime)(*time_to_wait_us));
//    return true;
//}
//
//shared_ptr<Variable> StartEggTimer::getTimeToWait() {
//    return time_to_wait_us;
//}

/****************************************************************
 *                       Wait Methods
 ****************************************************************/
Wait::Wait(shared_ptr<Variable> time_us) : Action() {
    waitTime = time_us;
	timeBase = shared_ptr<TimeBase>();
	setName("Wait");
}

Wait::Wait(shared_ptr<TimeBase> _timeBase,
			 shared_ptr<Variable> time_us) : Action() {
    waitTime = time_us;
	timeBase = _timeBase;
	setName("Wait");
}

Wait::~Wait() {
	
}

bool Wait::execute() {
	
	MWTime timeToWait_us;
	
	shared_ptr <Clock> clock = Clock::instance();

	if(timeBase != 0){
		timeToWait_us = (timeBase->getTime() + (MWTime)(*waitTime)) - clock->getCurrentTimeUS();
	} else {
		timeToWait_us = ((MWTime)(*waitTime));
	}
	
	if(timeToWait_us > 0) {
		clock->sleepUS(timeToWait_us);
	}
	
	return true;
}

shared_ptr<Variable> Wait::getTimeToWait() {
    return waitTime;
}

shared_ptr<mw::Component> WaitFactory::createObject(std::map<std::string, std::string> parameters,
												  ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "duration");
	
    shared_ptr<Variable> _timeToWait;
    
	shared_ptr<Variable> duration = reg->getVariable(parameters.find("duration")->second);
	checkAttribute(duration, parameters["reference_id"], "_timeToWait", parameters.find("_timeToWait")->second);		
	
    string duration_units = parameters["duration_units"];
   
    
    if(duration_units.empty() || duration_units == "us"){
        _timeToWait = duration;
    } else {
    
        ExpressionVariable e;
        if(duration_units == "s"){
            ConstantVariable one_million(1000000L);
            e = duration->operator*((Variable &)one_million);
        } else if(duration_units == "ms"){
            ConstantVariable one_thousand(1000L);
            e = duration->operator*((Variable &)one_thousand);
        }
        _timeToWait = shared_ptr<Variable>(e.clone());
    }
    
	
	shared_ptr <mw::Component> newWaitAction;
	std::map<std::string, std::string>::const_iterator timeBaseElement = parameters.find("timebase");
	if(timeBaseElement != parameters.end()) {
		shared_ptr<TimeBase> _timeBase = reg->getObject<TimeBase>(timeBaseElement->second);
		checkAttribute(_timeBase, parameters["reference_id"], "_timeBase", parameters.find("_timeBase")->second);		
		
		newWaitAction = shared_ptr<mw::Component>(new Wait(_timeBase, _timeToWait));
	} else {
		newWaitAction = shared_ptr<mw::Component>(new Wait(_timeToWait));
	}
	
	return newWaitAction;	
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
    stimnode->load(display);
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
	
    bool loaded = stimnode->isLoaded();
    Stimulus::load_style deferred = (Stimulus::load_style)stimnode->getDeferred();
    if(deferred == Stimulus::deferred_load && !loaded){
      stimnode->load(display);
    }
  
    // freeze the stimulus
    stimnode->freeze();
    stimnode->setPendingVisible(true);
	
    stimnode->addToDisplay(display);
	
    //	display->addStimulusNode(stimnode);
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
	
  if(!stimnode->isLoaded()){
    stimnode->load(display);
  }
  
  // don't freeze the stimulus
  stimnode->thaw();
  stimnode->setPendingVisible(true);
  
	
	// see "WTF is going on here" above
	stimnode->addToDisplay(display);
	//	display->addStimulusNode(stimnode);
	
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
	//stimnode->setPendingRemoval();
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
	
	
	shared_ptr <mw::Component> newSendStimulusToBackAction = shared_ptr<mw::Component>(new BringStimulusToFront(stimulus));
	return newSendStimulusToBackAction;	
}

/****************************************************************
 *                 UpdateStimulusDisplay Methods
 ****************************************************************/
UpdateStimulusDisplay::UpdateStimulusDisplay() : Action() {
	setName("UpdateStimulusDisplay");
	experiment = weak_ptr<Experiment>(GlobalCurrentExperiment);
}

UpdateStimulusDisplay::~UpdateStimulusDisplay() {
	
}

// 2 frames at 60Hz
#define IMAGE_PRESENTATION_SLOP_US (1000000*(2*((float)1/60)))

bool UpdateStimulusDisplay::execute() {
    /*if(experiment == NULL) {
	 experiment = getExperiment();
	 }*/
	
	
	
    //if(experiment == NULL) {		// still bad?
	//experiment = weak_ptr<Experiment>(GlobalCurrentExperiment); // a bit of a kludge
    //}
    if(experiment.expired() ) {		// there's no making you happy
        merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "No experiment object on which to update the display");
        return false;
    } else {
		shared_ptr<Experiment> experiment_shared(experiment);
        shared_ptr<StimulusDisplay> display = experiment_shared->getStimulusDisplay();
		
		if(display){
			display->updateDisplay();
		} else {
			// TODO: warn
		}
        
		return true;
    }
	return true;
}

shared_ptr<mw::Component> UpdateStimulusDisplayFactory::createObject(std::map<std::string, std::string> parameters,
																   ComponentRegistry *reg) {
	shared_ptr <mw::Component> newUpdateStimulusDisplayAction = shared_ptr<mw::Component>(new UpdateStimulusDisplay());
	return newUpdateStimulusDisplayAction;	
}

/****************************************************************
 *                 PlaySound Methods
 ****************************************************************/
shared_ptr<mw::Component> PlaySoundFactory::createObject(std::map<std::string, std::string> parameters,
													   ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "sound");
	
	shared_ptr<Sound> theSound = reg->getObject<Sound>(parameters.find("sound")->second);
	
	checkAttribute(theSound, parameters["reference_id"], "sound", parameters.find("sound")->second);		
	
	
	shared_ptr <mw::Component> newPlaySoundAction = shared_ptr<mw::Component>(new PlaySound(theSound));
	return newPlaySoundAction;		
}

/****************************************************************
 *                 StopSound Methods
 ****************************************************************/
shared_ptr<mw::Component> StopSoundFactory::createObject(std::map<std::string, std::string> parameters,
													   ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "sound");
	
	shared_ptr<Sound> theSound = reg->getObject<Sound>(parameters.find("sound")->second);
	
	checkAttribute(theSound, parameters["reference_id"], "sound", parameters.find("sound")->second);		
	
	
	shared_ptr <mw::Component> newStopSoundAction = shared_ptr<mw::Component>(new StopSound(theSound));
	return newStopSoundAction;		
}

/****************************************************************
 *                 PauseSound Methods
 ****************************************************************/
shared_ptr<mw::Component> PauseSoundFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "sound");
	
	shared_ptr<Sound> theSound = reg->getObject<Sound>(parameters.find("sound")->second);
	
	checkAttribute(theSound, parameters["reference_id"], "sound", parameters.find("sound")->second);		
	
	
	shared_ptr <mw::Component> newPauseSoundAction = shared_ptr<mw::Component>(new PauseSound(theSound));
	return newPauseSoundAction;		
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
If::If(shared_ptr<Variable> v1) {
	condition = v1;
	setName("If");
}

If::~If() { }

void If::addAction(shared_ptr<Action> act) {
	act->setOwner(getOwner());
	actionlist.addReference(act);
}

void If::addChild(std::map<std::string, std::string> parameters,
					ComponentRegistry *reg,
					shared_ptr<mw::Component> child){
	shared_ptr<Action> act = dynamic_pointer_cast<Action,mw::Component>(child);
	if(act == 0) {
		throw SimpleException("Attempting to add illegal object (" + child->getTag() + ") to coditional (if) action (" + this->getTag() + ")");
	}
	
	addAction(act);
}

bool If::execute() {
    if((bool)(*condition)) {
        for(int i=0; i < actionlist.getNElements(); i++) {
            actionlist[i]->execute();
        }
    } else {
        for(int i = 0; i < elselist.getNElements(); i++) {
			elselist[i]->execute();
        }
    }
	return true;
}

shared_ptr<mw::Component> IfFactory::createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "condition");
	shared_ptr<Variable> condition = reg->getVariable(parameters.find("condition")->second);
	
	checkAttribute(condition, parameters["reference_id"], "condition", parameters.find("condition")->second);		
	
	
	// TODO ... needs more work here, possibly done in connection phase
	
	shared_ptr <mw::Component> newIfAction = shared_ptr<mw::Component>(new If(condition));
	return newIfAction;			
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
			shared_ptr<State> transition_shared(transition);
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
			shared_ptr<State> transition_shared(transition);
			//currentState->setValue(name + " to " + transition_shared->getName());
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
	    
	if(to_lower_copy(parameters.find("type")->second) == "yield") {
		newTransition = shared_ptr<mw::Component>(new YieldToParent());	 
	} else {
		// if it doesn't yield it needs a target
		REQUIRE_ATTRIBUTES(parameters, "type", "target");
		shared_ptr<State> target = reg->getObject<State>(parameters.find("target")->second, parameters["parent_scope"]);
		
		checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
		
		string type = to_lower_copy(parameters.find("type")->second);
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
			shared_ptr<State> transition_shared(transition);
			//currentState->setValue(name + " to " + transition_shared->getName());
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
	
	weak_ptr<State> parent = owner_shared->getParent();
	if(parent.expired()) {
		merror(M_PARADIGM_MESSAGE_DOMAIN, 
			   "Attempting to yield in an owner-less transition");
		return GlobalCurrentExperiment;
	}
	return parent;
}

/****************************************************************
 *                 TaskSystemState Methods
 ****************************************************************/
TaskSystemState::TaskSystemState() : State() {
    done = false;
	action_list = new ExpandableList<Action>(4);
	transition_list = new ExpandableList<TransitionCondition>(4);
	name = "mTaskSystemState";
}

//mTaskSystemState::TaskSystemState(State *parent) : State(parent) {
//	done = false;
//	action_list = new ExpandableList<Action>(4);
//	transition_list = new ExpandableList<TransitionCondition>(4);
//	name = "mTaskSystemState";
//}

TaskSystemState::~TaskSystemState() {
    
	if(!isAClone()){
		if(action_list) {
			delete action_list;
			action_list = NULL;
		}
		if(transition_list) {
			delete transition_list;
			transition_list = NULL;
		}
	}
}

shared_ptr<mw::Component> TaskSystemState::createInstanceObject(){
	//void *TaskSystemState::scopedClone(){
	
	shared_ptr<mw::Component> alias(getSelfPtr<mw::Component>());
	return alias;
	
	//mTaskSystemState *new_state = new TaskSystemState();
//	new_state->setParent(parent);
//	//new_state->setLocalScopedVariableContext(getLocalScopedVariableContext());
//	new_state->setExperiment(getExperiment());
//	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
//	new_state->setDescription(getDescription());
//	new_state->setName(getName());
//	
//	// TODO: copy the list objects?
//	new_state->setActionList(action_list);
//	new_state->setTransitionList(transition_list);
//	
//	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
//	
//	shared_ptr<mw::Component> clone_ptr(new_state);
//	return clone_ptr;
//	
}

void TaskSystemState::action() {
	currentState->setValue(getCompactID());
	//currentState->setValue(name);
	if(!done) {
		int nelem = action_list->getNElements();
		for(int i=0; i < nelem; i++) {
			
			shared_ptr<Action> the_action = (*action_list)[i]; 
			
			if(the_action == NULL){
				continue;
			}
			the_action->setScopedVariableEnvironment(getScopedVariableEnvironment());
			the_action->updateCurrentScopedVariableContext();
			
			//cerr << "a" << i << "(" << (*action_list)[i] << ", normal execution)" << endl;
			the_action->announceEntry();
			the_action->execute();
			the_action->announceExit();
		}
	}
}

/*void TaskSystemState::announceIdentity(){
 std::string announcement("Task System State: " + name);	
 announceState(announcement.c_str());
 Datum announceString;
 announceString.setString(announcement);
 currentState->setValue(announceString);
 }*/

weak_ptr<State> TaskSystemState::next() {
    weak_ptr<State> trans;
	if(transition_list->getNElements() == 0) {
		mprintf("Error: no valid transitions. Ending experiment");
		trans = weak_ptr<State>(GlobalCurrentExperiment);
		return trans;
	}			
	
	int nelem = transition_list->getNElements();
	for(int i = 0; i < nelem; i++) {
		//cerr << "t" << i <<  "(" << (*transition_list)[i] << ")" << endl;
		trans = (*transition_list)[i]->execute();
		if(!trans.expired()) {
			shared_ptr<State> trans_shared(trans);
			done = false;
			
			shared_ptr<State> parent_shared(parent);
			if(trans_shared.get() != parent_shared.get()){
				trans_shared->setParent(parent); // TODO: this gets set WAY too many times
			}
			
			trans_shared->updateCurrentScopedVariableContext();
			return trans;
		}
	}	
	return weak_ptr<State>();
}


void TaskSystemState::addChild(std::map<std::string, std::string> parameters,
								ComponentRegistry *reg,
								shared_ptr<mw::Component> comp){
	
	shared_ptr<Action> as_action = dynamic_pointer_cast<Action, mw::Component>(comp);
	if(as_action != NULL){
		return addAction(as_action);
	}
	
	shared_ptr<TransitionCondition> as_transition = dynamic_pointer_cast<TransitionCondition, mw::Component>(comp);
	if(as_transition != NULL){
		return addTransition(as_transition);
	}
	
	throw SimpleException("Attempting to add something (" + comp->getTag() + ") to task state (" + this->getTag() + ") that is not a transition or action");
}

void TaskSystemState::addAction(shared_ptr<Action> act) {
    if(!act) { 
        mprintf("Attempt to add a NULL action");
		return;
    }
	act->setOwner(getSelfPtr<State>());
	action_list->addReference(act);
}

void TaskSystemState::addTransition(shared_ptr<TransitionCondition> trans) {
	if(!trans) {
		mprintf("Attempt to add NULL transition");
		return;
	}
	trans->setOwner(getSelfPtr<State>());
	transition_list->addReference(trans);
}

ExpandableList<Action> * TaskSystemState::getActionList() {
    return action_list;
}

ExpandableList<TransitionCondition> * TaskSystemState::getTransitionList() {
    return transition_list;
}

/****************************************************************
 *                 WaitState Methods
 ****************************************************************/
/*
 WaitState::WaitState(State *parent, Variable *wait) 
 : TaskSystemState(parent) : wait_time(wait) {
 addAction(new Wait(wait_time));
 }
 */

/****************************************************************
 *                 TaskSystem Methods
 ****************************************************************/
// execute what's in the box, leaving the transition 
// list open to be user defined
TaskSystem::TaskSystem() : ContainerState() {
	
	execution_triggered = 0;
	name = "mTaskSystem";
}

//mTaskSystem::TaskSystem(State *parent) : ContainerState(parent) {
//	execution_triggered = 0;
//}

TaskSystem::~TaskSystem() {  }

shared_ptr<mw::Component> TaskSystem::createInstanceObject(){
	//void *TaskSystem::scopedClone(){
	
	TaskSystem *new_state;
	
	if(!parent.expired()){
		new_state = new TaskSystem();
		new_state->setParent(parent);
	} else {
		new_state = new TaskSystem();
	}
	
	//new_state->setLocalScopedVariableContext(new ScopedVariableContext());
	new_state->setExperiment(getExperiment());
	
	weak_ptr<ScopedVariableEnvironment> env = getScopedVariableEnvironment();
	new_state->setScopedVariableEnvironment(env);
	
	if(!env.expired()){
		shared_ptr<ScopedVariableEnvironment> env_shared(env);
		shared_ptr<ScopedVariableContext> con = env_shared->createNewContext();
		
		new_state->setLocalScopedVariableContext(con);
	} else {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to scopedClone a state without an associated scoped environment");
	}
	
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	new_state->setList(getList());
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
	
}

void TaskSystem::updateHierarchy() {
	if(parent.expired()){
		// do nothing
	} else {
        // update various settings with respect to hierarchy
		setParent(parent);
	}
	for(unsigned int i = 0; i < list->size(); i++) {
        // recurse down the hierarchy
		// TODO: here there be dragons
		weak_ptr<State> self_ptr = getSelfPtr<State>();
		(list->operator[](i))->setParent(self_ptr);
		
		(list->operator[](i))->updateHierarchy();
	}
}

void TaskSystem::action() {
	//execution_triggered = 1;
	currentState->setValue(getCompactID());
	//currentState->setValue(name);
	updateHierarchy();  // TODO: need to rethink how all of this is working...
}

/*void TaskSystem::announceIdentity(){
 std::string announcement("Task System: " + name);	
 announceState(announcement.c_str());
 Datum announceString;
 announceString.setString(announcement);
 currentState->setValue(announceString);
 }*/

weak_ptr<State> TaskSystem::next() {
	if(execution_triggered) {
		execution_triggered = 0;
		//mprintf("Returning parent");
		// TODO: deal with updating etc.
		if(!parent.expired()){
			shared_ptr<State> parent_shared(parent);
			parent_shared->update();
			parent_shared->updateCurrentScopedVariableContext();
			reset();
			return parent;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to access invalid parent object");
		}
	} else {
		if(list->size() > 0) {
			execution_triggered= 1;
			//mprintf("Trial execution triggered");
			return list->operator[](0);
		} else {
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Warning: trial object contains no list");
			if(!parent.expired()){
				shared_ptr<State> parent_shared(parent);
				parent_shared->updateCurrentScopedVariableContext();
				return parent;
			} else {
				// TODO: better throw
				throw SimpleException("Attempt to access an invalid parent");
			}
		}
	}
}

/*void TaskSystem::addTaskSystemState(shared_ptr<TaskSystemState> state) {
 addState(state);
 }*/

/*void TaskSystem::addState(State *state) {
 list->addReference(state);  // trial state now owns it, not us
 // set that state to have this one as it's parent
 // (so it knows who to yield control to)
 state->setParent(this);
 state->updateHierarchy();
 }*/

/*void TaskSystem::setStartState(State *state) {
 startstate = state;
 }*/

weak_ptr<State> TaskSystem::getStartState() {
	if(list->size() > 0) {
		return list->operator[](0);
	} else {
		mprintf("Error: attempt to run an empty trial");
		return parent;
	}
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























