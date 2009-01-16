/*
 *  ObjectFactory.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sun Dec 29 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "ObjectFactory.h"

// class ObjectFactory {

ObjectFactory::ObjectFactory(){ type = M_UNRESOLVED_SERVICE; }
ObjectFactory::~ObjectFactory() { }
ServiceType ObjectFactory::whatType(){ return type; }
bool ObjectFactory::sanityCheck(){ return false; };


// class StateFactory : public ObjectFactory {

StateFactory::StateFactory(){ type = M_STATE_COMPONENT; };
StateFactory::~StateFactory() { }
State *StateFactory::createState(){ return NULL;}

bool StateFactory::sanityCheck(){ return (type == M_STATE_COMPONENT); }
  


//class SelectionFactory : public ObjectFactory {
SelectionFactory::SelectionFactory(){ type = M_SELECTION_COMPONENT; };
SelectionFactory::~SelectionFactory() { }
Selection *SelectionFactory::createSelection(){ return NULL; }

bool SelectionFactory::sanityCheck(){ return (type == M_SELECTION_COMPONENT); }


// class SchedulerFactory : public ObjectFactory {
SchedulerFactory::SchedulerFactory(){ 
    type = M_SCHEDULER_COMPONENT; 
}
SchedulerFactory::~SchedulerFactory() { }
Scheduler *SchedulerFactory::createScheduler(){ 
    return NULL; 
}

bool SchedulerFactory::sanityCheck(){ 
    return (type == M_SCHEDULER_COMPONENT); 
}



//class ClockFactory : public ObjectFactory {
ClockFactory::ClockFactory(){ 
    type = M_CLOCK_COMPONENT; 
}
ClockFactory::~ClockFactory(){ }
Clock *ClockFactory::createClock(){ 
    return NULL; 
}

bool ClockFactory::sanityCheck(){ 
    return (type == M_CLOCK_COMPONENT); 
}



// class StateSystemFactory : public ObjectFactory 
StateSystemFactory::StateSystemFactory(){ 
    type = M_STATE_SYSTEM_COMPONENT; 
}
StateSystemFactory::~StateSystemFactory(){ }
StateSystem *StateSystemFactory::createStateSystem(){
    return NULL; 
};

bool StateSystemFactory::sanityCheck(){ 
    return (type == M_STATE_SYSTEM_COMPONENT); 
}
