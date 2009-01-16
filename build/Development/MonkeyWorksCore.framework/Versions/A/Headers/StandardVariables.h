#ifndef STANDARD_VARIABLES_H
#define STANDARD_VARIABLES_H

/*
 *  StandardVariables.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Wed Jan 08 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include "GenericVariable.h"
#include "VariableRegistry.h"
namespace mw {
// Stimuli ======================================================   
#define ANNOUNCE_STIMULUS_TAGNAME       "#announceStimulus"


    // fields of stimulus announce Dictionary vars (not all stim announces contain all fields)
    #define STIM_NAME   "name"
    #define STIM_TYPE   "type"     
    #define STIM_ACTION "action"
    #define STIM_POSX   "pos_x"
    #define STIM_POSY   "pos_y"    
    #define STIM_SIZEX  "size_x"
    #define STIM_SIZEY  "size_y"
    #define STIM_ROT    "rotation"
    #define STIM_COLOR_R    "color_r"
    #define STIM_COLOR_G    "color_g"
    #define STIM_COLOR_B    "color_b"
    #define STIM_FILENAME   "filename"
    #define STIM_GROUP_NAME  "group"
    #define STIM_GROUP_INDEX "groupIndex"

        // stimulus types (STIM_TYPE above)
        #define STIM_TYPE_GENERIC           "generic"   
        #define STIM_TYPE_BASICTRANSFORM    "generic"
        #define STIM_TYPE_IMAGE             "image"
        #define STIM_TYPE_POINT             "point"
        #define STIM_TYPE_BLANK             "blankscreen"

        // stimulus actions (STIM_ACTION above)
        #define STIM_ACTION_DRAW            "draw"
        #define STIM_ACTION_ERASE           "erase"



// Sounds ===========================================================
#define ANNOUNCE_SOUND_TAGNAME			"#announceSound"

	#define SOUND_NAME	"name"
	#define SOUND_FILENAME	"filename"



// Calibrators ======================================================  
#define ANNOUNCE_CALIBRATOR_TAGNAME    "#announceCalibrator"

   // fields of calibrator announce
    #define CALIBRATOR_NAME         "name"
    #define CALIBRATOR_ACTION       "action"
    #define CALIBRATOR_PARAMS_H     "params_H"
    #define CALIBRATOR_PARAMS_V     "params_V"
    
        // calibrator announce actions
        #define CALIBRATOR_ACTION_SAMPLE           "sample"
        #define CALIBRATOR_ACTION_UPDATE_PARAMS    "newParams"
        #define CALIBRATOR_ACTION_IDENTIFY_SELF    "identifySelf"

        // eye calibrator fields associated with "sample" announcement
        #define CALIBRATOR_SAMPLE_SAMPLED_HV    "sampledHV"
        #define CALIBRATOR_SAMPLE_DESIRED_HV    "desiredHV"
        #define CALIBRATOR_SAMPLE_CALIBRATED_HV "calibratedHV"  


// Triggers ======================================================  
#define ANNOUNCE_TRIGGER_TAGNAME        "#announceTrigger"

    // Sept 2006 -- JJD note: these are currently only implemented through the 
    //          stim announce (fixation point)
    #define TRIGGER_CENTERX             "center_x"
    #define TRIGGER_CENTERY             "center_y"
    #define TRIGGER_WIDTH               "width"


// ===============================================================

// Request variables are used by the client to  
#define REQUEST_CALIBRATOR_TAGNAME     "#requestCalibrator"

   // fields calibrator request
    #define R_CALIBRATOR_NAME       "name"
    #define R_CALIBRATOR_ACTION     "action"
    #define R_CALIBRATOR_PARAMS_H   "params_H"
    #define R_CALIBRATOR_PARAMS_V   "params_V"
    
    // stimulus actions (R_CALIBRATOR_ACTION above)
    #define R_CALIBRATOR_ACTION_SET_PARAMETERS                 "set_params"
    #define R_CALIBRATOR_ACTION_SET_PARAMETERS_TO_DEFAULTS     "set_params_default"
    

// private data
#define PRIVATE_CALIBRATOR_TAGNAME     "#privateCalibrator"


// ======================================================  
// defines for state system modes
enum {IDLE, STOPPING, RUNNING, TASKMODES};	



#define PRIVATE_SYSTEM_VARIABLES "# SYSTEM VARIABLES"


extern shared_ptr<Variable> task_mode;

#define TASK_MODE_TAGNAME "#task_mode"

#define STIMULUS_DISPLAY_UPDATE_TAGNAME "#stimDisplayUpdate"
extern shared_ptr<Variable> stimDisplayUpdate;

extern shared_ptr<Variable> GlobalMessageVariable;
#define ANNOUNCE_MESSAGE_VAR_TAGNAME "#announceMessage"

extern shared_ptr<Variable> GlobalSystemEventVariable;
#define SYSTEM_VAR_TAGNAME "#system"


#define ANNOUNCE_CURRENT_STATE_TAGNAME	"#announceCurrentState"
extern shared_ptr<Variable> currentState;

#define ANNOUNCE_TRIAL_TAGNAME	"#annouceTrial"
#define ANNOUNCE_BLOCK_TAGNAME	"#announceBlock"

extern shared_ptr<Variable> trialAnnounce;
extern shared_ptr<Variable> blockAnnounce;

#define ANNOUNCE_ASSERTION_TAGNAME "#announceAssertion"
extern shared_ptr<Variable> assertionFailure;

#define SETUP_NAME_TAGNAME	"#serverName"
extern shared_ptr<Variable> serverName;


#define EXPERIMENT_LOAD_PROGRESS_TAGNAME	"#experimentLoadProgress"
extern shared_ptr<Variable> experimentLoadProgress;

extern shared_ptr<Variable> beamPosition;		// DDC added as an experiment, Aug 2006

/*        sampleSource,
        spikeSource,
        leverSource,
        dataFileOpen,
        writingToDisk,
        eyePosHRaw,
        eyePosVRaw,
        eyePosHDeg,
        eyePosVDeg,
        trialStart,
        trialEnd,
        spikeZero,
        myEyePosRawX,
        myEyePosRawY;*/

#define	MAIN_SCREEN_INFO_TAGNAME	"#mainScreenInfo"

#define M_DISPLAY_WIDTH_KEY					"width"
#define M_DISPLAY_HEIGHT_KEY				"height"
#define M_DISPLAY_DISTANCE_KEY				"distance"
#define M_DISPLAY_TO_USE_KEY				"display_to_use"
#define M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY	"always_display_mirror_window"
#define M_MIRROR_WINDOW_BASE_HEIGHT_KEY		"mirror_window_base_height"
#define M_REFRESH_RATE_KEY					"refresh_rate_hz"
extern shared_ptr<Variable> mainDisplayInfo;		


// Debugger variables
#define DEBUGGER_ACTIVE_TAGNAME "debuggerActive"
extern shared_ptr<Variable> debuggerActive;

#define DEBUGGER_RUNNING_TAGNAME "debuggerRunning"
extern shared_ptr<Variable> debuggerRunning;

#define DEBUGGER_STEP_TAGNAME "debuggerStep"
extern shared_ptr<Variable> debuggerStep;

        
void initializeStandardVariables(shared_ptr<VariableRegistry> registry);
}
#endif STANDARD_VARIABLES_H

