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

using boost::weak_ptr;


BEGIN_NAMESPACE_MW


class Experiment;


class State : public Component {
    
private:
    // who immediately owns this state? (e.g. a block)
    weak_ptr<State> parent;	
    
    // what experiment does this state belong to
    weak_ptr<Experiment> experiment;	
    
    shared_ptr<ScopedVariableContext> local_variable_context;
    
    bool interruptible { true };
    
protected:
    template <typename T>
    shared_ptr<T> clone() {
        shared_ptr<T> new_state(Component::clone<T>());
        
        new_state->setParent(getParent());
        new_state->setExperiment(getExperiment());
        new_state->setInterruptible(interruptible);
        
        return new_state;
    }
    
public:
    static void describeComponent(ComponentInfo &info);
    
    State();
    explicit State(const Map<ParameterValue> &parameters);
    
    virtual void requestVariableContext();
    
    virtual void action();
    
    /**
     * Returns the next state.
     */
    virtual weak_ptr<State> next();
    
    shared_ptr<State> getParent() const { return parent.lock(); }
    void setParent(shared_ptr<State> newparent) { parent = newparent; }
    
    virtual void updateHierarchy();
    
    virtual void reset() { }
	
    shared_ptr<Experiment> getExperiment() const { return experiment.lock(); }
    void setExperiment(shared_ptr<Experiment> _experiment) { experiment = _experiment; }
    
    shared_ptr<ScopedVariableContext> getLocalScopedVariableContext() const { return local_variable_context; }
    void setLocalScopedVariableContext(shared_ptr<ScopedVariableContext> c) {
        local_variable_context = c;
    }
    
    virtual void updateCurrentScopedVariableContext();
    
    void setInterruptible(bool _interruptible){ interruptible = _interruptible; }
    bool isInterruptible() const;
    
    /**
     * Aliases for setTag/getTag (for backwards compatibility)
     */
    void setName(const std::string &n) { setTag(n); }
    const std::string& getName() const { return getTag(); }
    
};


class ContainerState : public State {
    
private:
    // Shared pointer to a vector of pointers to states
    // (we need a pointer, rather than a bare object so that multiple 
    //  aliases to the same underlying state can share the same list)
    shared_ptr< vector< shared_ptr<State> > > list { new vector< shared_ptr<State> > }; // the list of states
    
protected:
    bool accessed { false };
	
    template <typename T>
    shared_ptr<T> clone() {
        shared_ptr<T> new_state(State::clone<T>());
        new_state->list = list;
        return new_state;
    }
    
public:
    static const std::string INTERRUPTIBLE;
    
    static void describeComponent(ComponentInfo &info);
    
    ContainerState();
    explicit ContainerState(const Map<ParameterValue> &parameters);
	
    const vector< shared_ptr<State> >& getList() const { return *list; }
    
    // Subclasses must decide for themselves how child states are traversed
    virtual weak_ptr<State> next() = 0;
    
    void updateHierarchy() override;
    
    void reset() override;
    
    // mw::Component methods
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<mw::Component> child) override;
    
};


class ListState : public ContainerState, public Selectable {
    
private:
    const SelectionType selection_type;
    const long nsamples;
    const SampleType sampling_method;
    
protected:
    bool hasMoreChildrenToRun() { return selection && !(selection->isFinished()); }
    
    template <typename T>
    shared_ptr<T> clone() {
        shared_ptr<T> new_state(ContainerState::clone<T>());
        
        shared_ptr<Selection> sel = getSelectionClone();
        if (sel) {
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
    weak_ptr<State> next() override;
    void reset() override;
    
    // Selectable methods
    int getNItems() override { return int(getList().size()); }
    
    void finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg) override;
	
};


END_NAMESPACE_MW


#endif
