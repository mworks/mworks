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


//#define announceState(statename) fprintf(stderr, "=> Current State: %s (%d)\n", statename, (int)this); fflush(stderr);
#define announceState(x);

class State : public ScopedVariableEnvironment, public Component {
    
private:
    // who immediately owns this state? (e.g. a block)
    weak_ptr<State> parent;	
    
    // what experiment does this state belong to
    weak_ptr<Experiment> experiment;	
    weak_ptr<ScopedVariableEnvironment> environment;  // TODO: what is this?
    
    shared_ptr<ScopedVariableContext> local_variable_context;
    
    // description of the state for user display
    std::string description;
    
    bool interruptible;
    
protected:
    template <typename T>
    shared_ptr<T> clone() {
        shared_ptr<T> new_state(Component::clone<T>());
        
        new_state->setParent(getParent());
        new_state->setExperiment(getExperiment());
        new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
        new_state->setDescription(getDescription());
        new_state->setInterruptible(interruptible);
        
        return new_state;
    }
    
public:
    /**
     * Default Constructor.
     */
    State();
    
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
    
    shared_ptr<ScopedVariableEnvironment> getScopedVariableEnvironment() const { return environment.lock(); }
    void setScopedVariableEnvironment(weak_ptr<ScopedVariableEnvironment> _env) { environment = _env; }
    
    shared_ptr<ScopedVariableContext> getLocalScopedVariableContext() const { return local_variable_context; }
    void setLocalScopedVariableContext(shared_ptr<ScopedVariableContext> c) {
        local_variable_context = c;
        setCurrentContext(local_variable_context);
    }
    
    virtual void updateCurrentScopedVariableContext();
    
    void setInterruptible(bool _interruptible){ interruptible = _interruptible; }
    bool isInterruptible() const;
    
    /**
     * Aliases for setTag/getTag (for backwards compatibility)
     */
    void setName(const std::string &n) { setTag(n); }
    const std::string& getName() const { return getTag(); }
    
    /**
     * Sets the description for this state.
     */
    void setDescription(const std::string &d) { description = d; }
    
    /**
     * Returns the description of this state or an empty string if no description
     * has been set.
     */
    const std::string& getDescription() const { return description; }
    
    void setParameters(std::map<std::string, std::string> parameters, ComponentRegistry *reg);
    
};


class ContainerState : public State {
    
private:
    // Shared pointer to a vector of pointers to states
    // (we need a pointer, rather than a bare object so that multiple 
    //  aliases to the same underlying state can share the same list)
    shared_ptr< vector< shared_ptr<State> > > list; // the list of states
    
protected:
    bool accessed;
	
    template <typename T>
    shared_ptr<T> clone() {
        shared_ptr<T> new_state(State::clone<T>());
        new_state->list = list;
        return new_state;
    }
    
public:
    ContainerState();
	
    const vector< shared_ptr<State> >& getList() const { return *list; }
    
    // Subclasses must decide for themselves how child states are traversed
    virtual weak_ptr<State> next() = 0;
    
    virtual void updateHierarchy();
    
    virtual void reset();
    
    // mw::Component methods
    virtual void addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistry *reg,
                          shared_ptr<mw::Component> child){
        
        shared_ptr<State> state = boost::dynamic_pointer_cast<State, mw::Component>(child);
        
        if(state == NULL){
            // TODO: better throw
            throw SimpleException("Attempt to add non-paradigm component object as child of a paradigm component");
        }
        
        list->push_back(state);
        state->setParent(component_shared_from_this<State>());
        state->updateHierarchy();
        
    }
    
};


class ListState : public ContainerState, public Selectable {
    
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
	
    ListState();
    
    // State methods
    virtual weak_ptr<State> next();
    virtual void reset();
    
    // Selectable methods
    virtual int getNItems() { return int(getList().size()); }
    
    virtual void finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg);
	
};

template <class T>
class ListStateFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> 
    createObject(std::map<std::string, std::string> parameters,
                 ComponentRegistry *reg){
        
		shared_ptr<T> newListState = shared_ptr<T>(new T());
		newListState->setParameters(parameters, reg);
		
		return newListState;
	}
};


END_NAMESPACE_MW


#endif
