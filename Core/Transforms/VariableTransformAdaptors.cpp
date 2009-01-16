/*
 *  VariableTransformAdaptors.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 8/8/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "VariableTransformAdaptors.h"

using namespace mw;


VarTransformAdaptor::VarTransformAdaptor() {

	active = true;
	nOutputs = 0;
	nInputs = 0;

	
}

VarTransformAdaptor::~VarTransformAdaptor() {
	
	inputVars.releaseElements();
	outputVars.releaseElements();
	
	for (int i=0; i<inputNotifications.getNElements(); ++i) {
		shared_ptr <VarTransformNotification> notif = inputNotifications.getElement(i);
		notif->remove();
	}
	
	inputNotifications.clear();	// delete the memory of each notification
	
}

// return the index to the input		
int VarTransformAdaptor::registerInput(shared_ptr<Variable> input) {

	if(input == shared_ptr<Variable>()){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
		  "Attempt to register a NULL input with a variable transform adaptor");
		return -1;
	}
	int inputIndex = inputVars.addReference(input);
	
	// setup the notification
	shared_ptr<VarTransformNotification> notification( 
							new VarTransformNotification(this, inputIndex));
    
	// set the var to callback to the "notify" method if the value of var is changed                            
	input->addNotification(notification);		
	inputNotifications.addReference(notification);	// keep list to kill later
    nInputs++;
	return(inputIndex);

}	

int VarTransformAdaptor::registerOutput(shared_ptr<Variable> output) {

	int outputIndex = outputVars.addReference(output);
    nOutputs++;
	return(outputIndex);
}	


// allows access to the registered input variables
bool VarTransformAdaptor::getInputVariable(int inputIndex, shared_ptr<Variable> *inputVariable){
    
    if ((inputIndex >= 0) && (inputIndex < inputVars.getNElements())) {
        *inputVariable = inputVars[inputIndex];
        return (true);
    }
    else {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			"mVarTransformAdaptor::getInputVariable inputIndex out of range!");
        return (false);
    }
        
}


// this method should be overridden by inherited classes -- warn if not done so
void VarTransformAdaptor::newDataReceived(int inputIndex, const Data& data, 
                                                MonkeyWorksTime timeUS) {
    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Notification of variable change without override to do anything.");
}
				

// will update this specific Var with this data 
bool VarTransformAdaptor::postResults(int outputIndex, const Data& data) {

	bool _active = active;

	if (_active) {
		shared_ptr<Variable> var = (outputVars.getElement(outputIndex));
		if(var != NULL){
			var->setValue(data);
		}
        return true;
	}
	return false;
	
}

// will update this specific Var with this data 
bool VarTransformAdaptor::postResults(int outputIndex, const Data& data, 
                                                        MonkeyWorksTime timeUS) {

	//lock();	// DDC edit
	bool _active = active;
	//unlock();
	if (_active) {
	
		shared_ptr<Variable> var = (outputVars[outputIndex]);
		if(var != NULL){
			var->setValue(data, timeUS);
		}

		return true;
	}
	return false;
	
}


// ===================================


VarTransformNotification::VarTransformNotification
                (VarTransformAdaptor *_varTransformAdaptor, int _inputIndex) {
	
	inputIndex = _inputIndex;
	varTransformAdaptor = _varTransformAdaptor;
   
}

VarTransformNotification::~VarTransformNotification() {}

void VarTransformNotification::notify(const Data& data, MonkeyWorksTime timeUS) {
	
	varTransformAdaptor->newDataReceived(inputIndex, data, timeUS);
}


