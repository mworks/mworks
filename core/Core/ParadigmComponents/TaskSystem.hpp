//
//  TaskSystem.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/20/23.
//

#ifndef TaskSystem_hpp
#define TaskSystem_hpp

#include "ComponentInfo.h"
#include "ParameterValue.h"
#include "StandardComponentFactory.h"
#include "States.h"
#include "Timer.h"


BEGIN_NAMESPACE_MW


class TaskSystemState;
class Transition;


class TaskSystem : public ContainerState {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    TaskSystem();
    explicit TaskSystem(const ParameterValueMap &parameters);
    
    boost::shared_ptr<Component> createInstanceObject() override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    void reset() override;
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> comp) override;
    
    void setNextState(const boost::shared_ptr<TaskSystemState> &state) { nextState = state; }
    
private:
    boost::shared_ptr<TaskSystemState> nextState;
    
};


class TaskSystemState : public ContainerState {
    
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
    
private:
    using TransitionList = std::vector<boost::shared_ptr<Transition>>;
    boost::shared_ptr<TransitionList> transition_list { boost::make_shared<TransitionList>() };
    
    int currentActionIndex { 0 };
    
};


class Transition : public Component {
    
public:
    static const std::string TARGET;
    
    explicit Transition(const ParameterValueMap &parameters);
    
    boost::shared_ptr<TaskSystemState> getTarget() const { return weakTarget.lock(); }
    void setTarget(const boost::shared_ptr<TaskSystemState> &target) { weakTarget = target; }
    
    virtual bool evaluate() = 0;
    
private:
    boost::weak_ptr<TaskSystemState> weakTarget;
    
};


class DirectTransition : public Transition {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using Transition::Transition;
    
    bool evaluate() override;
    
};


class ConditionalTransition : public Transition {
    
public:
    static const std::string CONDITION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ConditionalTransition(const ParameterValueMap &parameters);
    
    bool evaluate() override;
    
private:
    const VariablePtr condition;
    
};


class TimerExpiredTransition : public Transition {
    
public:
    static const std::string TIMER;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TimerExpiredTransition(const ParameterValueMap &parameters);
    
    bool evaluate() override;
    
private:
    const boost::shared_ptr<Timer> timer;
    
};


class GotoTransition : public Transition {
    
public:
    static const std::string WHEN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit GotoTransition(const ParameterValueMap &parameters);
    
    bool evaluate() override;
    
private:
    const VariablePtr when;
    
};


class YieldTransition : public Transition {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using Transition::Transition;
    
    bool evaluate() override;
    
};


template<typename TransitionType>
class StandardTransitionFactory : public StandardComponentFactory<TransitionType> {
    
public:
    ComponentPtr createObject(ComponentFactory::StdStringMap parameters, ComponentRegistryPtr reg) override {
        auto obj = StandardComponentFactory<TransitionType>::createObject(parameters, reg);
        if (auto trans = boost::dynamic_pointer_cast<Transition>(obj)) {
            auto &targetName = parameters[Transition::TARGET];
            auto target = reg->getObject<TaskSystemState>(targetName, parameters["parent_scope"]);
            if (!target) {
                throw MissingReferenceException(Transition::TARGET, targetName);
            }
            trans->setTarget(target);
        }
        return obj;
    }
    
};


END_NAMESPACE_MW


#endif /* TaskSystem_hpp */
