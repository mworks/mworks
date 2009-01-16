/*
 *  Announcers.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 7/17/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Announcers.h"

// variable registry for the experiment can be found through here
#include "Experiment.h" 

using namespace mw;

#define VERBOSE_ANNOUNCERS 0

Announcable::Announcable(std::string _announceVariableTagname) {
	
	shared_ptr<VariableRegistry> registry = 
				GlobalCurrentExperiment->getVariableRegistry();
	
    // check if the announce variable is in the registry and get its handle
    if (!(announceVariable = registry->getVariable(_announceVariableTagname))){
        throw SimpleException( "Announcable object has unknown variable tagname.");
    }
    
    validAnnounceData = false;
    
}

Announcable::~Announcable() {};


void Announcable::announce(Data _announceData, MonkeyWorksTime now) {    
    lastAnnouncedData = _announceData;
    validAnnounceData = true;
    announceVariable->setValue(lastAnnouncedData, now);
    if (VERBOSE_ANNOUNCERS > 1) mprintf("*** Announcing data now");
}

void Announcable::announce(Data _announceData) {
	shared_ptr <Clock> clock = Clock::instance();
    announce(_announceData, clock->getCurrentTimeUS());
}

void Announcable::announce() {    
    if (validAnnounceData) {
        announceVariable->setValue(lastAnnouncedData);
    }
    else {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, 
				 "Request to announce data, but nothing yet known.");
    }
}


Requestable::Requestable(std::string _requestVariableTagname) {
	
    //requestVariable = NULL;
    shared_ptr<VariableRegistry> registry = GlobalCurrentExperiment->getVariableRegistry();
	
	
    // check if the request variable is in the registry and get its handle
    if (requestVariable = registry->getVariable(_requestVariableTagname)) {
        
		
        // make an object to handle requests and attach to the variable
        requestNotificationObject = 
		shared_ptr<RequestNotification>(new RequestNotification(this));
        requestVariable->addNotification(requestNotificationObject);
        if (VERBOSE_ANNOUNCERS > 1) 
			mprintf("Request variable established and notification attached.");
		
	} else {       //  if not, register a new variable.
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, 
				 "Requestable object has unknown variable tagname.  "
				 "No requests will be serviced.");               
    }
    
}

Requestable::~Requestable() {
	requestNotificationObject->remove();
}


RequestNotification::RequestNotification(Requestable *_requestedObject) {
    requestedObject = _requestedObject;
}

void RequestNotification::notify(const Data& data, MonkeyWorksTime timeUS) {
    if (VERBOSE_ANNOUNCERS > 1) 
		mprintf("Request just received via update of request variable");
    
    requestedObject->notifyRequest(data, timeUS);
}



PrivateDataStorable::PrivateDataStorable(std::string _privateVariableBaseName, 
										   std::string _privateVariableSubName){
    
    // build expected request variable name
	privateVariableTagname = _privateVariableBaseName + _privateVariableSubName;
    
    shared_ptr<VariableRegistry> registry = 
		GlobalCurrentExperiment->getVariableRegistry();
	
	
    // check if the variable is in the registry and get its handle
    if (privateVariable = registry->getVariable(privateVariableTagname)) {
        
		privateVariableNameAlreadyInstantiated = true;
		
        // TODO -- make sure interface says not edittable
    } else {       //  if not, register a new variable.
		
        privateVariable = registry->createGlobalVariable(new VariableProperties(new Data(M_DICTIONARY, 
																						   (int)1),
																				 privateVariableTagname.c_str(), 
																				 "Private data",
																				 "private data",      
																				 M_NEVER,
																				 M_WHEN_CHANGED, 
																				 true, 
																				 true, 
																				 M_DISCRETE,
																				 PRIVATE_SYSTEM_VARIABLES));                    
		
		privateVariableNameAlreadyInstantiated = false;                        
    }
    
    // make an object to handle requests and attach to the variable
    privateDataNotificationObject = shared_ptr<PrivateDataNotification>(new PrivateDataNotification(this));
    privateVariable->addNotification(privateDataNotificationObject);
	
	
    if (VERBOSE_ANNOUNCERS > 1) 
		mprintf("Request variable established and notification attached.");
	
	
}


PrivateDataStorable::~PrivateDataStorable() { 
	privateDataNotificationObject->remove();
}

// used by the object to effectively save parameters in a request varaible
// only trick is that we must avoid infinite looping
void PrivateDataStorable::storePrivateData(Data data) {
	
	// this prevents re-entrant notification when we change the data ourselves
    privateDataNotificationObject->setLastPrivateData(data);    
	
    
    // now update the variable 
    if (! (privateDataNotificationObject->isRespondingToPrivateVarNotification())) {     
        privateVariable->setValue(data);
    } else {    // in this case, the variable was updated externally and we got to this point through a notification thread.
				//mwarning(M_SYSTEM_MESSAGE_DOMAIN,"mPrivateDataStorable: Trying to set private variable via notification on that same variable.  Not allowed. variable not updated.");
        mprintf(M_SYSTEM_MESSAGE_DOMAIN,"mPrivateDataStorable: We have appropriately blocked setting of private variable via notification on that same variable.  Not allowed. variable not updated.");
        // TODO -- when a new method is added to the variabile class for "updateLater" (new thread), then we can do that here.
    }
}


PrivateDataNotification::PrivateDataNotification(
												   PrivateDataStorable *_privateDataStorableObject) {
    privateDataStorableObject = _privateDataStorableObject;
    respondingToPrivateVarNotification = false;
    privateDataLock = new Lockable();
}

void PrivateDataNotification::notify(const Data& data, 
									  MonkeyWorksTime timeUS) {
    
    respondingToPrivateVarNotification = true;
    
    if (VERBOSE_ANNOUNCERS > 1) 
		mprintf("Private data change just received via update "
				"of private variable");
    
    // if the data is the same as the last requestSelf, then ignore the request
    // (this was probably generated by the object itself)
    
    bool thisDataIsSameAsWhatIalreadyHave = false;
    privateDataLock->lock();
    if (lastPrivateData == data) thisDataIsSameAsWhatIalreadyHave = true;
    privateDataLock->unlock();
    
    if (thisDataIsSameAsWhatIalreadyHave)  {
        if (VERBOSE_ANNOUNCERS > 1) 
			mprintf("Ignoring private data -- same as last private data."); 
    }
    else {
        if (VERBOSE_ANNOUNCERS > 1) 
			mprintf("About to run PrivateDataStorable object "
					"notifyPrivate method."); 
        privateDataStorableObject->notifyPrivate(data, timeUS);   
        // in the subclass that uses this method, do NOT try to update the 
		// variable.  It is locked!
        // (all if this is handled by the PrivateDataStorable class).
    }
    respondingToPrivateVarNotification = false;
}

void PrivateDataNotification::setLastPrivateData(Data data) {
    
    // save the data so that I can prevent re-entry on a notification
    privateDataLock->lock();
    lastPrivateData = data;
    privateDataLock->unlock();
    
}

bool PrivateDataNotification::isRespondingToPrivateVarNotification() {
    return respondingToPrivateVarNotification;
}








