#include "KeyTrigger.h"
#include "Experiment.h"


BEGIN_NAMESPACE_MW


void KeyTrigger::trigger(){  // called from somewhere else when the key is pressed

	if(*active){
	
		if(*togglevar){
			togglevar->setValue(Datum(false));
			//GlobalCurrentExperiment->setBool(togglevar, false);
		}else{
			togglevar->setValue(Datum(true));
			//GlobalCurrentExperiment->setBool(togglevar, true);
		}
		
		if(setadditionalvalue){
			radiovar->setValue(Datum(value));
			//GlobalCurrentExperiment->setInt(radiovar, value);
		}
	}
}

bool KeyTrigger::isActive() { 
	return (bool)(*active);
}


END_NAMESPACE_MW
