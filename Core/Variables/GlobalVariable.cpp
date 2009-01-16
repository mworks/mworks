/*
 *  GlobalVariable.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GlobalVariable.h"
#include "VariableProperties.h"
#include <boost/shared_ptr.hpp>
using namespace mw;


/*******************************************************************
*                   GlobalVariable memeber fucntions
*******************************************************************/

GlobalVariable::GlobalVariable(Data _value, 
									 VariableProperties *_props) :
									 Variable(_props) {
	value = shared_ptr<Data>(new Data(_value));
	valueLock = shared_ptr<Lockable>(new Lockable());
}


GlobalVariable::GlobalVariable(VariableProperties *_props) :
									 Variable(_props){
    if(properties != NULL){
		value = shared_ptr<Data>(new Data(properties->getDefaultValue()));
	} else {
		value = shared_ptr<Data>(new Data());
	}
	
	valueLock = shared_ptr<Lockable>(new Lockable());
}

Variable *GlobalVariable::clone(){
	valueLock->lock();
	GlobalVariable *returned = 
		new GlobalVariable((const GlobalVariable&)(*this));
	
	returned->value = value;
	returned->valueLock = valueLock;
	valueLock->unlock();
	return (Variable *)returned;
}


/*GlobalVariable::GlobalVariable(ScarabDatum * datum) {
	
    if(datum->type == SCARAB_DICT) {
        ScarabDatum * val;
        val = scarab_dict_get(datum, scarab_new_string("value"));
        value = Data(val);
    }        
	
	type = M_CONSTANT_VARIABLE;
}*/

GlobalVariable::~GlobalVariable() { }

Data GlobalVariable::getValue() {
	valueLock->lock();
    if(value->getDataType() == M_UNDEFINED) {
        Data undef;
		valueLock->unlock();
        return undef;
    }
	Data retval = *value;
	valueLock->unlock();
	
    return retval;
}
    
void GlobalVariable::setValue(Data newval) {
	setSilentValue(newval);
	announce();
}

void GlobalVariable::setValue(Data newval, MonkeyWorksTime time) {
	setSilentValue(newval, time);
	announce(time);
}

void GlobalVariable::setSilentValue(Data newval) {
	valueLock->lock();
	*value=newval;
	valueLock->unlock();
	performNotifications(newval);
}

void GlobalVariable::setSilentValue(Data newval, MonkeyWorksTime timeUS) {
	valueLock->lock();
	*value=newval;
	valueLock->unlock();
	performNotifications(newval, timeUS);
}

bool GlobalVariable::isValue() {
    return true;
}

/*ScarabDatum * GlobalVariable::toScarabPackage() {

	ScarabDatum * package = NULL;
	
    if(package == NULL) {
        package = scarab_dict_new(__VALUE_PACKAGE_SIZE__,
                                                    &scarab_dict_times2);
		ScarabDatum *key = scarab_new_string("value");
        scarab_dict_put(package, key, value.getScarabDatum());
		scarab_free_datum(key);
    }
    return package;
}*/

void GlobalVariable::printToSTDERR() {
	valueLock->lock();
    fprintf(stderr, "mGlobalVariable.value => %p", &value);
    value->printToSTDERR();
	valueLock->unlock();
}