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
using namespace std;

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
    
    /**
     * Destructor.
     */
    virtual ~State() { }
    
    virtual void requestVariableContext();
    
    virtual shared_ptr<mw::Component> createInstanceObject();
    
    void setParameters(std::map<std::string, std::string> parameters,
                       ComponentRegistry *reg);
    
    virtual void action();
    
    /**
     * Update the status of the state, e.g. to update a block after
     * a trial is done
     */
    virtual void update() { }
    
    /**
     * Returns the next state.
     */
    virtual weak_ptr<State> next();
    
    virtual void setParent(shared_ptr<State> newparent);
    shared_ptr<State> getParent() const { return parent.lock(); }
    
    virtual void updateHierarchy();
    
    virtual void reset() { }
	
    //        virtual void announceIdentity();
    
    //void inheritLocalScopedVariableContext(ScopedVariableContext *newinfo);
    shared_ptr<Experiment> getExperiment() const { return experiment.lock(); }
    void setExperiment(shared_ptr<Experiment> _experiment) { experiment = _experiment; }
    
    virtual weak_ptr<ScopedVariableEnvironment> getScopedVariableEnvironment();
    virtual void setScopedVariableEnvironment(weak_ptr<ScopedVariableEnvironment> _env);
    
    virtual void updateCurrentScopedVariableContext();
    void setLocalScopedVariableContext(shared_ptr<ScopedVariableContext> c);
    
    shared_ptr<ScopedVariableContext> getLocalScopedVariableContext();
    
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
    
};


class ContainerState : public State {
    
protected:
    // Shared pointer to a vector of pointers to states
    // (we need a pointer, rather than a bare object so that multiple 
    //  aliases to the same underlying state can share the same list)
    shared_ptr< vector< shared_ptr<State> > > list; // the list of states
    bool accessed;
	
    template <typename T>
    shared_ptr<T> clone() {
        shared_ptr<T> new_state(State::clone<T>());
        new_state->setList(list);
        return new_state;
    }
    
public:
	
    
    ContainerState();
    //mContainerState(Experiment *exp);
    //		ContainerState(State *parent);
    virtual ~ContainerState();
	
    virtual shared_ptr<mw::Component> createInstanceObject();
	
    shared_ptr< vector< shared_ptr<State> > >  getList();
    void setList(shared_ptr< vector< shared_ptr<State> > > newlist);
    
    virtual void updateHierarchy();
    
    // mw::Component methods
    virtual void addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistry *reg,
                          shared_ptr<mw::Component> child){
        
        shared_ptr<State> state = dynamic_pointer_cast<State, mw::Component>(child);
        
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
    
    bool has_more_children_to_run;
    
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
    //mListState(State *newparent);
    virtual ~ListState();
	
    // Clone this object, but create new local variable context (aka scope)
    virtual shared_ptr<mw::Component> createInstanceObject();
	
    // ContainerState methods
    virtual int getNChildren();
    //virtual void addState(shared_ptr<State> newstate);
    //		virtual void addState(int index, shared_ptr<State> newstate);
    
    virtual void update();
    
    // State methods
    virtual weak_ptr<State> next();
    virtual void reset();
    
    // Selectable methods
    virtual int getNItems();
    
    void finalize(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg);
	
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
