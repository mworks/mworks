#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

/*
 *  ObjectFactory.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sat Dec 28 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *  See "ServiceRegistry.h" in the ComponentRegistries section for a discussion
 *  of ObjectFactories and plugins.
 */

#include "States.h"
#include "Selection.h"
#include "Scheduler.h"
#include "Clock.h"
#include "StateSystem.h"
#include "Stimulus.h"
#include "ServiceTypes.h"

class ObjectFactory {
    // base class which must have a createXXXX methods that returns an appropriate
    // type of object
    protected:
        ServiceType type;
    
    public:
        ObjectFactory();
        virtual ~ObjectFactory();
        virtual ServiceType whatType();
        virtual bool sanityCheck();
};


class StateFactory : public ObjectFactory {

    public:
        StateFactory();
        virtual ~StateFactory();
        virtual State *createState();
        virtual bool sanityCheck();
};

class SelectionFactory : public ObjectFactory {
    
    public:
        SelectionFactory();
        virtual ~SelectionFactory();
        virtual Selection *createSelection();
        virtual bool sanityCheck();
};

class SchedulerFactory : public ObjectFactory {
    public:
        SchedulerFactory();
        virtual ~SchedulerFactory();
        virtual Scheduler *createScheduler();
        virtual bool sanityCheck();
};

class ClockFactory : public ObjectFactory {
    public:
        ClockFactory();
        virtual ~ClockFactory();
        virtual Clock *createClock();
        virtual bool sanityCheck();
};

class StateSystemFactory : public ObjectFactory {
    public:
        StateSystemFactory();
        virtual ~StateSystemFactory();
        virtual StateSystem *createStateSystem();
        virtual bool sanityCheck();
};

#endif