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


BEGIN_NAMESPACE_MW


// Stimuli ======================================================   


    // fields of stimulus announce Dictionary vars (not all stim announces contain all fields)
    #define STIM_NAME   "name"
    #define STIM_TYPE   "type"     
    #define STIM_ACTION "action"
    #define STIM_POSX   "pos_x"
    #define STIM_POSY   "pos_y"    
    #define STIM_SIZEX  "size_x"
    #define STIM_SIZEY  "size_y"
    #define STIM_ROT    "rotation"
    #define STIM_ALPHA  "alpha_multiplier"
    #define STIM_FULLSCREEN "fullscreen"
    #define STIM_COLOR_R    "color_r"
    #define STIM_COLOR_G    "color_g"
    #define STIM_COLOR_B    "color_b"
    #define STIM_FILENAME   "filename"
    #define STIM_FILE_HASH  "file_hash"
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
enum {IDLE, STOPPING, RUNNING, PAUSED, TASKMODES};	



#define PRIVATE_SYSTEM_VARIABLES "# SYSTEM VARIABLES"
#define EXPERIMENT_DEFINED_VARIABLES "# EXPERIMENT VARIABLES"


extern shared_ptr<Variable> state_system_mode;

#define STATE_SYSTEM_MODE_TAGNAME "#state_system_mode"

#define STIMULUS_DISPLAY_UPDATE_TAGNAME "#stimDisplayUpdate"
extern shared_ptr<Variable> stimDisplayUpdate;

#define STIMULUS_DISPLAY_CAPTURE_TAGNAME "#stimDisplayCapture"
extern shared_ptr<Variable> stimDisplayCapture;

extern shared_ptr<Variable> GlobalMessageVariable;
#define ANNOUNCE_MESSAGE_VAR_TAGNAME "#announceMessage"


#define ANNOUNCE_CURRENT_STATE_TAGNAME	"#announceCurrentState"
extern shared_ptr<Variable> currentState;

#define ANNOUNCE_TRIAL_TAGNAME	"#announceTrial"
#define ANNOUNCE_BLOCK_TAGNAME	"#announceBlock"

extern shared_ptr<Variable> trialAnnounce;
extern shared_ptr<Variable> blockAnnounce;

#define ANNOUNCE_ASSERTION_TAGNAME "#announceAssertion"
extern shared_ptr<Variable> assertionFailure;

#define SETUP_NAME_TAGNAME	"#serverName"
extern shared_ptr<Variable> serverName;


#define EXPERIMENT_LOAD_PROGRESS_TAGNAME	"#experimentLoadProgress"
extern shared_ptr<Variable> experimentLoadProgress;

#define LOADED_EXPERIMENT_TAGNAME  "#loadedExperiment"
extern shared_ptr<Variable> loadedExperiment;

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

#define	MAIN_SCREEN_INFO_TAGNAME            "#mainScreenInfo"
#define M_DISPLAY_WIDTH_KEY					"width"
#define M_DISPLAY_HEIGHT_KEY				"height"
#define M_DISPLAY_DISTANCE_KEY				"distance"
#define M_DISPLAY_TO_USE_KEY				"display_to_use"
#define M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY	"always_display_mirror_window"
#define M_MIRROR_WINDOW_BASE_HEIGHT_KEY		"mirror_window_base_height"
#define M_REFRESH_RATE_KEY					"refresh_rate_hz"
#define M_USE_COLOR_MANAGEMENT_KEY          "use_color_management"
#define M_SET_DISPLAY_GAMMA_KEY             "set_display_gamma"
#define M_DISPLAY_GAMMA_RED_KEY             "red_gamma"
#define M_DISPLAY_GAMMA_GREEN_KEY           "green_gamma"
#define M_DISPLAY_GAMMA_BLUE_KEY            "blue_gamma"
#define M_MAKE_WINDOW_OPAQUE_KEY            "make_window_opaque"
extern shared_ptr<Variable> mainDisplayInfo;

#define WARN_ON_SKIPPED_REFRESH_TAGNAME "#warnOnSkippedRefresh"
extern shared_ptr<Variable> warnOnSkippedRefresh;

#define STOP_ON_ERROR_TAGNAME "#stopOnError"
extern shared_ptr<Variable> stopOnError;

#define ALT_FAILOVER_TAGNAME "#allowAltFailover"
extern shared_ptr<Variable> alt_failover;

#define M_REALTIME_CLOCK_KEY        "clock"
#define M_REALTIME_SCHEDULER_KEY    "scheduler"
#define M_REALTIME_STATE_SYSTEM_KEY "state_system"
extern shared_ptr<Variable> realtimeComponents;

        
void initializeStandardVariables(shared_ptr<VariableRegistry> registry);


END_NAMESPACE_MW


#endif /* STANDARD_VARIABLES_H */

