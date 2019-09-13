/**
 * VariableRegistry.cpp
 *
 * Copyright (c) 2005-2016 MIT. All rights reserved.
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
    boost::mutex::scoped_lock s_lock(lock);

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
	
	int maxCodecCode = -1;
	// find the maximum codec value
    for (auto &item : codec.getDict()) {
        maxCodecCode = std::max(maxCodecCode, int(item.first));
    }
		
	// add each variable in order to the registry
	for(int i = N_RESERVED_CODEC_CODES; i<=maxCodecCode; ++i) {
        Datum serializedVariable = codec.getElement(i);
		
		if (serializedVariable.isUndefined()) {
            shared_ptr<EmptyVariable> empty_var(new EmptyVariable);
            master_variable_list.push_back(empty_var);
            continue;
        } else {
			if (!serializedVariable.isDictionary()) {
				// these must be  placeholder datums in the package
				// that we should ignore.
				mwarning(M_SYSTEM_MESSAGE_DOMAIN,
						 "Bad variable received from network stream");
				
                shared_ptr<EmptyVariable> empty_var(new EmptyVariable);
                master_variable_list.push_back(empty_var);
                continue;
			}
						
			VariableProperties *props = new VariableProperties(serializedVariable);
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
	
	if (codec_code < N_RESERVED_CODEC_CODES || codec_code >= master_variable_list.size() + N_RESERVED_CODEC_CODES) {
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


int VariableRegistry::getNVariables() {
    boost::mutex::scoped_lock s_lock(lock);
	return master_variable_list.size();
}


void VariableRegistry::registerVariableName(const boost::shared_ptr<Variable> &var) {
    auto tag = var->getVariableName();
    if (!tag.empty()) {
        auto &slot = master_variable_dictionary[tag];
        if (slot) {
            throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                                  boost::format("A variable with the name \"%s\" already exists") % tag);
        }
        slot = var;
    }
}


void VariableRegistry::addVariable(const boost::shared_ptr<Variable> &var) {
    registerVariableName(var);
    master_variable_list.push_back(var);
    var->setCodecCode(master_variable_list.size() + N_RESERVED_CODEC_CODES - 1);
    var->setEventTarget(event_buffer);
}


shared_ptr<ScopedVariable> VariableRegistry::addScopedVariable(weak_ptr<ScopedVariableEnvironment> env,
                                                               const VariableProperties &props)
{
    // create a new entry and return one instance
	
	VariableProperties *props_copy = new VariableProperties(props);
    shared_ptr<ScopedVariable> new_variable(new ScopedVariable(props_copy));
    
    addVariable(new_variable);
    
    local_variable_list.push_back(new_variable);
	new_variable->setContextIndex(local_variable_list.size() - 1);
	new_variable->setLogging(M_WHEN_CHANGED);
	
    shared_ptr<ScopedVariableEnvironment> env_shared = env.lock();
	if(env_shared){
		env_shared->addVariable(new_variable);
	}
	
	return new_variable;
}


shared_ptr<GlobalVariable> VariableRegistry::addGlobalVariable(const VariableProperties &props) {
	VariableProperties *copy = new VariableProperties(props);
	shared_ptr<GlobalVariable> returnref(new GlobalVariable(copy));
    
    addVariable(returnref);
    
    global_variable_list.push_back(returnref);
	
	return returnref;
}


shared_ptr<Timer> VariableRegistry::createTimer(const VariableProperties &props) {
    boost::mutex::scoped_lock s_lock(lock);
    
	VariableProperties *props_copy = new VariableProperties(props);
	shared_ptr<Timer> new_timer(new Timer(props_copy));
	
    registerVariableName(new_timer);
	new_timer->setCodecCode(-1);
	new_timer->setEventTarget(event_buffer);
	
	return new_timer;
}


shared_ptr<SelectionVariable> VariableRegistry::addSelectionVariable(const VariableProperties &props) {
	VariableProperties *props_copy = new VariableProperties(props);
	shared_ptr<SelectionVariable> returnref(new SelectionVariable(props_copy));
    
    addVariable(returnref);
    
    selection_variable_list.push_back(returnref);
	
	return returnref;
}

shared_ptr<ScopedVariable> VariableRegistry::createScopedVariable(weak_ptr<ScopedVariableEnvironment> env,
                                                                  const VariableProperties &props)
{
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<ScopedVariable> var = addScopedVariable(env, props);
	
	return var;
}

shared_ptr<GlobalVariable> VariableRegistry::createGlobalVariable(const VariableProperties &props) {
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<GlobalVariable> var = addGlobalVariable(props);
	
	return var;
}


shared_ptr<SelectionVariable> VariableRegistry::createSelectionVariable(const VariableProperties &props) {
    boost::mutex::scoped_lock s_lock(lock);

	shared_ptr<SelectionVariable> var = addSelectionVariable(props);
	
	return var;
}


Datum VariableRegistry::generateCodecDatum() {
	boost::mutex::scoped_lock s_lock(lock);

    const int dictSize = master_variable_list.size();
    Datum codec(M_DICTIONARY, dictSize);
	
    for(int i = 0; i < dictSize; i++) {
        shared_ptr<Variable> var = master_variable_list[i];
		
		if(var == NULL) { 
            continue; 
        }
        
		int codec_code = var->getCodecCode();
		
		if(codec_code == RESERVED_CODEC_CODE) {
			continue;
		}
        
		auto props = var->getProperties();
		
        if(props == NULL){ 
            continue; 
        }
        
        Datum serialized_var(*props);
		
        if(serialized_var.isUndefined()) {
            mdebug("local parameter null value at param (%d)", i);
        }
        codec.setElement(codec_code, serialized_var);
    }
	
    return codec;
}


/// Return the (constant) value of a variable.
Datum VariableRegistry::lookupVariable(const std::string &varname) const {
	
	shared_ptr<Variable> var = getVariable(varname);
	if(var == NULL){
        throw UnknownVariableException(varname);
	}
	
	return *(var);
	
}


shared_ptr<VariableRegistry> global_variable_registry;


END_NAMESPACE_MW



























