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
#include "EventConstants.h"
#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


VariableRegistry::VariableRegistry(shared_ptr<EventBuffer> _buffer) {
	event_buffer = _buffer;
	
    current_unique_code = N_RESERVED_CODEC_CODES;
}

void VariableRegistry::reset(){

    master_variable_list.clear();
	
    // for faster lookups by tag name
    master_variable_dictionary = map< string, shared_ptr<Variable> >();
    
	// just the local variables
	local_variable_list.clear();
	
	// just the global variables
    global_variable_list.clear();
	
	// just the selection variables
    selection_variable_list.clear();
    
    //addPlaceholders();
}
	
VariableRegistry::~VariableRegistry() { }

void VariableRegistry::updateFromCodecDatum(const Datum &codec) {
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
	//addPlaceholders();
	
	
	//////////////////////////////////////////////////////////////////
	// now add what's in the codec 
	
	ScarabDatum *datum = codec.getScarabDatum();
	
	ScarabDatum ** keys = datum->data.dict->keys;
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
	for(int i = N_RESERVED_CODEC_CODES; i<=maxCodecCode; ++i) {
		ScarabDatum *key = scarab_new_integer(i);
		ScarabDatum *serializedVariable = scarab_dict_get(datum, key);
		scarab_free_datum(key);
		
		if(!serializedVariable) {
            shared_ptr<EmptyVariable> empty_var(new EmptyVariable);
            master_variable_list.push_back(empty_var);
            continue;
        } else {
			if(serializedVariable->type != SCARAB_DICT) {
				// these must be  placeholder datums in the package
				// that we should ignore.
				mwarning(M_SYSTEM_MESSAGE_DOMAIN,
						 "Bad variable received from network stream");
				
                shared_ptr<EmptyVariable> empty_var(new EmptyVariable);
                master_variable_list.push_back(empty_var);
                continue;
			}
						
			VariableProperties *props = 
				new VariableProperties(serializedVariable);
			
			if(props == NULL){
				mwarning(M_SYSTEM_MESSAGE_DOMAIN,
						 "Bad variable received from network stream");
				
                shared_ptr<EmptyVariable> empty_var(new EmptyVariable);
                master_variable_list.push_back(empty_var);
                continue;
			}
			
			shared_ptr<Variable> newvar(new GlobalVariable(props));
			newvar->setCodecCode(i); //necessary? .. Yup
			
			master_variable_list.push_back(newvar);
            
            std::string tag = newvar->getVariableName();
            if(!tag.empty()){
                master_variable_dictionary[tag] = newvar;
            }
		}
	}
}


void VariableRegistry::announceLocalVariables(){
	boost::mutex::scoped_lock s_lock(lock);
	
    vector< shared_ptr<Variable> >::iterator i;
	for(i = local_variable_list.begin(); i != local_variable_list.end(); i++){
		(*i)->announce();
	}
}

void VariableRegistry::announceGlobalVariables(){
	boost::mutex::scoped_lock s_lock(lock);
	
	vector< shared_ptr<Variable> >::iterator i;
	for(i = global_variable_list.begin(); i != global_variable_list.end(); i++){
		(*i)->announce();
	}
	
}

void VariableRegistry::announceSelectionVariables(){
	boost::mutex::scoped_lock s_lock(lock);
	
	vector< shared_ptr<Variable> >::iterator i;
	for(i = selection_variable_list.begin(); i != selection_variable_list.end(); i++){
		(*i)->announce();
	}
}

void VariableRegistry::announceAll() {
	boost::mutex::scoped_lock s_lock(lock);
	
    vector< shared_ptr<Variable> >::iterator i;
    for(i = master_variable_list.begin(); i != master_variable_list.end(); i++){
		(*i)->announce();
	}	
}


// There is potential room for speed up with a hash_map, though this would
// require const char * machinations, as hash_map cannot have string keys
shared_ptr<Variable> VariableRegistry::getVariable(const std::string& tagname) const{
	boost::mutex::scoped_lock s_lock(lock);
	
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

	boost::mutex::scoped_lock s_lock(lock);
	
    // DDC: removed what was this for?
	//mExpandableList<Variable> list(master_variable_list);
	
	if(codec_code < 0 || codec_code > master_variable_list.size() + N_RESERVED_CODEC_CODES){
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Attempt to get an invalid variable (code: %d)",
			   codec_code);
		
		var = shared_ptr<Variable>();
	} else {
        // DDC: removed copying.  What was that for?
		//var = list[codec_code];
        var = master_variable_list[codec_code - N_RESERVED_CODEC_CODES];
	}
	
	return var;
}

std::vector<std::string> VariableRegistry::getVariableTagNames() {
	boost::mutex::scoped_lock s_lock(lock);
	
	std::vector<std::string> tagnames;
	
    vector< shared_ptr<Variable> >::iterator i;
	for(i = master_variable_list.begin(); i != master_variable_list.end(); i++){		
		shared_ptr<Variable> var = (*i);		
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
	return master_variable_list.size();
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
    
    master_variable_list.push_back(new_variable);
	codec_code = master_variable_list.size() + N_RESERVED_CODEC_CODES - 1;
    
    std::string tag = new_variable->getVariableName();
    if(!tag.empty()){
        master_variable_dictionary[tag] = new_variable;
	}
    
    local_variable_list.push_back(new_variable);
	variable_context_index = local_variable_list.size() - 1;
	
	new_variable->setContextIndex(variable_context_index);
	new_variable->setLogging(M_WHEN_CHANGED);
	new_variable->setCodecCode(codec_code);
	new_variable->setEventTarget(boost::static_pointer_cast<EventReceiver>(event_buffer));
	
    shared_ptr<ScopedVariableEnvironment> env_shared = env.lock();
	if(env_shared){
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
	
    master_variable_list.push_back(returnref);
	int codec_code = master_variable_list.size() + N_RESERVED_CODEC_CODES - 1;
	
    std::string tag = returnref->getVariableName();
    if(!tag.empty()){
        master_variable_dictionary[tag] = returnref;
	}
    
    
    global_variable_list.push_back(returnref);
	
	returnref->setCodecCode(codec_code);
	returnref->setEventTarget(boost::static_pointer_cast<EventReceiver>(event_buffer));
	
	return returnref;
}

shared_ptr<ConstantVariable> VariableRegistry::addConstantVariable(Datum value){
	
	
	shared_ptr<ConstantVariable> returnref(new ConstantVariable(value));
	returnref->setCodecCode(-1);
	returnref->setEventTarget(boost::static_pointer_cast<EventReceiver>(event_buffer));
	
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
	new_timer->setEventTarget(boost::static_pointer_cast<EventReceiver>(event_buffer));	
	
	return new_timer;
}

//shared_ptr<EmptyVariable> VariableRegistry::addPlaceholderVariable(VariableProperties *props){
//	
//	VariableProperties *props_copy;
//	
//	if(props != NULL){
//		props_copy = new VariableProperties(*props);	
//	} else {
//		props_copy = NULL;
//	}
//	
//	
//	shared_ptr<EmptyVariable> returnref(new EmptyVariable(props_copy));
//    
//    master_variable_list.push_back(returnref);
//	int codec_code = master_variable_list.size() + N_RESERVED_CODEC_CODES - 1;
//	
//	returnref->setCodecCode(codec_code);
//	returnref->setEventTarget(static_pointer_cast<EventReceiver>(event_buffer));
//	
//	return returnref;
//}


shared_ptr<SelectionVariable> VariableRegistry::addSelectionVariable(VariableProperties *props){
	
	VariableProperties *props_copy;
	
	if(props != NULL){
		props_copy = new VariableProperties(*props);	
	} else {
		props_copy = NULL;
	}
	
	
	shared_ptr<SelectionVariable> returnref(new SelectionVariable(props_copy));
	
    master_variable_list.push_back(returnref);
	int codec_code = master_variable_list.size() + N_RESERVED_CODEC_CODES - 1;
    
	std::string tag = returnref->getVariableName();
    if(!tag.empty()){
        master_variable_dictionary[tag] = returnref;
	}
    
    selection_variable_list.push_back(returnref);
	
	returnref->setCodecCode(codec_code);
	returnref->setEventTarget(boost::static_pointer_cast<EventReceiver>(event_buffer));
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

shared_ptr<ConstantVariable> VariableRegistry::createConstantVariable(Datum value){	
	return addConstantVariable(value);
}

//shared_ptr<EmptyVariable> VariableRegistry::createPlaceholderVariable(VariableProperties *props){
//    boost::mutex::scoped_lock s_lock(lock);
//
//	shared_ptr<EmptyVariable> var = addPlaceholderVariable(props);
//	
//	return var;
//}


shared_ptr<SelectionVariable> VariableRegistry::createSelectionVariable(VariableProperties *props){
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<SelectionVariable> var = addSelectionVariable(props);
	
	return var;
}


Datum VariableRegistry::generateCodecDatum() {
	
	ScarabDatum *codec = NULL;
    shared_ptr<Variable> var;
	
	
	boost::mutex::scoped_lock s_lock(lock);

    int dictSize = master_variable_list.size();
	
	codec = scarab_dict_new(dictSize, &scarab_dict_times2);
	
	
    for(int i = 0; i < dictSize; i++) {
        var = master_variable_list[i];
		
		if(var == NULL) { 
            continue; 
        }
        
		int codec_code = var->getCodecCode();
		
		if(codec_code == RESERVED_CODEC_CODE) {
			continue;
		}
        
		VariableProperties *props = var->getProperties();
		
        if(props == NULL){ 
            continue; 
        }
        
        Datum serialized_var(props->operator Datum());
		
        if(serialized_var.isUndefined()) {
            mdebug("local parameter null value at param (%d)", i);
        }
		ScarabDatum *codec_key = scarab_new_integer(codec_code);
        scarab_dict_put(codec, codec_key, serialized_var.getScarabDatum());
		scarab_free_datum(codec_key);
    }
	
	
    Datum returnCodec(codec);
	scarab_free_datum(codec);
    return returnCodec;  
}


/// Return the (constant) value of a variable.
stx::AnyScalar	VariableRegistry::lookupVariable(const std::string &varname) const{
	
	shared_ptr<Variable> var = getVariable(varname);
	if(var == NULL){
        throw UnknownVariableException(varname);
	}
	
 Datum value = *(var);
	stx::AnyScalar retval = value;
	return retval;
	
}


stx::AnyScalar VariableRegistry::lookupVariable(const std::string &varname, const stx::AnyScalar &subscript) const {
    shared_ptr<Variable> var = getVariable(varname);
    if (!var) {
        throw UnknownVariableException(varname);
    }
    
    shared_ptr<SelectionVariable> sel = boost::dynamic_pointer_cast<SelectionVariable>(var);
    if (!sel) {
        throw SimpleException("Variable does not support subscripts", varname);
    }
    
    stx::AnyScalar value = sel->getTentativeSelection(subscript.getInteger());
    return value;
}


shared_ptr<VariableRegistry> global_variable_registry;
//static bool registry_initialized = false;


//void initializeVariableRegistry() {
//	global_variable_registry = shared_ptr<VariableRegistry>(new VariableRegistry(global_outgoing_event_buffer));
//    registry_initialized = true;
//	
//}


END_NAMESPACE_MW



























