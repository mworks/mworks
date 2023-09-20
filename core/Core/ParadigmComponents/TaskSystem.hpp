//
//  TaskSystem.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/20/23.
//

#ifndef TaskSystem_hpp
#define TaskSystem_hpp

#include "States.h"
#include "Timer.h"
#include "ComponentRegistry.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


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


END_NAMESPACE_MW


#endif /* TaskSystem_hpp */
