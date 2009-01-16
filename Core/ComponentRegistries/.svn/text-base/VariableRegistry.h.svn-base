#ifndef PARAMETER_REGISTRY_H
#define PARAMETER_REGISTRY_H

/**
* VariableRegistry.h
 * 
 * Description:
 * The parameter registry creates, stores and retrieves parameter identifiers.
 * There should typically only be a single global instance of this object, 
 * which manages all of the variables defined in a given experiment.  These 
 * variables might come from a variety of sources, including those hard-coded 
 * into the core framework, built into plugins for various components and
 * services, defined in user experiment definition bundles, or (eventually) 
 * defined in XML files.
 *
 * The object is basically a dictionary of Variableobjects and VariableProperties 
 * objects.  The Variableuniquely signifies a given variable, and the 
 * VariableProperties describes how the variable is to be written to the event
 * stream and how the user is allowed to interact with it via the UI. The 
 * actual data of the variable is not stored in the Variableitself, but is 
 * contained with an ScopedVariableContext object contained within an State object.
 * The Variableitself is a lexical token whose meaning (e.g. value) depends on 
 * the context (which is embodied in the experiment/protocol/block/state
				* paradigm hierarchy).
 *
 * A distinction is made between "paradigm" variables and "status" variables.
 * One important difference is that paradigm variables potentially change 
 * value as one moves up and down the paradigm hierarchy (i.e. they are 
														  * overridden by local context), while status variables are necessarily global 
 * in scope.  In addition, status variables should typically not be editable
 * by the user from the UI (rather, they reflect the status of the system). 
 * Global variables which are user-editable should be specified as 
 * experiment-level paradigm variables.  Conversely, paradigm variables should 
 * only be changed through user-interaction: the states themselves should not 
 * edit these variables.  For added flexibility, the system does not strictly 
 * enforce the read/write nature of these variables, but for good style, users
 * should observe this behavior in almost all cases.  Once defined, status and 
 * paradigm variables are get and set through exactly the same mechanism, and
 * the user need not worry about which is which.
 *
 * While this distinction is somewhat artificial (e.g. all variables could be 
												  * made to be paradigm variables without loss of basic functionality), the 
 * distinction hopefully helps organize variables into two basic, natural 
 * classes, and the dichotomy allows for slightly greater memory efficiency.
 *
 * Once defined, Params cannot be changed.
 *
 * See GenericVariable.h for more detail.
 *
 * NOTE: TODO Control Variables are not implemented.  It was the original
 * intention that all events sent over the network be data events and 
 * therefore these were meant to be control events.  They have been replaced
 * as of now by the scarab package events.  We may want to revisit this
 * in the future.
 
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 1/21/05 - Added method to generate translation codec.
 * Paul Jankunas on 4/29/05 - Added scarab package code.
 * Paul Jankunas on 5/17/05 - Removed Codec code things.
 * Paul Jankunas on 06/15/05 - Added addVariable(Variable *)
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "Event.h"
#include "GenericVariable.h"
#include "ScopedVariable.h"
#include "ConstantVariable.h"
#include "SelectionVariable.h"
#include "Timer.h"
#include "VariableProperties.h"
#include "ExpandableList.h"
#include "ScopedVariableContext.h"
#include "Scarab/scarab.h"

#include <boost/thread/mutex.hpp>

#include <vector>
//#include <hash_map.h>

#include "AnyScalar.h"
#include "ExpressionParser.h"
namespace mw {
#define CODEC_RESERVED_TAGNAME "#codec"
#define COMPONENT_CODEC_RESERVED_TAGNAME "#componentCodec"

class VariableRegistry : public stx::BasicSymbolTable {

private:	
	
	int currentUnique; // the next unique number to be added to codec.
	
	boost::mutex lock;
	
	shared_ptr<BufferManager> buffer_manager; // a given variable registry is
											   // intimately tied to a given
											   // event buffer set, since this is
											   // where all of the events related
											   // to its variables will be posted

	
protected:
    
    // TODO: Migrate to STL containers...
    
	// all variables, in codec order
	ExpandableList<Variable> master_variable_list;
	
    // for faster lookups by tag name
    map< string, shared_ptr<Variable> > master_variable_dictionary;
    
	// just the local variables
	ExpandableList<ScopedVariable> local_variable_list;
	
	// just the global variables
	ExpandableList<GlobalVariable> global_variable_list;
	
	// just the selection variables
	ExpandableList<SelectionVariable> selection_variable_list;

	// ********************************************************
	// Factory methods
	// ********************************************************
	
	// create a new entry and return one instance
	shared_ptr<ScopedVariable> addScopedVariable(weak_ptr<ScopedVariableEnvironment> env,
												  VariableProperties *p = NULL); 
	shared_ptr<GlobalVariable> addGlobalVariable(VariableProperties *p = NULL);
	shared_ptr<SelectionVariable> addSelectionVariable(VariableProperties *p = NULL);
	
	shared_ptr<ConstantVariable> addConstantVariable(Data value);
	shared_ptr<EmptyVariable> addPlaceholderVariable(VariableProperties *p = NULL);
	

public:
		
	VariableRegistry(shared_ptr<BufferManager> _buffer_manager);        
	~VariableRegistry();
	
	void addPlaceholders();
	
	// ********************************************************
	// Accessor methods
	// ********************************************************	
	
    // Get this registry's buffer manager object
    shared_ptr<BufferManager> getBufferManager(){  return buffer_manager; }
    
	// Get individual variables
	
	//shared_ptr<Variable> getVariable(const char *tagname);
	shared_ptr<Variable> getVariable(const std::string& tagname) const;
	shared_ptr<Variable> getVariable(int codecCode);
	
	std::vector<std::string> getVariableTagnames();
	
	
	bool hasVariable(const char *tagname) const;
	bool hasVariable(std::string &tagname) const;	
	
	int getNVariables();	
	
	ExpandableList<ScopedVariable> *getLocalVariables();
	ExpandableList<GlobalVariable> *getGlobalVariables();	
	// ********************************************************
	// Announcement methods
	// ********************************************************
	
	void announceLocalVariables();
	void announceGlobalVariables();
	void announceSelectionVariables();
	void announceAll();
	
	
	
	shared_ptr<ScopedVariable> createScopedVariable(weak_ptr<ScopedVariableEnvironment> env,
													 VariableProperties *p = NULL); 
													 
	shared_ptr<GlobalVariable> createGlobalVariable(VariableProperties *p = NULL);
	shared_ptr<SelectionVariable> createSelectionVariable(VariableProperties *p = NULL);
	
	shared_ptr<ConstantVariable> createConstantVariable(Data value);
	shared_ptr<Timer> createTimer(VariableProperties *p = NULL);
	shared_ptr<EmptyVariable> createPlaceholderVariable(VariableProperties *p = NULL);
	
	
	// ********************************************************
	// Codec generation
	// ********************************************************
	/**
		* Returns the codec.  DON'T EDIT IT!!  The codec dictionary
	 * is a list of key value pairs.  The keys are the codec code numbers
	 * and the values are the corresponding Variableobjects.  These Param
	 * objects hold all the essential information about a variable.
	 * This includes its default value, name, logging among others.
	 */
	Data getCodec(); 
	void update(const Data &codec);
	
	
	// ********************************************************
	// SymbolTable methods
	// From the stx expression parser
	// ********************************************************
	
	/// Return the (constant) value of a variable.
	virtual stx::AnyScalar	lookupVariable(const std::string &varname) const;
	
};

extern shared_ptr<VariableRegistry> GlobalVariableRegistry;
void initializeVariableRegistry(); // create a new global registry
}

#endif

