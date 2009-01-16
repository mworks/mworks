/*
 *  ServiceTypes.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Thu Jan 02 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 *
 *  This header defines the basic types of objects that one can create from within
 *  a plugin.  If a type of class isn't in here, you can't extended it without recompiling
 *  the core.
 */



enum ServiceType{  M_UNRESOLVED_SERVICE, 
                    
                    // stimuli
                    M_STIMULUS_COMPONENT, 
                    M_STIMULUS_DISPLAY_COMPONENT,
                    
                    // paradigm
                    M_STATE_COMPONENT, 
                    M_PROTOCOL_COMPONENT,
					M_BLOCK_COMPONENT,
					M_TRIAL_COMPONENT,
					M_TRIALSTATE_COMPONENT, 
                    M_SELECTION_COMPONENT, 
                    M_ACTION_COMPONENT,
                    M_TRANSITION_COMPONENT,
                    
                    // realtime
                    M_CLOCK_COMPONENT, 
                    M_SCHEDULER_COMPONENT, M_SCHEDULER_NODE_COMPONENT,
                    M_STATE_SYSTEM_COMPONENT,
                    
                    // event buffer
                    M_EVENT_COMPONENT,
                    M_EVENT_BUFFER_COMPONENT,
                    M_EVENT_BUFFER_READER_COMPONENT,
                    
                    // io
                    M_IO_DEVICE_COMPONENT,
                    
                    M_EVENT_CONSUMER_COMPONENT,
                    M_DISPLAY_TOOL_COMPONENT, M_DISPLAY_HOST_MONITOR_COMPONENT,
                    M_DISPLAY_REALTIME_MONITOR_COMPONENT, M_N_SERVICE_TYPES
                    };
                    
ServiceType lookUpServiceType(char *name);
