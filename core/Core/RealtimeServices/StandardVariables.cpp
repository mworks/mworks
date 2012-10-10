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
#include "ConstantVariable.h"

#include "TrialBuildingBlocks.h"

#include "Announcers.h"

#include "Debugging.h"


BEGIN_NAMESPACE_MW


	shared_ptr<Variable> state_system_mode; 
	shared_ptr<Variable> GlobalMessageVariable; 
	shared_ptr<Variable> GlobalSystemEventVariable; 
	shared_ptr<Variable> stimDisplayUpdate;   // JJD added June 2006
	shared_ptr<Variable> beamPosition;		// DDC added as an experiment, Aug 2006
	shared_ptr<Variable> mainDisplayInfo;
    shared_ptr<Variable> warnOnSkippedRefresh;
	shared_ptr<Variable> currentState;
	
	shared_ptr<Variable> trialAnnounce;
	shared_ptr<Variable> blockAnnounce;
	
	shared_ptr<Variable> assertionFailure;
	
	shared_ptr<Variable> serverName;
	
	shared_ptr<Variable> debuggerActive;
	shared_ptr<Variable> debuggerRunning;
	shared_ptr<Variable> debuggerStep;
	
	shared_ptr<Variable> experimentLoadProgress;
    
    shared_ptr<Variable> alt_failover;

	
	void initializeStandardVariables(shared_ptr<VariableRegistry> registry) {
		
        
        alt_failover = registry->createGlobalVariable( new VariableProperties( new Datum((bool)true), 
                                                                                   ALT_FAILOVER_TAGNAME, 
                                                                                   "allow_alt_failover", 
                                                                                   "Allow failover to 'alt' defined objects", 
                                                                                   M_WHEN_CHANGED,
                                                                                   M_WHEN_CHANGED, 
                                                                                   true, 
                                                                                   false, 
                                                                                   M_DISCRETE_BOOLEAN,
                                                                                   PRIVATE_SYSTEM_VARIABLES));
                                                                                   
		state_system_mode = registry->createGlobalVariable( new VariableProperties(
																			new Datum((long)IDLE), 
																			STATE_SYSTEM_MODE_TAGNAME, 
																			"Task Mode", 
																			"Current Task Mode", 
																			M_WHEN_CHANGED,
																			M_WHEN_CHANGED, 
																			true, 
																			false, 
																			M_INTEGER_INFINITE,
																			PRIVATE_SYSTEM_VARIABLES));
		
		
		
		
		GlobalMessageVariable = registry->createGlobalVariable(new VariableProperties(new Datum((long)0),
																					   ANNOUNCE_MESSAGE_VAR_TAGNAME,
																					   "message", 
																					   "message event channel",
																					   M_NEVER,			// never user edit
																					   M_WHEN_CHANGED,		// log when changed
																					   true, 
																					   false, 
																					   M_STRUCTURED,
																					   PRIVATE_SYSTEM_VARIABLES));  // view                                       
		
		
		stimDisplayUpdate = registry->createGlobalVariable(
														   new VariableProperties(
																				   new Datum(M_LIST, 1),
																				   STIMULUS_DISPLAY_UPDATE_TAGNAME, 
																				   "Stimulus Display Update",
																				   "Stimulus Display about to be Updated", 
																				   M_NEVER,			// never user edit
																				   M_WHEN_CHANGED,		// log when changed
																				   true, 
																				   false, 
																				   M_DISCRETE_BOOLEAN,
																				   PRIVATE_SYSTEM_VARIABLES));  // view                                       
		
		beamPosition = registry->createGlobalVariable(
													  new VariableProperties(
																			  new Datum((long)0),
																			  "#beamPosition", 
																			  "Main stimulus display beam position",
																			  "Main stimulus display beam position", 
																			  M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, 
																			  M_INTEGER_FINITE,
																			  PRIVATE_SYSTEM_VARIABLES));
		
		
		experimentLoadProgress = registry->createGlobalVariable(
																new VariableProperties(
																						new Datum((double)0),
																						EXPERIMENT_LOAD_PROGRESS_TAGNAME, 
																						"Progress while loading an experiment",
																						"Progress while loading an experiment", 
																						M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, 
																						M_CONTINUOUS_FINITE,
																						PRIVATE_SYSTEM_VARIABLES));
		
		
		// any stimulus object will announce it appearance and disappearance through this variable
		//  when they appear/disappear on the screen (open GL), they will be asked to announce
		//    values by changing this variable
		registry->createGlobalVariable(
									   new VariableProperties(
															   new Datum((long)0),       // constructor to create default
															   ANNOUNCE_STIMULUS_TAGNAME, "Announce stimulus",
															   "A stimulus change has occured", 
															   M_NEVER,M_WHEN_CHANGED,     // never useer edit, log when changed
															   true, false, M_STRUCTURED,
															   PRIVATE_SYSTEM_VARIABLES));  // view                                       
		
		
		registry->createGlobalVariable(
									   new VariableProperties(
															   new Datum((long)0),       // constructor to create default
															   ANNOUNCE_SOUND_TAGNAME, "Announce Sound",
															   "Indicates that a sound was played", 
															   M_NEVER,M_WHEN_CHANGED,     // never useer edit, log when changed
															   true, false, M_STRUCTURED,
															   PRIVATE_SYSTEM_VARIABLES));  // view   
		
		
		registry->createGlobalVariable(
									   new VariableProperties(
															   new Datum(M_DICTIONARY, (int)1),       // constructor to create default
															   ANNOUNCE_CALIBRATOR_TAGNAME, "Announce calibration",
															   "A calibration event has occured", 
															   M_NEVER,M_WHEN_CHANGED,     // never useer edit, log when changed
															   true, false, M_STRUCTURED,
															   PRIVATE_SYSTEM_VARIABLES));  // view                                     
		
		registry->createGlobalVariable(
									   new VariableProperties(
															   new Datum(M_DICTIONARY, (int)1),
															   REQUEST_CALIBRATOR_TAGNAME, "Request calibrator",
															   "used to request calibrator actions",       // e.g. update parameters
															   M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
															   PRIVATE_SYSTEM_VARIABLES));                           
		
		
		
		currentState = registry->createGlobalVariable(new VariableProperties(new Datum(std::string("<empty>")),
																			  ANNOUNCE_CURRENT_STATE_TAGNAME,
																			  "Current State",
																			  "reports the current state",   
																			  M_WHEN_CHANGED,
																			  M_WHEN_CHANGED, 
																			  true, 
																			  false, 
																			  M_DISCRETE,
																			  PRIVATE_SYSTEM_VARIABLES));                           
		// Hooks to report state progression conveniently to the console
		// #define ANNOUNCE_CURRENT_STATE
#ifdef	ANNOUNCE_CURRENT_STATE
		shared_ptr<ReportString> r(new ReportString("currentState = $#announceCurrentState"));
		shared_ptr<ActionVariableNotification> note(new ActionVariableNotification(r));
		currentState->addNotification(note);
#endif
		
		trialAnnounce = registry->createGlobalVariable(
													   new VariableProperties(
																			   new Datum(0L),
																			   ANNOUNCE_TRIAL_TAGNAME, 
																			   "Trial Announce",
																			   "reports the entry or exit of a trial paradigm component",       // e.g. update parameters
																			   M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
																			   PRIVATE_SYSTEM_VARIABLES));                           
		
		blockAnnounce = registry->createGlobalVariable(new VariableProperties(
																			   new Datum(0L),
																			   ANNOUNCE_BLOCK_TAGNAME, "Block Announce",
																			   "reports the entry or exit of a block paradigm component",       // e.g. update parameters
																			   M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
																			   PRIVATE_SYSTEM_VARIABLES));                           
		
		
		assertionFailure = registry->createGlobalVariable(
														  new VariableProperties(
																				  new Datum(0L),
																				  ANNOUNCE_ASSERTION_TAGNAME, "Assertion Failure",
																				  "Reports when a debugging assertion fails",
																				  M_WHEN_CHANGED,M_WHEN_CHANGED, true, false, M_DISCRETE,
																				  PRIVATE_SYSTEM_VARIABLES));  
		
		
		serverName = registry->createGlobalVariable(
												   new VariableProperties(
																		   new Datum(""),
																		   SETUP_NAME_TAGNAME,
																		   "The name of this server",
																		   "A unique name to distinguish this server machine (\"setup\") from others",
																		   M_WHEN_CHANGED, M_WHEN_CHANGED,
																		   true, false, M_STRUCTURED, PRIVATE_SYSTEM_VARIABLES));
		
		
        Datum default_screen_info;
		mainDisplayInfo = registry->createGlobalVariable(new VariableProperties(&default_screen_info,
																				MAIN_SCREEN_INFO_TAGNAME, 
																				"Main Screen Geometry Information",
																				"Used to convert from screen units into degrees",
																				M_WHEN_CHANGED, M_WHEN_CHANGED, true, false, M_STRUCTURED,
																				PRIVATE_SYSTEM_VARIABLES));
        
        
        warnOnSkippedRefresh = registry->createGlobalVariable(new VariableProperties(new Datum((bool)true), 
                                                                                     WARN_ON_SKIPPED_REFRESH_TAGNAME, 
                                                                                     "Warn On Skipped Refresh", 
                                                                                     "Issue a warning if stimulus display driver misses a display refresh cycle", 
                                                                                     M_WHEN_CHANGED,
                                                                                     M_WHEN_CHANGED, 
                                                                                     true, 
                                                                                     false, 
                                                                                     M_DISCRETE_BOOLEAN,
                                                                                     PRIVATE_SYSTEM_VARIABLES));
		
		
		debuggerActive = registry->createGlobalVariable(new VariableProperties(new Datum(0L),
																				DEBUGGER_ACTIVE_TAGNAME, 
																				"Debugger Active",
																				"When true, engages state system debugger",
																				M_WHEN_CHANGED, M_WHEN_CHANGED, true, false, M_INTEGER_INFINITE,
																				PRIVATE_SYSTEM_VARIABLES));
		
		shared_ptr<VariableNotification> active_note(new DebuggerActiveNotification());
		debuggerActive->addNotification(active_note);
		
		
		debuggerRunning = registry->createGlobalVariable(
														 new VariableProperties(
																				 new Datum(1L),
																				 DEBUGGER_RUNNING_TAGNAME, 
																				 "Debugger Running",
																				 "When true, allows the state system to move freely",
																				 M_WHEN_CHANGED, M_WHEN_CHANGED, true, false, M_INTEGER_INFINITE,
																				 PRIVATE_SYSTEM_VARIABLES));
		
		debuggerStep = registry->createGlobalVariable(
													  new VariableProperties(
																			  new Datum(0L),
																			  DEBUGGER_STEP_TAGNAME, 
																			  "Debugger Step",
																			  "When non-zero, allows state system to move.  Each state "
																			  "allowed to move decrements this variable's value",
																			  M_WHEN_CHANGED, M_WHEN_CHANGED, true, false, M_INTEGER_INFINITE,
																			  PRIVATE_SYSTEM_VARIABLES));
		
		
	}


END_NAMESPACE_MW
