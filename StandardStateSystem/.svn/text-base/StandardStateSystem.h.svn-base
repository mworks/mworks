#ifndef STANDARD_STATE_SYSTEM_H
#define STANDARD_STATE_SYSTEM_H


/*
 *  StandardStateSystem.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Mon Dec 30 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */


#include "MonkeyWorksCore/StateSystem.h"
#include "MonkeyWorksCore/Plugin.h"


///////////////////////////////////////////////////////////////////////////////////////
// Plugin entry point
//
// The extern "C" bit ensures that the name of this function is not mangled (C++
// requires name-mangling in order to achieve function overriding.  Our plugin host
// needs to know what name to look for, and for this plugin architecture, that name
// is 'getPlugin'
///////////////////////////////////////////////////////////////////////////////////////

extern "C"{
    Plugin *getPlugin();
}



class StandardStateSystem : public StateSystem, public StateSystemFactory {

    public:
    
        StandardStateSystem();
    
        virtual void start();
        virtual void stop();
        virtual void pause();
        
        virtual bool isRunning();
        virtual bool isPaused();
        
        virtual bool isInAction();
        virtual bool isInTransition();
        
        virtual void setInAction(bool);
        virtual void setInTransition(bool);
        
        State *getCurrentState();
        void setCurrentState(State *);
        
        StateSystem *createStateSystem(){
            return (StateSystem *)(new StandardStateSystem());
        }
};


class StandardStateSystemPlugin : public Plugin {
    
    
    virtual ExpandableList<ServiceDescription> *getServiceDescriptions(){
    
        ExpandableList<ServiceDescription> *list = new ExpandableList<ServiceDescription>();
        
        // only one service in this plugin (just the scheduler), but a plugin could, in principle,
        // contain a whole suite of related services.
        
        list->addElement(new ServiceDescription("StandardStateSystem", (ObjectFactory *)(new StandardStateSystem()), M_STATE_SYSTEM_COMPONENT));
    
        return list;
    }
};



#endif

