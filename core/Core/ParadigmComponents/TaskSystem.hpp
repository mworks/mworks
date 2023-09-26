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


class Transition : public Component { };


class TransitionFactory : public ComponentFactory {
public:
    virtual boost::shared_ptr<Component> createObject(std::map<std::string, std::string> parameters,
                                                      ComponentRegistry *reg);
};


class TransitionCondition : public Transition {
protected:
    boost::shared_ptr<Variable> condition;
    boost::weak_ptr<State> transition;
    bool always_go;
    
    boost::weak_ptr<State> owner;
    std::string name;
    
public:
    // if v1
    TransitionCondition(boost::shared_ptr<Variable> v1,
                        boost::weak_ptr<State> trans);
    // always go
    TransitionCondition(boost::weak_ptr<State> trans);
    virtual ~TransitionCondition();
    virtual boost::weak_ptr<State> execute();
    void setOwner(boost::weak_ptr<State> _owner);
    boost::weak_ptr<State> getOwner();
    boost::weak_ptr<State> getTransition();
};


class TransitionIfTimerExpired : public TransitionCondition {
protected:
    boost::shared_ptr<Timer> timer;
public:
    TransitionIfTimerExpired(boost::shared_ptr<Timer> _timer,
                             boost::weak_ptr<State> trans);
    virtual boost::weak_ptr<State> execute();
};


class YieldToParent : public TransitionCondition {
public:
    YieldToParent();
    virtual ~YieldToParent();
    virtual boost::weak_ptr<State> execute();
};


class TaskSystemState : public ContainerState {
    
private:
    using TransitionList = std::vector<boost::shared_ptr<TransitionCondition>>;
    boost::shared_ptr<TransitionList> transition_list { boost::make_shared<TransitionList>() };
    int currentActionIndex { 0 };
    
public:
    static void describeComponent(ComponentInfo &info);
    
    TaskSystemState();
    explicit TaskSystemState(const ParameterValueMap &parameters);
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
    boost::weak_ptr<State> next() override;
    void reset() override;
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> comp) override;
    
};


class TaskSystem : public ContainerState {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    TaskSystem();
    explicit TaskSystem(const ParameterValueMap &parameters);
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> comp) override;
    
};


END_NAMESPACE_MW


#endif /* TaskSystem_hpp */
