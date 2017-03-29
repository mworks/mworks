#ifndef PARAMETER_REGISTRY_H
#define PARAMETER_REGISTRY_H

/**
 * VariableRegistry.h
 *
 * Copyright (c) 2002-2016 MIT. All rights reserved.
 */

#include "Event.h"
#include "EventBuffer.h"
#include "GlobalVariable.h"
#include "ScopedVariable.h"
#include "SelectionVariable.h"
#include "Timer.h"
#include "VariableProperties.h"
#include "ExpandableList.h"
#include "ScopedVariableContext.h"

#include <boost/thread/mutex.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

#include "ExpressionParser.h"

using std::map;
using std::string;
using std::vector;


BEGIN_NAMESPACE_MW


#define CODEC_RESERVED_TAGNAME "#codec"
#define COMPONENT_CODEC_RESERVED_TAGNAME "#componentCodec"
    
class VariableRegistry : public stx::BasicSymbolTable {

private:	
	
	int current_unique_code; // the next unique number to be added to codec.
	
    // Declare lock mutable so that it can be acquired in const methods
	mutable boost::mutex lock;
	
	shared_ptr<EventBuffer> event_buffer; // a given variable registry is
											   // intimately tied to a given
											   // event buffer, since this is
											   // where all of the events related
											   // to its variables will be posted

	
protected:
    
	// all variables, in codec order
	vector< shared_ptr<Variable> > master_variable_list;
	
    // for faster lookups by tag name
    map< string, shared_ptr<Variable> > master_variable_dictionary;
    
	// just the local variables
	vector< shared_ptr<Variable> > local_variable_list;
	
	// just the global variables
	vector< shared_ptr<Variable> > global_variable_list;
	
	// just the selection variables
	vector< shared_ptr<Variable> > selection_variable_list;

	// ********************************************************
	// Factory methods
	// ********************************************************
	
	// create a new entry and return one instance
	shared_ptr<ScopedVariable> addScopedVariable(boost::weak_ptr<ScopedVariableEnvironment> env,
												 const VariableProperties &p);
	shared_ptr<GlobalVariable> addGlobalVariable(const VariableProperties &p);
	shared_ptr<SelectionVariable> addSelectionVariable(const VariableProperties &p);
	
	shared_ptr<ConstantVariable> addConstantVariable(Datum value);
	

public:
		
	VariableRegistry(shared_ptr<EventBuffer> _event_buffer);        
	~VariableRegistry();
	
    void reset();
    
	
	// ********************************************************
	// Accessor methods
	// ********************************************************	
	
    // Get this registry's buffer manager object
    shared_ptr<EventBuffer> getEventBuffer(){  return event_buffer; }
    
	// Get individual variables
	shared_ptr<Variable> getVariable(const std::string& tagname) const;
	shared_ptr<Variable> getVariable(int codecCode);
	vector<string> getVariableTagNames();
	
    vector< shared_ptr<Variable> > getGlobalVariables(){ return global_variable_list; }
	
	bool hasVariable(const char *tagname) const;
	bool hasVariable(std::string &tagname) const;	
	
	int getNVariables();	
	    
    
	// ********************************************************
	// Announcement methods
	// ********************************************************
	
	void announceLocalVariables();
	void announceGlobalVariables();
	void announceSelectionVariables();
	void announceAll();
	
	// ********************************************************
	// Creation methods
	// ********************************************************
	
	shared_ptr<ScopedVariable> createScopedVariable(boost::weak_ptr<ScopedVariableEnvironment> env,
                                                    const VariableProperties &p);
													 
	shared_ptr<GlobalVariable> createGlobalVariable(const VariableProperties &p);
	shared_ptr<SelectionVariable> createSelectionVariable(const VariableProperties &p);
	
	shared_ptr<ConstantVariable> createConstantVariable(Datum value);
	shared_ptr<Timer> createTimer(const VariableProperties &p);
	
	
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
    Datum generateCodecDatum();
    Datum generateReverseCodecDatum();
    map<int, string> generateCodecMap();
    map<string, int> generateReverseCodecMap();
    
    
	void updateFromCodecDatum(const Datum &codec);
	
	
	// ********************************************************
	// SymbolTable methods
	// From the stx expression parser
	// ********************************************************
	
	/// Return the (constant) value of a variable.
	Datum lookupVariable(const std::string &varname) const override;
	
};

extern shared_ptr<VariableRegistry> global_variable_registry;
//void initializeVariableRegistry(); // create a new global registry


END_NAMESPACE_MW


#endif

