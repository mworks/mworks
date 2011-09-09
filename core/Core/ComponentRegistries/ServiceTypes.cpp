#include "ServiceTypes.h"
#include <string.h>


/*

 // stimuli
                    M_STIMULUS_COMPONENT, 
                    M_STIMULUS_DISPLAY_COMPONENT,
                    
                    // paradigm
                    M_STATE_COMPONENT, 
                    M_PROTOCOL_COMPONENT, 
                    M_SELECTION_COMPONENT, 
                    M_ACTION_COMPONENT,
                    M_TRANSITION_COMPONENT,
                    
                    // realtime
                    M_CLOCK_COMPONENT, 
                    M_SCHEDULER_COMPONENT, M_SCHEDULER_NODE_COMPONENT,
                    M_STATE_SYSTEM_COMPONENT,

*/


ServiceType lookUpServiceType(char *name){

    if(strcmp(name, "State") == 0){
        return M_STATE_COMPONENT;
    
    } else if(strcmp(name, "Protocol") == 0){
        return M_PROTOCOL_COMPONENT;
    
    } else if(strcmp(name, "Selection") == 0){
        return M_SELECTION_COMPONENT;
    
    } else if(strcmp(name, "Action") == 0){
        return M_ACTION_COMPONENT;
    
    } else if(strcmp(name, "Transition") == 0){
        return M_TRANSITION_COMPONENT;
    
    } else if(strcmp(name, "Clock") == 0){
        return M_CLOCK_COMPONENT;
    
    } else if(strcmp(name, "Scheduler") == 0){
        return M_SCHEDULER_COMPONENT;
    
    } else if(strcmp(name, "SchedulerNode") == 0){
        return M_SCHEDULER_NODE_COMPONENT;
        
    } else if(strcmp(name, "StateSystem") == 0){
        return M_STATE_SYSTEM_COMPONENT;
    }
    

    return M_UNRESOLVED_SERVICE;

}