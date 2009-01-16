/*
 *  Announcers.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 7/17/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef ANNOUNCERS_H
#define ANNOUNCERS_H

#include "GenericVariable.h"
#include <boost/weak_ptr.hpp>


// class to define standard experiment component interface for
// announcing status by making changes to pre-negotiated standard announce variables
namespace mw {
class RequestNotification;
class PrivateDataNotification;

class Announcable {

	private:
	shared_ptr<Variable> announceVariable;
        Data       lastAnnouncedData;
        bool        validAnnounceData;
        
    public:
        Announcable(std::string _announceVariableTagname);
        virtual ~Announcable();
        
		void announce(Data _announceData, MonkeyWorksTime time);  // will announce this data
		
		void announce(Data _announceData);  // will announce this data
        void announce();                       // will announce last data
        
};




class Requestable {

private:
	shared_ptr<Variable> requestVariable;
	std::string requestVariableTagname;
	shared_ptr<RequestNotification> requestNotificationObject;

protected:
	void requestSelf(Data data);
	
public:
	Requestable(std::string _requestVariableTagname);
	virtual ~Requestable();
	virtual void notifyRequest(const Data& data, MonkeyWorksTime timeUS) = 0;        // abstract class
                
};

class RequestNotification : public VariableNotification{

protected:
	Requestable *requestedObject;  // weak reference
	
public:
	RequestNotification(Requestable *_requestable);
	virtual void notify(const Data& data, MonkeyWorksTime timeUS);

};


// this class and the accesorry class below provide a standard way for objects to 
//  maintain data in private variables and allow reload of that data
// they deal with all the problems of re-entrant notifications (from or to the subclass object)
class PrivateDataStorable {

protected:
	bool privateVariableNameAlreadyInstantiated;        
	shared_ptr<Variable> privateVariable;
	std::string privateVariableTagname;
	shared_ptr<PrivateDataNotification> privateDataNotificationObject;
	void storePrivateData(Data data);    // called by object to update data
	
public:
	PrivateDataStorable(std::string _privateVariableBaseName, 
						 std::string _privateVariableSubName);
	virtual ~PrivateDataStorable();
	virtual void notifyPrivate(const Data& data, MonkeyWorksTime timeUS)=0;        
                
};

class PrivateDataNotification : public VariableNotification{

protected:
	PrivateDataStorable * privateDataStorableObject;  
	Data lastPrivateData;
	bool respondingToPrivateVarNotification;
	Lockable *privateDataLock;
	
public:
	PrivateDataNotification(PrivateDataStorable *_privateDataStorableObject);
	virtual void notify(const Data& data, MonkeyWorksTime timeUS);
	void setLastPrivateData(Data data);
	bool isRespondingToPrivateVarNotification();

};



}

#endif