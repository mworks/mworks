/*
 *  Announcers.h
 *  MWorksCore
 *
 *  Created by dicarlo on 7/17/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef ANNOUNCERS_H
#define ANNOUNCERS_H

#include "GenericVariable.h"


// class to define standard experiment component interface for
// announcing status by making changes to pre-negotiated standard announce variables


BEGIN_NAMESPACE_MW


class RequestNotification;
class PrivateDataNotification;


class Announcable {
    
public:
    explicit Announcable(const std::string &announceVariableTagname);
    virtual ~Announcable() { }
    
    void announce(Datum announceData, MWTime time) const;
    void announce(Datum announceData) const;
    
private:
    const boost::shared_ptr<Variable> announceVariable;
    
};


class Requestable {

private:
	shared_ptr<Variable> requestVariable;
	std::string requestVariableTagname;
	shared_ptr<RequestNotification> requestNotificationObject;

protected:
	void requestSelf(Datum data);
	
public:
	Requestable(std::string _requestVariableTagname);
	virtual ~Requestable();
	virtual void notifyRequest(const Datum& data, MWTime timeUS) = 0;        // abstract class
                
};

class RequestNotification : public VariableNotification{

protected:
	Requestable *requestedObject;  // weak reference
	
public:
	RequestNotification(Requestable *_requestable);
	virtual void notify(const Datum& data, MWTime timeUS);

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
	void storePrivateData(Datum data);    // called by object to update data
	
public:
	PrivateDataStorable(std::string _privateVariableBaseName, 
						 std::string _privateVariableSubName);
	virtual ~PrivateDataStorable();
	virtual void notifyPrivate(const Datum& data, MWTime timeUS)=0;        
                
};

class PrivateDataNotification : public VariableNotification{

protected:
	PrivateDataStorable * privateDataStorableObject;  
 Datum lastPrivateData;
	bool respondingToPrivateVarNotification;
	Lockable *privateDataLock;
	
public:
	PrivateDataNotification(PrivateDataStorable *_privateDataStorableObject);
	virtual void notify(const Datum& data, MWTime timeUS);
	void setLastPrivateData(Datum data);
	bool isRespondingToPrivateVarNotification();

};


END_NAMESPACE_MW


#endif
