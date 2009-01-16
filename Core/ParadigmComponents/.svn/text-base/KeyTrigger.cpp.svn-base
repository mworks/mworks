#include "KeyTrigger.h"
#include "Experiment.h"
using namespace mw;

void KeyTrigger::trigger(){  // called from somewhere else when the key is pressed

	if(*active){
	
		if(*togglevar){
			togglevar->setValue(Data(false));
			//GlobalCurrentExperiment->setBool(togglevar, false);
		}else{
			togglevar->setValue(Data(true));
			//GlobalCurrentExperiment->setBool(togglevar, true);
		}
		
		if(setadditionalvalue){
			radiovar->setValue(Data(value));
			//GlobalCurrentExperiment->setInt(radiovar, value);
		}
	}
}

bool KeyTrigger::isActive() { 
	return (bool)(*active);
}