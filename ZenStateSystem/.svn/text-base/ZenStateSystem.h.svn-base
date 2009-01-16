/*
 *  ZenStateSystem.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

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
using namespace mw;

class StandardStateSystem : public StateSystem {
    public:
    
        StandardStateSystem(const shared_ptr <Clock> &a_clock);
    
        virtual void start();
        virtual void stop();
        virtual void pause();
        
        virtual bool isRunning();
        virtual bool isPaused();
        
        virtual bool isInAction();
        virtual bool isInTransition();
        
        virtual void setInAction(bool);
        virtual void setInTransition(bool);
        
        weak_ptr<State> getCurrentState();
        void setCurrentState(weak_ptr<State> new_current);
};

#endif
