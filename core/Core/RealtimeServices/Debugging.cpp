/*
 *  Debugging.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Debugging.h"


#include "Clock.h"
#include "StandardVariables.h"


BEGIN_NAMESPACE_MW


bool debugger_enabled;


// Stall or proceed according to the settings of the debugging variables
void debuggerCheck(){
    
    if(!debuggerRunning->getValue()){
        
        while(!debuggerRunning->getValue()){
            if((long)(debuggerStep->getValue()) > 0){
                debuggerStep->setValue((long)debuggerStep->getValue() - 1);
                break;
            }
            shared_ptr <Clock> clock = Clock::instance();
            clock->sleepMS(50);
        }
    }
}


END_NAMESPACE_MW
