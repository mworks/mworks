/**
 * States.h
 *
 * Description:
 *
 * History:
 * ??/??/?? ??? - Created
 * 04/28/04 PAJ - added utility function to ListState to retrieve the list.
 * 04/29/04 PAJ - added utility functions to get SampleType and Selection.
 * Paul Janknas on 05/16/05 - fixed spacing, added functionality to state
 *                  class to add a name so users can see states by name.
 *
 * Copyright 2005 MIT. All rights reserved. 
 */

#ifndef STATES_HEADER
#define STATES_HEADER

#include <string>
#include <iostream>

#include <boost/weak_ptr.hpp>

#include "MWorksMacros.h"
#include "Utilities.h"
#include "Selection.h"
#include "ScopedVariableContext.h"
#include "Selectable.h"
#include "StandardVariables.h"
#include "ScopedVariableEnvironment.h"
#include "Announcers.h"


BEGIN_NAMESPACE_MW


class Experiment;


class State : public Component {
    
private:
    // who immediately owns this state? (e.g. a block)
    boost::weak_ptr<State> parent;
    
    // what experiment does this state belong to
    boost::weak_ptr<Experiment> experiment;
    
    bool interruptible { true };
    
protected:
    template <typename T>
    boost::shared_ptr<T> clone() {
        auto new_state = Component::clone<T>();
        
        new_state->setParent(getParent());
        new_state->setExperiment(getExperiment());
        new_state->setInterruptible(interruptible);
        
        return new_state;
    }
    
public:
    // The end state is a singleton instance that, if returned by next(), indicates
    // that there are no additional states to execute
    static const boost::shared_ptr<State> & getEndState();
    
    static void describeComponent(ComponentInfo &info);
    
    State();
    explicit State(const Map<ParameterValue> &parameters);
    
    virtual void action();
    
    /**
     * Returns the next state.
     */
    virtual boost::weak_ptr<State> next();
    
    boost::shared_ptr<State> getParent() const { return parent.lock(); }
    void setParent(const boost::shared_ptr<State> &newparent) { parent = newparent; }
    
    virtual void updateHierarchy();
    
    virtual void reset() { }
    
    boost::shared_ptr<Experiment> getExperiment() const { return experiment.lock(); }
    void setExperiment(const boost::shared_ptr<Experiment> &_experiment) { experiment = _experiment; }
    
    virtual boost::shared_ptr<ScopedVariableContext> getLocalScopedVariableContext() const;
    
    virtual void updateCurrentScopedVariableContext();
    
    void setInterruptible(bool _interruptible) { interruptible = _interruptible; }
    bool isInterruptible() const;
    
    /**
     * Aliases for setTag/getTag (for backwards compatibility)
     */
    void setName(const std::string &n) { setTag(n); }
    const std::string & getName() const { return getTag(); }
    
};


class ContainerState : public State {
    
private:
    boost::shared_ptr<ScopedVariableContext> local_variable_context;
    
    // Shared pointer to a vector of pointers to states
    // (we need a pointer, rather than a bare object so that multiple
    //  aliases to the same underlying state can share the same list)
    boost::shared_ptr<std::vector<boost::shared_ptr<State>>> list { new std::vector<boost::shared_ptr<State>> }; // the list of states
    
    void requestVariableContext();
    
protected:
    bool accessed { false };
    
    template <typename T>
    boost::shared_ptr<T> clone() {
        auto new_state = State::clone<T>();
        new_state->list = list;
        return new_state;
    }
    
    void setLocalScopedVariableContext(const boost::shared_ptr<ScopedVariableContext> &c) {
        local_variable_context = c;
    }
    
public:
    static const std::string INTERRUPTIBLE;
    
    static void describeComponent(ComponentInfo &info);
    
    ContainerState();
    explicit ContainerState(const Map<ParameterValue> &parameters);
    
    const std::vector<boost::shared_ptr<State>> & getList() const { return *list; }
    
    // Subclasses must decide for themselves how child states are traversed
    virtual boost::weak_ptr<State> next() = 0;
    
    void updateHierarchy() override;
    
    void reset() override;
    
    boost::shared_ptr<ScopedVariableContext> getLocalScopedVariableContext() const override;
    
    void updateCurrentScopedVariableContext() override;
    
    // mw::Component methods
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<mw::Component> child) override;
    
};


class ListState : public ContainerState, public Selectable {
    
private:
    SelectionType selection_type;
    long nsamples;
    SampleType sampling_method;
    
protected:
    bool hasMoreChildrenToRun() const { return selection && !(selection->isFinished()); }
    
    template <typename T>
    boost::shared_ptr<T> clone() {
        auto new_state = ContainerState::clone<T>();
        
        new_state->selection_type = selection_type;
        new_state->nsamples = nsamples;
        new_state->sampling_method = sampling_method;
        
        if (auto sel = getSelectionClone()) {
            new_state->attachSelection(sel);
        }
        
        return new_state;
    }
    
public:
    static const std::string SELECTION;
    static const std::string NSAMPLES;
    static const std::string SAMPLING_METHOD;
    
    static void describeComponent(ComponentInfo &info);
    
    ListState();
    explicit ListState(const Map<ParameterValue> &parameters);
    
    // State methods
    boost::weak_ptr<State> next() override;
    void reset() override;
    
    // Selectable methods
    int getNItems() override { return int(getList().size()); }
    
    void finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg) override;
    
};


END_NAMESPACE_MW


#endif
