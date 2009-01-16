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

#include "Utilities.h"
#include "Selection.h"
#include "ScopedVariableContext.h"
#include "Clonable.h"
#include "Selectable.h"
#include "StandardVariables.h"
#include "ScopedVariableEnvironment.h"
#include <string>
#include <iostream>
#include "Announcers.h"
namespace mw {
class Experiment;

using namespace std;

//#define announceState(statename) fprintf(stderr, "=> Current State: %s (%d)\n", statename, (int)this); fflush(stderr);
#define announceState(x);

class State : public ScopedVariableEnvironment, 
			   public Clonable, 
			   public mw::Component,
			   public ComponentFactory {
	protected:
        // who immediately owns this state? (e.g. a block)
        weak_ptr<State> parent;	
        
		// what experiment does this state belong to
        weak_ptr<Experiment> experiment;	
		weak_ptr<ScopedVariableEnvironment> environment;  // TODO: what is this?
        
		shared_ptr<ScopedVariableContext> local_variable_context;
        
		// name of the state for user display
        std::string name;
        std::string description;
		
		bool interruptible;

	public:
	
		
	
        /**
         * Default Constructor.
         */
        State();
		//mState(Experiment *exp);
	
        /**
         * Constructor to create a state with its parent state set to
         * 'newparent'
         */
		//mState(State *newparent);
        
		
        /**
         * Destructor.
         */
        virtual ~State();
		
		virtual void requestVariableContext();

		virtual shared_ptr<mw::Component> createInstanceObject();
		virtual void *scopedClone(); // deprecated

		void setParameters(std::map<std::string, std::string> parameters,
											mwComponentRegistry *reg);

        virtual void action();

        /**
         * Update the status of the state, e.g. to update a block after
         * a trial is done
         */
        virtual void update(); 
    
        /**
         * Returns the next state.
         */
        virtual weak_ptr<State> next();

        virtual void setParent(weak_ptr<State> newparent);
        weak_ptr<State> getParent();
		
		
        virtual void updateHierarchy();

        virtual void reset();
	
//        virtual void announceIdentity();
                
        //void inheritLocalScopedVariableContext(ScopedVariableContext *newinfo);
        virtual weak_ptr<Experiment> getExperiment();
        virtual void setExperiment(weak_ptr<Experiment> _experiment);
		
		virtual weak_ptr<ScopedVariableEnvironment> State::getScopedVariableEnvironment();
        virtual void setScopedVariableEnvironment(weak_ptr<ScopedVariableEnvironment> _env);

		virtual void updateCurrentScopedVariableContext();
		void setLocalScopedVariableContext(shared_ptr<ScopedVariableContext> c);
        
		shared_ptr<ScopedVariableContext> getLocalScopedVariableContext();

		// this is a semi-shallow copy that just keeps pointers to most things
		// except the variable context, for which it gets a new object that points
		// through at the original
		State *getStateInstance();

		void setInterruptible(bool _interruptible){ interruptible = _interruptible; }
		bool isInterruptible(){ return interruptible; }
		
        /**
         * Sets the user defined name of the state.  Originally it will be
         * NULL.
         */
        void setName(std::string);
		void setName(const char *n);
        
        /**
         * Returns the user defined name or NULL if no name has been defined.
         */
        std::string getName();
        
        /**
         * Sets the description for this state.
         */
        void setDescription(std::string);
        
        /**
         * Returns the description of this state or NULL if no description
         * has been set.
         */
        std::string getDescription();
        
};


class StateReference : public State {

	protected:
		State *state;
	
	public:
	
		StateReference(State *ref);
		
		virtual void action();
		virtual weak_ptr<State> next();
		
		virtual void update();

};

class ContainerState : public State {

	protected:
		// Shared pointer to a vector of pointers to states
		// (we need a pointer, rather than a bare object so that multiple 
		//  aliases to the same underlying state can share the same list)
		shared_ptr< vector< shared_ptr<State> > > list; // the list of states
		bool accessed;
	
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
								mwComponentRegistry *reg,
								shared_ptr<mw::Component> child){
			
			std::cerr << "addChild start" << std::endl;
			shared_ptr<State> state = dynamic_pointer_cast<State, mw::Component>(child);

			if(state == NULL){
				// TODO: better throw
				throw SimpleException("Attempt to add non-paradigm component object as child of a paradigm component");
			}
			
			list->push_back(state);
			state->setParent(getSelfPtr<State>());
			state->updateHierarchy();
			
			std::cerr << "addChild end" << std::endl;
		}

};


class ListState : public ContainerState, public Selectable {
	protected:
		
		bool has_more_children_to_run;

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
												mwComponentRegistry *reg);
	
};

template <class T>
class ListStateFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> 
		createObject(std::map<std::string, std::string> parameters,
					mwComponentRegistry *reg){
					
		shared_ptr<T> newListState = shared_ptr<T>(new T());
		newListState->setParameters(parameters, reg);
		
		return newListState;
	}
};
}

#endif
