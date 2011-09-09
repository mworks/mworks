/*
*  VariableContext.cpp
*  Experimental Control with Cocoa UI
*
*  Created by David Cox on Wed Dec 11 2002.
*  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
*
*/

#include "VariableContext.h"
#include "Utilities.h"

//mExpandableList<Data> *data; // a list of pointers to data
//mExpandableList<Transparency> *transparency; // a list of flags whether the object


VariableContext::VariableContext(){
    data = new ExpandableList<Data>(6);
    transparency = new ExpandableList<Transparency>();
}

VariableContext::VariableContext(int nfields){	     // just create an empty structure
    data = new ExpandableList<Data>(nfields);
    transparency = new ExpandableList<Transparency>(nfields);

    for(int j = 0; j < nfields; j++){
        data->addElement(NULL);
        transparency->addElement(M_TRANSPARENT);
    }
}

VariableContext::VariableContext(VariableContext *ownersinfo){  // point through to a parent
    int nfields = ownersinfo->getNFields();

    data = new ExpandableList<Data>(nfields);	
    transparency = new ExpandableList<Transparency>(nfields);		

    for(int j = 0; j < nfields; j++){
        data->addElement(ownersinfo->getDataPointer(j)); // point through to the previous object
        transparency->addElement(ownersinfo->getTransparency(j));
    }
}

VariableContext::~VariableContext() {
    // TODO: make sure this doesn't leak
    data->releaseElements();
	delete data;
	
	transparency->releaseElements();
    delete transparency;
}

Transparency VariableContext::getTransparency(Variable *param){
    return getTransparency(param->getIndex());
}
Transparency VariableContext::getTransparency(int i){
    return *(transparency->getElement(i));
}

void VariableContext::inheritFrom(VariableContext *info_to_inherit){  // inherit from a parent, unless already defined opaque
    
    if(data == NULL){
		data = new ExpandableList<Data>(info_to_inherit->getNFields());
	}
	
	if(transparency == NULL){
		transparency = new ExpandableList<Transparency>(info_to_inherit->getNFields());
	}
	
	
    int nfields = data->getNElements();
    
    if(nfields == 0){  // lists need initialization
        nfields = info_to_inherit->getNFields();
        for(int i = 0; i < info_to_inherit->getNFields(); i++){
            data->addElement(i,NULL);
            transparency->addElement(i,M_TRANSPARENT);
        }
    }
    
            
    for(int i = 0; i < nfields; i++){
		Transparency *tr = transparency->getElement(i);
		if(tr == NULL){
			transparency->addElement(i, M_TRANSPARENT);
		}
		// TODO: clean out all of this garbage; it didn't work right
        /*if(*(transparency->getElement(i)) == M_TRANSPARENT){
			if(info_to_inherit->getDataPointer(i) == NULL){
				mprintf("Trying to inherit a null data pointer...");
			} else {
				setDataPointer(i, info_to_inherit->getDataPointer(i));
			}
        }*/
    }                                                                		
    
}


Data *VariableContext::getDataPointer(Variable *param){ // this gets the pointer stored in the data list
    return getDataPointer(param->getIndex());
}

Data *VariableContext::getDataPointer(int index){ // this gets the pointer stored in the data list
    return data->getElement(index);
}

void VariableContext::setDataPointer(Variable *param, Data *dp){  // set the pointer manually
    setDataPointer(param->getIndex(), dp);
}
void VariableContext::setDataPointer(int index, Data *dp){  // set the pointer manually
    data->addElement(index, dp);
}

int VariableContext::getNFields(){ // how many fields?
    return data->getNElements();
}

Data VariableContext::get(Variable *param){

    if(param == NULL){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
			"Attempt to access a NULL variable");
		return Data(0L);
	}
	
	int index = param->getIndex();

	//lock();
	
	
	if(index < 0){// || index > data->getNElements()){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
			"Attempt to access a variable context with an invalid index (%d; context has %d actual elements)",
			index, data->getNElements());
		return Data(0L);
	}
	
	Data result; 
	bool success = data->getElement(param->getIndex(), &result);
	
	if(!success){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
			"Received an invalid value from a variable context (NULL)");
		return Data();
	}
	
	// unlock();
	return result;
}

void VariableContext::set(Variable *param, Data *newdata){
    //lock();
	data->addElement(param->getIndex(), newdata);
    transparency->addElement(param->getIndex(), M_OPAQUE);
	// unlock();
}

void VariableContext::set(Variable *param, const Data& _newdata){
    //lock();
	Data newdata(_newdata);
	data->addElement(param->getIndex(), newdata);
    transparency->addElement(param->getIndex(), M_OPAQUE);
	// unlock();
}

void VariableContext::set(Variable *param, long newdata){
    //lock();
	Data thedata(newdata);
    data->addElement(param->getIndex(), &thedata);
    transparency->addElement(param->getIndex(), M_OPAQUE);
	// unlock();
}

void VariableContext::set(Variable *param, double newdata){
    //lock();
	Data thedata(newdata);
    data->addElement(param->getIndex(), &thedata);
    transparency->addElement(param->getIndex(), M_OPAQUE);
	// unlock();
}

void VariableContext::set(Variable *param, bool newdata){
    //lock();
	Data thedata(newdata);
    data->addElement(param->getIndex(), &thedata);
    transparency->addElement(param->getIndex(), M_OPAQUE);
	// unlock();
}

void VariableContext::inc(Variable *param){
    //lock();
	data->addElement(param->getIndex(), 1 + data->getElement(param->getIndex()));
	// unlock();
}

GenericDataType VariableContext::getDataType(Variable *index){
	//lock();
	// TODO: this is dangerous
	Data thedata = *(data->getElement(index->getIndex()));
   
	// unlock(); 
	return thedata.getDataType();
}

ExpandableList<Data> * VariableContext::getDataList() {
    return data;
}

ExpandableList<Transparency> * VariableContext::getTransparencyList() {
    return transparency;
}
/*
template <class T>
T getObject(int index){
        Data *temp = (*data)[resolve(index)];
        
        int object_type = temp->getObjectType();
        if(object_type == M_GENERIC_OBJECT_DATA){
                GenericObjectData *genobj;

                if(!(genobj = dynamic_cast<GenericObjectData<T> *>((temp->)))){  // C++ dark arts
                        throw("Error: attempt to cast to an incorrect generic object data type!");
                }
                
                return genobj->getData();
        }
        
        if(object_type == M_BOUND_GENERIC_OBJECT){
                BoundGenericObject *boundgen;

                if(!(boundgen = dynamic_cast<BoundGenericObject<T> *>((temp->)))){ // C++ dark arts
                        throw("Error: attempt to cast to an incorrect bound generic object data type!");
                }
                
                return boundgen->getData();
        }
        
        throw("Error: failed to return a generic object in a getObject() call");
}
*/		
/*
void set(int ind, GenericData value){
        (*data)[ind]->setData(value);
}*/

/*
void VariableContext::setInt(int ind, long value){
        (*data)[resolve(ind)]->setLong(value);
}

void VariableContext::setFloat(int ind, double value){
        (*data)[resolve(ind)]->setDouble(value);
}

void VariableContext::setBool(int ind, bool value){
        (*data)[resolve(ind)]->setBool(value);
}


void VariableContext::setOpaque(int index, Data newdata){
        data->addElement(index, newdata);
}

void VariableContext::setIntOpaque(int ind, long value){
        data->setElement(ind, new Data(value));
}

void VariableContext::setFloatOpaque(int ind, double value){
        data->setElement(ind, new Data(value));
}

void VariableContext::setBoolOpaque(int ind, bool value){
        data->setElement(ind, new Data(value));
}
 TODO: fix this
template <class T>
void setObjectOpaque(int ind, T newdata){
        data->addElement(ind, newdata);
}
*/
/*
template <class T>	
void setObject(int ind, T newdata){
        
        Data *temp = (*data)[resolve(ind)];
        
        int object_type = temp->getObjectType();
        if(object_type == M_GENERIC_OBJECT_DATA){
                GenericObjectData *genobj;

                if(!(genobj = dynamic_cast<GenericObjectData<T> *>((temp->)))){   // C++ black arts!
                        throw("Error: attempt to cast to an incorrect generic object data type!");
                }
                
                genobj->setData(newdata);
        }
                                
        if(object_type == M_BOUND_GENERIC_OBJECT){
                BoundGenericObject *boundgen;

                if(!(boundgen = dynamic_cast<BoundGenericObject<T> *>(temp->))){ // C++ black arts here.
                        throw("Error: attempt to cast to an incorrect bound generic object data type!");
                }
                
                boundgen->setData(newdata);
        }
        
        throw("Error: failed to return a generic object in a getObject() call");
}
*/

