/*
*  ScopedVariableContext.cpp
*  Experimental Control with Cocoa UI
*
*  Created by David Cox on Wed Dec 11 2002.
*  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
*
*/

#include "ScopedVariableContext.h"
#include "Utilities.h"

using namespace mw;

ScopedVariableContext::ScopedVariableContext(ScopedVariableEnvironment *env) {
    
	if(env == NULL){
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Attempt to create a context with an invalid environmnet");
		return;
	}
	environment = env;
	
	int nfields = environment->getNVariables();
	
	transparency = map<int, Transparency>();
	data = map<int, shared_ptr<Data> >();
	
	shared_ptr<Data> undefined(new Data());
    for(int j = 0; j < nfields; j++){
        transparency[j] = M_TRANSPARENT;
		data[j] = undefined;
	}
}

ScopedVariableContext::ScopedVariableContext(shared_ptr<ScopedVariableContext> ownersinfo){  // point through to a parent
    environment = ownersinfo->getEnvironment();
	inheritFrom(ownersinfo);
}

ScopedVariableContext::~ScopedVariableContext() { }

Transparency ScopedVariableContext::getTransparency(ScopedVariable *param){
    return getTransparency(param->getContextIndex());
}
Transparency ScopedVariableContext::getTransparency(int i){
	if(i < 0 || i > (int)transparency.size()){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to access invalid index in variable context");
		return M_TRANSPARENT;
	}
    return transparency[i];
}


// TODO: There is still much work to be done in this area.
// The intent is to properly "push" the context onto the stack of
// contexts that came before

void ScopedVariableContext::inheritFrom(shared_ptr<ScopedVariableContext> info_to_inherit){  // inherit from a parent, unless already defined opaque
    

	int nfields = data.size();
	
    if(nfields == 0){  // lists need initialization
        nfields = info_to_inherit->getNFields();
        for(int i = 0; i < info_to_inherit->getNFields(); i++){
            //data.addElement(i,NULL);
            transparency[i] = M_TRANSPARENT;
        }
    }
    
            
    for(int i = 0; i < nfields; i++){
		Transparency tr = transparency[i];
		
		if(tr == M_TRANSPARENT){
			Data *datum = new Data(info_to_inherit->get(i));
			data[i] =  shared_ptr<Data>(datum);
		}
    }                                                                		    
}

int ScopedVariableContext::getNFields(){ // how many fields?
    return data.size();
}

Data ScopedVariableContext::get(int index){
	
	if(index < 0 || index > (int)data.size()){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
			"Attempt to access a variable context with an invalid index (%d; context has %d actual elements)",
			index, data.size());
		return Data();
	}
	
	
	Data result; 

	if(data.find(index) == data.end()){
		// TODO: throw
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to access invalid index in a variable context: %d",
			   index);
	   return Data(0L);
	}
	result = *(data[index]);

	return result;
}

void ScopedVariableContext::set(int index, shared_ptr<Data> newdata){
	
	data[index] = newdata;
	transparency[index] = M_OPAQUE;
	
}

void ScopedVariableContext::set(int index, const Data& _newdata){
	Data *newdata = new Data(_newdata);
	
	//if(index >= (int)data.size()){
	//	data.reserve(index);
	//}
	
	data[index] =  shared_ptr<Data>(newdata);
    transparency[index] = M_OPAQUE;
}

void ScopedVariableContext::setWithTransparency(int index, shared_ptr<Data> newdata){
	data[index] = newdata;
	
    transparency[index] = M_TRANSPARENT;
}

void ScopedVariableContext::setWithTransparency(int index, const Data& _newdata){
	Data *newdata = new Data(_newdata);
	data[index] = shared_ptr<Data>(newdata);
	
    transparency[index] = M_TRANSPARENT;
}

