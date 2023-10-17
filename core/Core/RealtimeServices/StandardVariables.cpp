/**
 * StandardVariables.cpp
 * 
 * History:
 * David Cox on Wed Jan 08 2003 - Created.
 * Paul Jankunas on 02/11/05 - Fixed Spacing.
 *
 * Copyright (c) 2003 MIT. All rights reserved.
 */

#include "StandardVariables.h"
#include "Utilities.h"

#include "TrialBuildingBlocks.h"

#include "Announcers.h"


BEGIN_NAMESPACE_MW


	shared_ptr<Variable> state_system_mode; 
	shared_ptr<Variable> GlobalMessageVariable; 
	shared_ptr<Variable> GlobalSystemEventVariable; 
	shared_ptr<Variable> stimDisplayUpdate;   // JJD added June 2006
    shared_ptr<Variable> stimDisplayCapture;
	shared_ptr<Variable> mainDisplayInfo;
    shared_ptr<Variable> warnOnSkippedRefresh;
    shared_ptr<Variable> stopOnError;
    shared_ptr<Variable> realtimeComponents;
	shared_ptr<Variable> announceCurrentState;
	
	shared_ptr<Variable> trialAnnounce;
	shared_ptr<Variable> blockAnnounce;
	
	shared_ptr<Variable> assertionFailure;
	
	shared_ptr<Variable> serverName;

	shared_ptr<Variable> experimentLoadProgress;
    shared_ptr<Variable> loadedExperiment;

    shared_ptr<Variable> alt_failover;

	
	void initializeStandardVariables(shared_ptr<VariableRegistry> registry) {
		
        
        alt_failover = registry->createGlobalVariable(    VariableProperties(     Datum((bool)true),
                                                                                   ALT_FAILOVER_TAGNAME, 
                                                                                   "allow_alt_failover", 
                                                                                   "Allow failover to alternative I/O devices",
                                                                                   M_WHEN_CHANGED,
                                                                                   M_WHEN_CHANGED, 
                                                                                   true, 
                                                                                   false, 
                                                                                   M_DISCRETE_BOOLEAN,
                                                                                   PRIVATE_SYSTEM_VARIABLES));
                                                                                   
		state_system_mode = registry->createGlobalVariable( VariableProperties(
																			Datum((long)IDLE), 
																			STATE_SYSTEM_MODE_TAGNAME, 
																			"Task Mode", 
																			"Reports the execution state of the current experiment",
																			M_WHEN_CHANGED,
																			M_WHEN_CHANGED, 
																			true, 
																			false, 
																			M_INTEGER_INFINITE,
																			PRIVATE_SYSTEM_VARIABLES));
		
		
		
		
		GlobalMessageVariable = registry->createGlobalVariable(VariableProperties(Datum((long)0),
																					   ANNOUNCE_MESSAGE_VAR_TAGNAME,
																					   "message", 
																					   "Used to log messages to the event stream and event file",
																					   M_NEVER,			// never user edit
																					   M_WHEN_CHANGED,		// log when changed
																					   true, 
																					   false, 
																					   M_STRUCTURED,
																					   PRIVATE_SYSTEM_VARIABLES));  // view                                       
		
		
		stimDisplayUpdate = registry->createGlobalVariable(
														   VariableProperties(
																				   Datum(M_LIST, 1),
																				   STIMULUS_DISPLAY_UPDATE_TAGNAME, 
																				   "Stimulus Display Update",
																				   "Reports updates to the visual stimulus presentation",
																				   M_NEVER,			// never user edit
																				   M_WHEN_CHANGED,		// log when changed
																				   true, 
																				   false, 
																				   M_DISCRETE_BOOLEAN,
																				   PRIVATE_SYSTEM_VARIABLES));  // view                                       
		
		
        stimDisplayCapture = registry->createGlobalVariable(VariableProperties(Datum(""),
                                                                               STIMULUS_DISPLAY_CAPTURE_TAGNAME,
                                                                               M_WHEN_CHANGED,
                                                                               false,
                                                                               PRIVATE_SYSTEM_VARIABLES,
                                                                               false,
                                                                               "Used to record captured stimulus display frames"));
        
        
		experimentLoadProgress = registry->createGlobalVariable(
																VariableProperties(
																						Datum((double)0),
																						EXPERIMENT_LOAD_PROGRESS_TAGNAME, 
																						"Progress while loading an experiment",
																						"Reports experiment loading progress",
																						M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, 
																						M_CONTINUOUS_FINITE,
																						PRIVATE_SYSTEM_VARIABLES));
		
		
		loadedExperiment = registry->createGlobalVariable(
                                                          VariableProperties(
                                                                                 Datum(""),
                                                                                 LOADED_EXPERIMENT_TAGNAME,
                                                                                 "Source code of current experiment",
                                                                                 "Used to record the source file(s) of the currently-loaded experiment to the event stream and event file",
                                                                                 M_WHEN_CHANGED, M_WHEN_CHANGED,
                                                                                 true, false, M_STRUCTURED, PRIVATE_SYSTEM_VARIABLES));
		
		
		registry->createGlobalVariable(
									   VariableProperties(
															   Datum((long)0),       // constructor to create default
															   ANNOUNCE_SOUND_TAGNAME, "Announce Sound",
															   "Reports sound-related actions",
															   M_NEVER,M_WHEN_CHANGED,     // never useer edit, log when changed
															   true, false, M_STRUCTURED,
															   PRIVATE_SYSTEM_VARIABLES));  // view   
		
		
		registry->createGlobalVariable(
									   VariableProperties(
															   Datum(M_DICTIONARY, (int)1),       // constructor to create default
															   ANNOUNCE_CALIBRATOR_TAGNAME, "Announce calibration",
															   "Reports changes to the state of a calibrator",
															   M_NEVER,M_WHEN_CHANGED,     // never useer edit, log when changed
															   true, false, M_STRUCTURED,
															   PRIVATE_SYSTEM_VARIABLES));  // view                                     
		
		registry->createGlobalVariable(
									   VariableProperties(
															   Datum(M_DICTIONARY, (int)1),
															   REQUEST_CALIBRATOR_TAGNAME, "Request calibrator",
															   "Used to request updates to a calibrator’s parameters",
															   M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
															   PRIVATE_SYSTEM_VARIABLES));                           
		
		
		
		announceCurrentState = registry->createGlobalVariable(VariableProperties(Datum(std::string("<empty>")),
																			  ANNOUNCE_CURRENT_STATE_TAGNAME,
																			  "Current State",
																			  "Reports the numeric identifier of the component that is currently executing",
																			  M_WHEN_CHANGED,
																			  M_WHEN_CHANGED, 
																			  true, 
																			  false, 
																			  M_DISCRETE,
																			  PRIVATE_SYSTEM_VARIABLES));                           
		
		trialAnnounce = registry->createGlobalVariable(
													   VariableProperties(
																			   Datum(0L),
																			   ANNOUNCE_TRIAL_TAGNAME, 
																			   "Trial Announce",
																			   "Reports the entry or exit of a trial paradigm component",
																			   M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
																			   PRIVATE_SYSTEM_VARIABLES));                           
		
		blockAnnounce = registry->createGlobalVariable(VariableProperties(
																			   Datum(0L),
																			   ANNOUNCE_BLOCK_TAGNAME, "Block Announce",
																			   "Reports the entry or exit of a block paradigm component",
																			   M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
																			   PRIVATE_SYSTEM_VARIABLES));                           
		
		
		assertionFailure = registry->createGlobalVariable(
														  VariableProperties(
																				  Datum(0L),
																				  ANNOUNCE_ASSERTION_TAGNAME, "Assertion Failure",
																				  "Reports when a debugging assertion fails",
																				  M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
																				  PRIVATE_SYSTEM_VARIABLES));  
		
		
		serverName = registry->createGlobalVariable(
												   VariableProperties(
																		   Datum(""),
																		   SETUP_NAME_TAGNAME,
																		   "The name of this server",
																		   "A unique name to distinguish this server machine (\"setup\") from others",
																		   M_WHEN_CHANGED, M_WHEN_CHANGED,
																		   true, false, M_STRUCTURED, PRIVATE_SYSTEM_VARIABLES));
		
		
        Datum default_screen_info(M_DICTIONARY, 0);
		mainDisplayInfo = registry->createGlobalVariable(VariableProperties(default_screen_info,
																				MAIN_SCREEN_INFO_TAGNAME, 
																				"Main Screen Geometry Information",
																				"Used to configure the stimulus display",
																				M_WHEN_CHANGED, M_WHEN_CHANGED, true, false, M_STRUCTURED,
																				PRIVATE_SYSTEM_VARIABLES));
        
        
        warnOnSkippedRefresh = registry->createGlobalVariable(VariableProperties(Datum((bool)true), 
                                                                                     WARN_ON_SKIPPED_REFRESH_TAGNAME, 
                                                                                     "Warn On Skipped Refresh", 
                                                                                     "Issue a warning if the stimulus presentation skips a display refresh cycle",
                                                                                     M_WHEN_CHANGED,
                                                                                     M_WHEN_CHANGED, 
                                                                                     true, 
                                                                                     false, 
                                                                                     M_DISCRETE_BOOLEAN,
                                                                                     PRIVATE_SYSTEM_VARIABLES));
		
		
        stopOnError = registry->createGlobalVariable(VariableProperties(Datum(false),
                                                                        STOP_ON_ERROR_TAGNAME,
                                                                        M_WHEN_CHANGED,
                                                                        false,
                                                                        PRIVATE_SYSTEM_VARIABLES,
                                                                        false,
                                                                        "Stop the experiment automatically when an error is reported"));
        
        
        Datum defaultRealtimeComponents(M_DICTIONARY, 0);
        realtimeComponents = registry->createGlobalVariable(VariableProperties(defaultRealtimeComponents,
                                                                                   "#realtimeComponents",
                                                                                   "Realtime Component Selections",
                                                                                   "Used to select clock, scheduler, and state system components",
                                                                                   M_WHEN_CHANGED, M_WHEN_CHANGED, true, false, M_STRUCTURED,
                                                                                   PRIVATE_SYSTEM_VARIABLES));
		
		
	}


END_NAMESPACE_MW
