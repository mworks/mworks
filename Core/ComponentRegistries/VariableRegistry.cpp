/**
* VariableRegistry.cpp
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 4/29/05 - Added the experiment package codec to package.
 *                          Also added the constant code to codec package.
 * Paul Jankunas on 5/17/05 - Removed codec code, it now is stored in a 
 *                                                          scarab package.
 * Paul Jankunas on 06/15/05 - Fixed ScarabDatum constructor, added function
 *      addVariable that takes a Variable arg, fixed a bug in getVariable
 *      that was returning empty variables for 0 index.
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */

#include "VariableRegistry.h"
#include "Utilities.h"
#include "Experiment.h"
#include "EventBuffer.h"
using namespace mw;

VariableRegistry::VariableRegistry(shared_ptr<BufferManager> _buffer_manager) {
	buffer_manager = _buffer_manager;
	

	addPlaceholders();
	
}


void VariableRegistry::addPlaceholders(){
	Data defaultInt(0L);
	VariableProperties componentCodecProps(&defaultInt,       // constructor to create default
											COMPONENT_CODEC_RESERVED_TAGNAME, 
											"component codec",
											"Reserved for the mw::Component Codec", 
											M_NEVER,M_WHEN_CHANGED,	// never user edit, log when changed
											true, 
											false, 
											M_STRUCTURED, "");
	addPlaceholderVariable(&componentCodecProps);  // view   
	
	VariableProperties codecProps(&defaultInt,       // constructor to create default
								   CODEC_RESERVED_TAGNAME, 
								   "codec",
								   "Reserved for the Codec", 
								   M_NEVER,M_WHEN_CHANGED,	// never user edit, log when changed
								   true, 
								   false, 
								   M_STRUCTURED, "");
	addPlaceholderVariable(&codecProps);  // view   
	
	VariableProperties terminationProps(&defaultInt,       // constructor to create default
										 "#termination", 
										 "termination",
										 "Deprecated", 
										 M_NEVER,M_WHEN_CHANGED,	// never user edit, log when changed
										 true, 
										 false, 
										 M_STRUCTURED, "");
	addPlaceholderVariable(&terminationProps);  // view  
}
	
VariableRegistry::~VariableRegistry() {
	
	// assumes that all stored variables are in one of the other lists
	// and will get deleted from there.
	master_variable_list.releaseElements();
}

void VariableRegistry::update(const Data &codec) {
	mprintf(M_SYSTEM_MESSAGE_DOMAIN,
			"Received new codec, updating variable registry.");
	
    if(!codec.isDictionary()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Invalid codec received.  Registry is unchanged.");
		return;
	}
 	
	boost::mutex::scoped_lock s_lock(lock);
	
	master_variable_list.clear();
	
	// add the placeholders
	addPlaceholders();
	
	
	//////////////////////////////////////////////////////////////////
	// now add what's in the codec 
	
	ScarabDatum *datum = codec.getScarabDatum();
	
	ScarabDatum ** keys = scarab_dict_keys(datum);
	int size = datum->data.dict->tablesize;
	
	
	
	int maxCodecCode = -1;
	// find the maximum codec value
	for(int i = 0; i < size; ++i) {
		if(keys[i]) {
			long long code = keys[i]->data.integer;
			maxCodecCode = (maxCodecCode < code) ? code : maxCodecCode;
		}
	}
		
	// add each variable in order to the registry
	for(int i = M_MAX_RESERVED_EVENT_CODE; i<=maxCodecCode; ++i) {
		ScarabDatum *key = scarab_new_integer(i);
		ScarabDatum *serializedVariable = scarab_dict_get(datum, key);
		scarab_free_datum(key);
		
		if(serializedVariable) {
			if(serializedVariable->type != SCARAB_DICT) {
				// these must be  placeholder datums in the package
				// that we should ignore.
				mwarning(M_SYSTEM_MESSAGE_DOMAIN,
						 "Bad variable received from network stream");
				continue;
			}
						
			VariableProperties *props = 
				new VariableProperties(serializedVariable);
			
			if(props == NULL){
				mwarning(M_SYSTEM_MESSAGE_DOMAIN,
						 "Bad variable received from network stream");
				continue;
			}
			
			shared_ptr<GlobalVariable> newvar(new GlobalVariable(props));
			newvar->setCodecCode(i); //necessary? .. Yup
			
			master_variable_list.addReference(newvar);
            
            std::string tag = newvar->getVariableName();
            if(!tag.empty()){
                master_variable_dictionary[tag] = newvar;
            }
		}
	}
}


void VariableRegistry::announceLocalVariables(){
	boost::mutex::scoped_lock s_lock(lock);
	
	for(int i = 0; i < local_variable_list.getNElements(); i++){
		local_variable_list[i]->announce();
	}
}

void VariableRegistry::announceGlobalVariables(){
	boost::mutex::scoped_lock s_lock(lock);
		
	for(int i = 0; i < global_variable_list.getNElements(); i++){
		global_variable_list[i]->announce();
	}
	
}

void VariableRegistry::announceSelectionVariables(){
	boost::mutex::scoped_lock s_lock(lock);
		
	for(int i = 0; i < selection_variable_list.getNElements(); i++){
		selection_variable_list[i]->announce();
	}
}

void VariableRegistry::announceAll() {
	boost::mutex::scoped_lock s_lock(lock);
	for(int i = 0; i < master_variable_list.getNElements(); i++){
		master_variable_list[i]->announce();
	}	
}


// There is potential room for speed up with a hash_map, though this would
// require const char * machinations, as hash_map cannot have string keys
shared_ptr<Variable> VariableRegistry::getVariable(const std::string& tagname) const{
	boost::mutex::scoped_lock s_lock((boost::mutex&)lock);
	
    map< string, shared_ptr<Variable> >::const_iterator it;
    it = master_variable_dictionary.find(tagname);
    if(it == master_variable_dictionary.end()){
        return shared_ptr<Variable>();
    } else {
        return it->second;
    }
        
    // This one line is how it would have been written if the stx parser guy 
    // didn't have a const-correctness stick up his butt
    //return master_variable_dictionary[tagname];
}


shared_ptr<Variable> VariableRegistry::getVariable(int codec_code) {
	
	shared_ptr<Variable> var;	

	boost::mutex::scoped_lock s_lock((boost::mutex&)lock);
	
    // DDC: removed what was this for?
	//mExpandableList<Variable> list(master_variable_list);
	
	if(codec_code < 0 || codec_code > master_variable_list.getNElements()){
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Attempt to get an invalid variable (code: %d)",
			   codec_code);
		
		var = shared_ptr<Variable>();
	} else {
        // DDC: removed copying.  What was that for?
		//var = list[codec_code];
        var = master_variable_list.getElement(codec_code);
	}
	
	return var;
}

std::vector<std::string> VariableRegistry::getVariableTagnames() {
	boost::mutex::scoped_lock s_lock(lock);
	
	std::vector<std::string> tagnames;
	
	for(int i = 0; i < master_variable_list.getNElements(); i++){		
		shared_ptr<Variable> var = master_variable_list[i];		
		if(var) {
			tagnames.push_back(var->getVariableName());
		}
	}
	
	return tagnames;	
}

bool VariableRegistry::hasVariable(const char *tagname) const { 
	return (getVariable(tagname) != NULL); 
}
bool VariableRegistry::hasVariable(std::string &tagname) const { 
	return (getVariable(tagname) != NULL); 
}


int VariableRegistry::getNVariables() { 
	return master_variable_list.getNElements();
}


ExpandableList<ScopedVariable> *VariableRegistry::getLocalVariables() { 
	return &local_variable_list;
}

ExpandableList<GlobalVariable> *VariableRegistry::getGlobalVariables() { 
	return &global_variable_list;
}



shared_ptr<ScopedVariable> VariableRegistry::addScopedVariable(weak_ptr<ScopedVariableEnvironment> env,
																 VariableProperties *props) {
    // create a new entry and return one instance
    if(props == NULL) {
        // TODO warn
		throw SimpleException("Failed to add scoped variable to registry");
    }
	
    int variable_context_index = -1;
	int codec_code = -1;
	
	VariableProperties *props_copy = new VariableProperties(*props);
	
    shared_ptr<ScopedVariable> new_variable(new ScopedVariable(props_copy));
	//GlobalCurrentExperiment->addVariable(new_variable);
    
	codec_code = master_variable_list.addReference(new_variable);
    
    std::string tag = new_variable->getVariableName();
    if(!tag.empty()){
        master_variable_dictionary[tag] = new_variable;
	}
    
	variable_context_index = local_variable_list.addReference(new_variable);	
	
	new_variable->setContextIndex(variable_context_index);
	new_variable->setLogging(M_WHEN_CHANGED);
	new_variable->setCodecCode(codec_code);
	new_variable->setBufferManager(buffer_manager);
	
	if(!env.expired()){
		shared_ptr<ScopedVariableEnvironment> env_shared(env);
		env_shared->addVariable(new_variable);
	}
	
	return new_variable;
}


shared_ptr<GlobalVariable> VariableRegistry::addGlobalVariable(VariableProperties *props){
	
	if(props == NULL){
		// TODO: warn
		throw SimpleException("Failed to add global variable to registry");
	}
	
	VariableProperties *copy = new VariableProperties(*props);
	shared_ptr<GlobalVariable> returnref(new GlobalVariable(copy));
	
	int codec_code = master_variable_list.addReference(returnref);
	
    std::string tag = returnref->getVariableName();
    if(!tag.empty()){
        master_variable_dictionary[tag] = returnref;
	}
    
    global_variable_list.addReference(returnref);
	
	returnref->setCodecCode(codec_code);
	returnref->setBufferManager(buffer_manager);
	
	return returnref;
}

shared_ptr<ConstantVariable> VariableRegistry::addConstantVariable(Data value){
	
	
	shared_ptr<ConstantVariable> returnref(new ConstantVariable(value));
	returnref->setCodecCode(-1);
	returnref->setBufferManager(buffer_manager);
	
	return returnref;
}

shared_ptr<Timer> VariableRegistry::createTimer(VariableProperties *props) {
	VariableProperties *props_copy;
	
	if(props != NULL){
		props_copy = new VariableProperties(*props);	
	} else {
		props_copy = NULL;
	}
	
	shared_ptr<Timer> new_timer(new Timer(props_copy));
	
//	int codec_code = master_variable_list.addReference(new_timer);
	
	std::string tag = new_timer->getVariableName();
	if(!tag.empty()){
        master_variable_dictionary[tag] = new_timer;
	}
	
	new_timer->setCodecCode(-1);
	new_timer->setBufferManager(buffer_manager);	
	
	return new_timer;
}

shared_ptr<EmptyVariable> VariableRegistry::addPlaceholderVariable(VariableProperties *props){
	
	VariableProperties *props_copy;
	
	if(props != NULL){
		props_copy = new VariableProperties(*props);	
	} else {
		props_copy = NULL;
	}
	
	
	shared_ptr<EmptyVariable> returnref(new EmptyVariable(props_copy));
	
	int codec_code = master_variable_list.addReference(returnref);
	
	returnref->setCodecCode(codec_code);
	returnref->setBufferManager(buffer_manager);
	
	return returnref;
}


shared_ptr<SelectionVariable> VariableRegistry::addSelectionVariable(VariableProperties *props){
	
	VariableProperties *props_copy;
	
	if(props != NULL){
		props_copy = new VariableProperties(*props);	
	} else {
		props_copy = NULL;
	}
	
	
	shared_ptr<SelectionVariable> returnref(new SelectionVariable(props_copy));
	
	int codec_code = master_variable_list.addReference(returnref);
	std::string tag = returnref->getVariableName();
    if(!tag.empty()){
        master_variable_dictionary[tag] = returnref;
	}
    
    selection_variable_list.addReference(returnref);
	
	returnref->setCodecCode(codec_code);
	returnref->setBufferManager(buffer_manager);
	return returnref;
}

shared_ptr<ScopedVariable> VariableRegistry::createScopedVariable(weak_ptr<ScopedVariableEnvironment> env,
																	VariableProperties *props) {
	
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<ScopedVariable> var = addScopedVariable(env, props);
	
	return var;
}

shared_ptr<GlobalVariable> VariableRegistry::createGlobalVariable(VariableProperties *props) {
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<GlobalVariable> var = addGlobalVariable(props);
	
	return var;
}

shared_ptr<ConstantVariable> VariableRegistry::createConstantVariable(Data value){	
	return addConstantVariable(value);
}

shared_ptr<EmptyVariable> VariableRegistry::createPlaceholderVariable(VariableProperties *props){
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<EmptyVariable> var = addPlaceholderVariable(props);
	
	return var;
}


shared_ptr<SelectionVariable> VariableRegistry::createSelectionVariable(VariableProperties *props){
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<SelectionVariable> var = addSelectionVariable(props);
	
	return var;
}


Data VariableRegistry::getCodec() {
	
	ScarabDatum *codec = NULL;
    shared_ptr<Variable> var;
	
	
	boost::mutex::scoped_lock s_lock(lock);

    int dictSize = master_variable_list.getNElements();
	
	codec = scarab_dict_new(dictSize, &scarab_dict_times2);
	
	
    for(int i = 0; i < dictSize; i++) {
        var = master_variable_list[i];
		
		if(var == NULL) { continue; }
        
		int codec_code = var->getCodecCode();
		
		if(codec_code == RESERVED_CODEC_CODE) {
			continue;
		}
        
		VariableProperties *props = var->getProperties();
		if(props == NULL){ continue; }
		Data serialized_var(props->operator Data());
		
        if(serialized_var.isUndefined()) {
            mdebug("local parameter null value at param (%d)", i);
        }
		ScarabDatum *codec_key = scarab_new_integer(codec_code);
        scarab_dict_put(codec, codec_key, serialized_var.getScarabDatum());
		scarab_free_datum(codec_key);
    }
	
	
	Data returnCodec(codec);
	scarab_free_datum(codec);
    return returnCodec;  
}


/// Return the (constant) value of a variable.
stx::AnyScalar	VariableRegistry::lookupVariable(const std::string &varname) const{
	
	shared_ptr<Variable> var = getVariable(varname);
	if(var == NULL){
		// TODO: throw better
		throw  SimpleException("Failed to find variable during expression evaluation", varname);
	}
	
	Data value = *(var);
	stx::AnyScalar retval = value;
	return retval;
	
}

namespace mw {
shared_ptr<VariableRegistry> GlobalVariableRegistry;
static bool registry_initialized = false;
}

void initializeVariableRegistry() {
	GlobalVariableRegistry = shared_ptr<VariableRegistry>(new VariableRegistry(GlobalBufferManager));
    registry_initialized = true;
	
}
