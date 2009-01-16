/*
 *  EyeStatus.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 8/9/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "EyeMonitors.h"
#include "ComponentRegistry_new.h"
#include <boost/lexical_cast.hpp>
using namespace mw;

// all of these units should be assumed to be arbitrary, even though they many are tagged
//      by the term "deg"  TODO -- drop all the "deg" refs


// === EyeStatusMonitor is a very specific type of var transform adaptor  ======
// the input data is expected to be calibrated eye data

EyeStatusMonitor::EyeStatusMonitor(shared_ptr<Variable> _eyeHCalibratedVar, 
                    shared_ptr<Variable> _eyeVCalibratedVar, shared_ptr<Variable> _eyeStatusVar, 
                    int _filterWidthSamples) {
	
    // _filterWidthSamples is used for both the eye signale filters and the 
    //      velocity filters in the eyeStatusComputer 
    
	// pointers to the input vars -- will setup notifications
	eyeHindex = registerInput(_eyeHCalibratedVar);	// input 0
	eyeVindex = registerInput(_eyeVCalibratedVar);	// input 1
	
	// pointers to the output vars
	eyeStatusIndex = registerOutput(_eyeStatusVar);
		
    // TODO -- should we post an update to this variable immediately?
	eyeStatus = M_EYE_STATUS_UNKNOWN;	
	shared_ptr <Clock> clock = Clock::instance();
	eyeStatusTimeUS = clock->getCurrentTimeUS();  // get current time
    
	// setup boxcar filters	-- could use another filter down the road
    int filterWidthSamples = _filterWidthSamples;
    if (_filterWidthSamples<1) {
        filterWidthSamples = 1;
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Tried to create eye status monitor boxcar filters with width <1, setting to 1 (no filtering)");
    }
	filterH = new BoxcarFilter1D(filterWidthSamples);
	filterV = new BoxcarFilter1D(filterWidthSamples);
	
    int buffer_size = M_ASSUMED_EYE_SAMPLES_PER_MS * M_MAX_EYE_BUFFER_SIZE_MS; 
    pairedEyeData = new PairedEyeData(buffer_size);     // object to pair eye data

}

 
    
EyeStatusMonitor::~EyeStatusMonitor() {

	delete filterH;
	delete filterV;
    
    delete pairedEyeData;
    
}	

// flush method (e.g. when IO is about to restart)
void EyeStatusMonitor::reset() {

    lock();
    filterH->reset();
    filterV->reset();
    pairedEyeData->reset();
    eyeStatusComputer->reset();
    unlock();
}


// base class method activated when any of the registered input vars change 
// this class assumes that the eye data is already calibrated (with respect to 
//    to parameters of the saccade detection.  That is, the units are not important
//    as long as they are the same units used to instatiate the monitor
//    (e.g. saccade speed start, etc...)

void EyeStatusMonitor::newDataReceived(int inputIndex, const Data& data, MonkeyWorksTime timeUS) {

	// DDC: be careful
    //lock();
    
	double eyeDataIn = (double)data;        
	double eyeDataOut;
	MonkeyWorksTime postFilterTimeUS;
    
    //mprintf("  **** EyeStatusMonitor::newDataReceived.  inputIndex = %d, data = %f, time = %d us", 
    //            inputIndex, (double)data, (long)timeUS);
                
	// every piece of data gets boxcar filtered
	// the filters are smart enough to manage their own memory
	if (inputIndex==eyeHindex) {
			filterH->input(eyeDataIn, timeUS);
			filterH->output(&eyeDataOut, &postFilterTimeUS);
            pairedEyeData->putDataH(eyeDataOut, postFilterTimeUS);
            
	}
	else if (inputIndex==eyeVindex) {
			filterV->input(eyeDataIn, timeUS);
			filterV->output(&eyeDataOut, &postFilterTimeUS);
            pairedEyeData->putDataV(eyeDataOut, postFilterTimeUS);
            
			// DDC: experimental: stop hammer this thing so hard?
			//unlock();
			return;
	}
    else {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			" **** EyeStatusMonitor::newDataReceived  Unknown input index");
    }
	
     
    // update the eye status based on current information
    // we also update the time of the change and these times are all based 
    //  on the times sent through the notification objects.
    //  Thus, the time posted (below) represents our best estiamte of the real time 
    //  that the eye status actually changed (even if it takes additonal time delay
    //  to get through the filters and/or propogate all the way back to a variable change. 
    
    // if a pair is ready, pull it out and process it
    MonkeyWorksTime eyeTimeUS;
    double eyeH, eyeV;
    while (pairedEyeData->getAvailable(&eyeH, &eyeV, &eyeTimeUS)) {
        processAndPostEyeData(eyeH,eyeV,eyeTimeUS); 
    }    
    
    //unlock();
				
}


void EyeStatusMonitor::processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MonkeyWorksTime _eyeTimeUS) { };

void EyeStatusMonitorVer1::processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MonkeyWorksTime _eyeTimeUS) {
       
    // call a transform object to do the computation of eye status right away 
    //mprintf("  *** calling eyeStatus computer...");
    eyeStatusComputer->input(_eyeHdeg, _eyeVdeg, _eyeTimeUS);
    
    if (eyeStatusComputer->output(&eyeStatus, &eyeStatusTimeUS)) {
        // base class method to post the new eye status to the variable 
        //      --> this will trigger any notifications for these vars
        //mprintf("  *** posting results from eyeStatus computer:  eyeStatus = %d", eyeStatus);
        postResults(eyeStatusIndex, (Data)((long)eyeStatus), eyeStatusTimeUS);	
    }

}
    

void EyeStatusMonitorVer2::processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MonkeyWorksTime _eyeTimeUS) {
       
    // call a transform object to do the computation of eye status right away 
    EyeStatusComputerWithSaccades *eyeComputer =  (EyeStatusComputerWithSaccades *)eyeStatusComputer;
    
    eyeComputer->input(_eyeHdeg, _eyeVdeg, _eyeTimeUS);
    
    if (eyeComputer->output(&eyeStatus, &eyeStatusTimeUS)) {
        // base class method to post the new eye status to the variable 
        //      --> this will trigger any notifications for these vars
        //mprintf("  *** posting results from ADVANCED eyeStatus computer:  eyeStatus = %d", eyeStatus);
        postResults(eyeStatusIndex, (Data)((long)eyeStatus), eyeStatusTimeUS);	
    }
    
    // if a saccade has been fully computed, post all its details here
    // This should perhaps be an announcement variable?
    SaccadeData  saccadeData;
    if (eyeComputer->outputSaccade(&saccadeData)) {
        // descide what to post -- these vars should be registered at the time of construciton
        
    }
    

}



EyeStatusMonitorVer1::EyeStatusMonitorVer1(shared_ptr<Variable> _eyeHCalibratedVar, 
                    shared_ptr<Variable> _eyeVCalibratedVar, 
                    shared_ptr<Variable> _eyeStatusVar, 
                    int _filterWidthSamples,                     // =5
                    shared_ptr<Variable> _saccadeEntrySpeedDegPerSec,      // = 5
                    shared_ptr<Variable> _saccadeExitSpeedDegPerSec) :    // = 20
                    EyeStatusMonitor(_eyeHCalibratedVar, _eyeVCalibratedVar, 
                                        _eyeStatusVar, _filterWidthSamples) {
                                                                
    eyeStatusComputer = new EyeStatusComputer(_filterWidthSamples,
        _saccadeEntrySpeedDegPerSec,_saccadeExitSpeedDegPerSec);
}

EyeStatusMonitorVer2::EyeStatusMonitorVer2(shared_ptr<Variable> _eyeHCalibratedVar, 
                    shared_ptr<Variable> _eyeVCalibratedVar, shared_ptr<Variable> _eyeStatusVar, 
                    int _filterWidthSamples, 
                    shared_ptr<Variable> _saccadeEntrySpeedDegPerSec,     // = 5,
                    shared_ptr<Variable> _saccadeExitSpeedDegPerSec,      // = 20,
                    shared_ptr<Variable> _minimumStartDirectionChangeSeg,  // = 10,
                    shared_ptr<Variable> _minimumAmplitudeDeg,            // = 0.2,
                    shared_ptr<Variable> _minimumDurationMS) :            // = 10
                    EyeStatusMonitor(_eyeHCalibratedVar, _eyeVCalibratedVar, 
                                        _eyeStatusVar, _filterWidthSamples) {
                                                                                                                            
    eyeStatusComputer = new EyeStatusComputerWithSaccades(_filterWidthSamples,
        _saccadeEntrySpeedDegPerSec, _saccadeExitSpeedDegPerSec,
        _minimumStartDirectionChangeSeg,_minimumAmplitudeDeg,_minimumDurationMS);                            

}

	
EyeStatusMonitorVer1::~EyeStatusMonitorVer1() {
    delete eyeStatusComputer;
}

EyeStatusMonitorVer2::~EyeStatusMonitorVer2() {
    delete eyeStatusComputer;
}

 
shared_ptr<mw::Component> EyeStatusMonitorVer1Factory::createObject(std::map<std::string, std::string> parameters,
													mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, 
					   
					   "saccade_exit_speed", 
					   "saccade_entry_speed", 
					   "width_samples", 
					   "eye_state", 
					   "eyeh_calibrated", 
					   "eyev_calibrated");
	
	shared_ptr<Variable> saccade_exit_speed = reg->getVariable(parameters.find("saccade_exit_speed")->second);	
	shared_ptr<Variable> saccade_entry_speed = reg->getVariable(parameters.find("saccade_entry_speed")->second);	
	shared_ptr<Variable> eye_state = reg->getVariable(parameters.find("eye_state")->second);	
	shared_ptr<Variable> eyeh_calibrated = reg->getVariable(parameters.find("eyeh_calibrated")->second);	
	shared_ptr<Variable> eyev_calibrated = reg->getVariable(parameters.find("eyev_calibrated")->second);
	unsigned int width_samples = 0;
	try {
		width_samples = boost::lexical_cast< unsigned int >(parameters.find("width_samples")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "width_samples", parameters.find("width_samples")->second);
	}
	
	checkAttribute(saccade_exit_speed, parameters.find("reference_id")->second, "saccade_exit_speed", parameters.find("saccade_exit_speed")->second);
	checkAttribute(saccade_entry_speed, parameters.find("reference_id")->second, "saccade_entry_speed", parameters.find("saccade_entry_speed")->second);
	checkAttribute(eye_state, parameters.find("reference_id")->second, "eye_state", parameters.find("eye_state")->second);
	checkAttribute(eyeh_calibrated, parameters.find("reference_id")->second, "eyeh_calibrated", parameters.find("eyeh_calibrated")->second);
	checkAttribute(eyev_calibrated, parameters.find("reference_id")->second, "eyev_calibrated", parameters.find("eyev_calibrated")->second);
	
	shared_ptr <mw::Component> newEyeStatusMonitorVer1 = shared_ptr<mw::Component>(new EyeStatusMonitorVer1(eyeh_calibrated,
																									   eyev_calibrated,
																									   eye_state,
																									   width_samples,
																									   saccade_entry_speed,
																									   saccade_exit_speed));
	return newEyeStatusMonitorVer1;
}

shared_ptr<mw::Component> EyeStatusMonitorVer2Factory::createObject(std::map<std::string, std::string> parameters,
													mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, 
					   
					   "saccade_exit_speed", 
					   "saccade_entry_speed", 
					   "width_samples", 
					   "eye_state", 
					   "eyeh_calibrated", 
					   "eyev_calibrated",
					   "saccade_entry_direction_change",
					   "saccade_min_amplitude",
					   "saccade_min_duration");
	
	shared_ptr<Variable> saccade_exit_speed = reg->getVariable(parameters.find("saccade_exit_speed")->second);	
	shared_ptr<Variable> saccade_entry_speed = reg->getVariable(parameters.find("saccade_entry_speed")->second);	
	shared_ptr<Variable> eye_state = reg->getVariable(parameters.find("eye_state")->second);	
	shared_ptr<Variable> eyeh_calibrated = reg->getVariable(parameters.find("eyeh_calibrated")->second);	
	shared_ptr<Variable> eyev_calibrated = reg->getVariable(parameters.find("eyev_calibrated")->second);
	shared_ptr<Variable> saccade_entry_direction_change = reg->getVariable(parameters.find("saccade_entry_direction_change")->second);
	shared_ptr<Variable> saccade_min_amplitude = reg->getVariable(parameters.find("saccade_min_amplitude")->second);
	shared_ptr<Variable> saccade_min_duration = reg->getVariable(parameters.find("saccade_min_duration")->second);
	unsigned int width_samples = 0;
	try {
		width_samples = boost::lexical_cast< unsigned int >(parameters.find("width_samples")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "width_samples", parameters.find("width_samples")->second);
	}

	checkAttribute(saccade_exit_speed, parameters.find("reference_id")->second, "saccade_exit_speed", parameters.find("saccade_exit_speed")->second);
	checkAttribute(saccade_entry_speed, parameters.find("reference_id")->second, "saccade_entry_speed", parameters.find("saccade_entry_speed")->second);
	checkAttribute(eye_state, parameters.find("reference_id")->second, "eye_state", parameters.find("eye_state")->second);
	checkAttribute(eyeh_calibrated, parameters.find("reference_id")->second, "eyeh_calibrated", parameters.find("eyeh_calibrated")->second);
	checkAttribute(eyev_calibrated, parameters.find("reference_id")->second, "eyev_calibrated", parameters.find("eyev_calibrated")->second);
	checkAttribute(saccade_entry_direction_change, parameters.find("reference_id")->second, "saccade_entry_direction_change", parameters.find("saccade_entry_direction_change")->second);
	checkAttribute(saccade_min_amplitude, parameters.find("reference_id")->second, "saccade_min_amplitude", parameters.find("saccade_min_amplitude")->second);
	checkAttribute(saccade_min_duration, parameters.find("reference_id")->second, "saccade_min_duration", parameters.find("saccade_min_duration")->second);
	
	shared_ptr <mw::Component> newEyeStatusMonitorVer2 = shared_ptr<mw::Component>(new EyeStatusMonitorVer2(eyeh_calibrated,
																									   eyev_calibrated,
																									   eye_state,
																									   width_samples,
																									   saccade_entry_speed,
																									   saccade_exit_speed,
																									   saccade_entry_direction_change,
																									   saccade_min_amplitude,
																									   saccade_min_duration));
	return newEyeStatusMonitorVer2;
}


// ======================================================================

EyeStatusComputer::EyeStatusComputer(int _filterWidthSamples,
                shared_ptr<Variable> _saccadeEntrySpeedDegPerSec, shared_ptr<Variable> _saccadeExitSpeedDegPerSec) {
        
        // parameters
        sacStartSpeedDegPerSec= _saccadeEntrySpeedDegPerSec;
        sacEndSpeedDegPerSec= _saccadeExitSpeedDegPerSec;
                
        Null_doublePoint.h = 0;
        Null_doublePoint.v = 0;
        
        // create all the transformers we need
        velocityComputerH = new VelocityComputer1D();
        velocityComputerV = new VelocityComputer1D();
        velocityFilterH = new BoxcarFilter1D(_filterWidthSamples);
        velocityFilterV = new BoxcarFilter1D(_filterWidthSamples);
        
        reset();
        
}

EyeStatusComputer::~EyeStatusComputer() {
            
        delete velocityComputerH;
        delete velocityComputerV;
        delete velocityFilterH;
        delete velocityFilterV;
            
}

// this is be done before IO is restarted to properly deal with boundary conditions
void EyeStatusComputer::reset() {
   
        velocityComputerH->reset();
        velocityComputerV->reset();
        velocityFilterH->reset();
        velocityFilterV->reset();
        
        // control of output
        eyeInformationComputedSuccessfully = false; 
        
        // call its super class reset
        Transform::reset();    
        
}


// take new eye data and perform storage and calculations
void EyeStatusComputer::input(float _eyeHdeg, float _eyeVdeg, MonkeyWorksTime _eyeTimeUS) {

    // not that only one time is passed in, and it is carried with the H channel
    //  (it is assumed that the H and V values passed in have the same time)

    eyeLocDeg.h = _eyeHdeg;
    eyeLocDeg.v = _eyeVdeg;

   
    eyeSpeedDegPerSec.valid = false;
    MonkeyWorksTime timeUS, timeUS_2, dummyTime;
        	

	// compute VELOCITY ------------------------------------------
    DOUBLE_POINT eyeVelocityTemp;
    velocityComputerH->input(_eyeHdeg, _eyeTimeUS);
    bool valid1 = velocityComputerH->output(&eyeVelocityTemp.h, &timeUS);
    
    velocityComputerV->input(_eyeVdeg, _eyeTimeUS);
    bool valid2 = velocityComputerV->output(&eyeVelocityTemp.v, &dummyTime);

    //mprintf("Eye status computer velh and velv = %f %f",eyeVelocityTemp.h, eyeVelocityTemp.v);

    // filter velocity with boxcar ------------------------------------------
    if (valid1 && valid2) {
    
        velocityFilterH->input(eyeVelocityTemp.h, timeUS);
        bool valid3 = velocityFilterH->output(&eyeVelocityDegPerSec.h, &timeUS_2);
        
        velocityFilterV->input(eyeVelocityTemp.v);
        bool valid4 = velocityFilterV->output(&eyeVelocityDegPerSec.v);
     
        //mprintf("Eye status computer velh and velv (filtered) = %f %f",eyeVelocityDegPerSec.h, eyeVelocityDegPerSec.v);
    
        // note: the eye velocity that is used to detect saccades has been filtered twice:
        // 1 = filtering of "raw" calibrated eye samples (these are used to estimate eye Velocity)
        // 2 = filtering of eyeVelocity
            
        // determine current eye speed
        if (valid3 && valid4) {
        
            // absolute mag (i.e. mag relativew to 0)
            eyeSpeedDegPerSec.value = sqrt( (pow(eyeVelocityDegPerSec.h,2)) + (pow(eyeVelocityDegPerSec.v,2))  );
            eyeSpeedDegPerSec.timeUS = timeUS_2;
            eyeSpeedDegPerSec.valid = true;
            // best estimate of when the eye actually reached this speed
            //  (asume you have allowed the filters to lag compensate)
            
            // mprintf("Eye status computer eyeVelocityDegPerSec = %f",eyeSpeedDegPerSec.value);
            
            // here one can use the basic model or something fancier
            // (e.g. something with saccade information)
            eyeInformationComputedSuccessfully = computeEyeInformation();
            //mprintf("Eye status computer: velocity, eyeInformationComputedSuccessfully = %f %d",eyeSpeedDegPerSec.value,eyeInformationComputedSuccessfully);
        }
    }
}
	

bool EyeStatusComputer::output(EyeStatusEnum *eyeState, MonkeyWorksTime *eyeStatusTimeUS) {

    if (eyeInformationComputedSuccessfully) {
        *eyeState = computedEyeStatus;
        *eyeStatusTimeUS = computedEyeStatusTimeUS;
        return true;
    } else {
        *eyeState = M_EYE_STATUS_UNKNOWN;
		shared_ptr <Clock> clock = Clock::instance();
        *eyeStatusTimeUS = clock->getCurrentTimeUS();
        return false;
    }
}

// this is the basic version    
bool EyeStatusComputer::computeEyeInformation() {
    
    // we may not yet have enough data to make a valid report
    if (!eyeSpeedDegPerSec.valid) {
        computedEyeStatus = M_EYE_STATUS_UNKNOWN;
		shared_ptr <Clock> clock = Clock::instance();
        computedEyeStatusTimeUS = clock->getCurrentTimeUS();
        return false;
    }
    
    // hysteresis built in here
    if ( eyeSpeedDegPerSec.value <= (double)(*sacEndSpeedDegPerSec)) {			// 5,3  units per msec is about right ! = approx 14,9 deg / sec
        computedEyeStatus = M_EYE_STATUS_FIXATING;
    }	
    else if  (eyeSpeedDegPerSec.value > (double)(*sacStartSpeedDegPerSec)){
        computedEyeStatus = M_EYE_STATUS_SACCADING;	
    }    
    computedEyeStatusTimeUS = eyeSpeedDegPerSec.timeUS;     
	return true;
    
} 

double	EyeStatusComputer::magnitude(DOUBLE_POINT vector1, DOUBLE_POINT vector2)
{
	double mag;
	mag = sqrt(pow((vector1.h-vector2.h),2) + pow((vector1.v-vector2.v),2) );
	return(mag);
}


double	EyeStatusComputer::phase(DOUBLE_POINT vector1, DOUBLE_POINT vector2)
{
	double phase;
	phase = atan2((vector1.v-vector2.v) , (vector1.h-vector2.h));
	// printf("arctan (%f / %f) = %f\n", y, x, atan2(y, x));
	return(phase);
}	


EyeStatusComputerWithSaccades::EyeStatusComputerWithSaccades(int _filterWidthSamples,
                shared_ptr<Variable> _saccadeEntrySpeedDegPerSec, shared_ptr<Variable> _saccadeExitSpeedDegPerSec,
                shared_ptr<Variable> _minimumStartDirectionChangeSeg, shared_ptr<Variable> _minimumAmplitudeDeg, shared_ptr<Variable> _minimumDurationMS) 
                : EyeStatusComputer(_filterWidthSamples, _saccadeEntrySpeedDegPerSec,_saccadeExitSpeedDegPerSec) {

    // extra params needed for the computer with saccades:
    
    // this is the velocity direction change needed to call the start of the saccade (find the look back point)
    sacStartDirectionChangeDeg = _minimumStartDirectionChangeSeg;
    
    // these are are the minimal values needed to "accept" a saccade as having occurred.
    minSacAmplitudeDeg = _minimumAmplitudeDeg;
    minSacDurationMS = _minimumDurationMS;
    
    computeSacTrueEnd = false;

}

EyeStatusComputerWithSaccades::~EyeStatusComputerWithSaccades() {};


void EyeStatusComputerWithSaccades::reset() {

        // saccade detection stuff
        saccadeInformationIsAvailable = false;
		shared_ptr <Clock> clock = Clock::instance();
        timeofLastReset = clock->getCurrentTimeUS();
        engageSaccadeDetection = false;
        checkForSaccadeStart = false;
        
        EyeStatusComputer::reset();  

}

bool EyeStatusComputerWithSaccades::outputSaccade(SaccadeData *saccadeData) {
    if (saccadeInformationIsAvailable) {
        *saccadeData = computedSaccadeData;
        return true;
    }
    else {
        return false;
    }
}



bool EyeStatusComputerWithSaccades::computeEyeInformation() {

    // wait until the speed has been successfully computed
    if (!eyeSpeedDegPerSec.valid) {
        computedEyeStatus = M_EYE_STATUS_UNKNOWN;
		shared_ptr <Clock> clock = Clock::instance();
        computedEyeStatusTimeUS = clock->getCurrentTimeUS();
        return false;
    }

  
    // eye status update stuff (similar to the base eye computer ) ------
  
    // we use the saccade detector to lock us in an assumed fixed state
    //   for some period of time after a saccade has just ended. 
    if (computeSacTrueEnd) {
        computedEyeStatus = M_EYE_STATUS_FIXATING;
    }
    else {
        // hysteresis built in here
        if ( eyeSpeedDegPerSec.value <= (double)(*sacEndSpeedDegPerSec)) {			// 5,3  units per msec is about right ! = approx 14,9 deg / sec
            computedEyeStatus = M_EYE_STATUS_FIXATING;
        }	
        else if  (eyeSpeedDegPerSec.value > (double)(*sacStartSpeedDegPerSec)){
            computedEyeStatus = M_EYE_STATUS_SACCADING;	
        }
        // else eye state is same as last state (value not changed)
    }
    
    computedEyeStatusTimeUS = eyeSpeedDegPerSec.timeUS; 
    
    // -----------------------------------------------------------------    
            
    // now we do the fancy saccade detection stuff
           
    #define MIN_ENGAGE_TIME_MS     10       // wait at least this many ms after last reset to start looking for saccades
    #define TRUE_END_DELAY_MS		5		// wait xx msec after detected end to compute true end
    #define TRUE_END_DURATION_MS	5		// coumpute true end over next xx msec 
    #define PEAK_BASED_PREDICTION_MULTIPLIER    0.65	// multiply changes to peak by this value to get predicted (total) changes 
    
    // this is the estiamte of the actual time of the eyeSoeedValue that has been passed in        
    long currentDataTimeMS = (eyeSpeedDegPerSec.timeUS/1000);
    
     // check if it is now OK to turn on saccade detection
     // eye is fixated after last reset call (e.g. start of IO), begin checking
    if (!engageSaccadeDetection) {
		shared_ptr <Clock> clock = Clock::instance();
        long timeSinceLastResetMS = ((long)(clock->getCurrentTimeUS()-timeofLastReset))/1000;
        if ((timeSinceLastResetMS > MIN_ENGAGE_TIME_MS) && (computedEyeStatus == M_EYE_STATUS_FIXATING)) {		
            engageSaccadeDetection = true;      // will only get in here once
            checkForSaccadeStart = true;        // this is the flag to get going
        }
    }
 
       
    // old (modified) saccade detection code follows here
    
    // compute the current change in movement direction 
    eyeDirectionAngle = (float)(phase(eyeVelocityDegPerSec,eyeVelocityDegPerSecLast));		// direction from last sample

    // TODO buffer things:  location, time, direction (to allow look back for change)
    //eyeDirectionBuffer = xx(eyeDirectionAngle);                                       // will buffer last (lookBackSamples) direction samples
    //bufferTimeMS == xxx(computedEyeStatusTimeUS) currentDataTimeMS
    //bufferLoc ==xxx(eyeLocDeg)
                        
                
    if (checkForSaccadeStart) {		
    
        if ((computedEyeStatus == M_EYE_STATUS_SACCADING) && (computedEyeStatusLast == M_EYE_STATUS_FIXATING)) {		// saccade start
            
            mprintf("Advanced Eye Monitor:  Saccade start detected.");
            
            sacData.sacStartTimeMS = currentDataTimeMS;
            sacData.sacStartLocDeg = eyeLocDeg;
            
            // TODO -- this was ad hoc originally -- need to work on a better way to do this.
            
            /*
            // determine "actual" start time and location by looking back for direction change
            // if no direction change detected, will look back to lookBackSamples = time of LOOK_BACK_MS (hard msec limit)
            for (int i=1;i<=lookBackSamples;i++) {		// sample 1 == Last sample, sample 0 = current sample 
                lookBackSamplesToActual = i;
                if (fabs(bufferDirection[i] - bufferDirection[0]) > sacStartDirectionChangeDeg) {
                    i=lookBackSamples+1;	// break look
                }
            }
            long timeAtWhichDirectionChangeDetectedMS = bufferTimeMS[lookBackSamplesToActual];      
            
            // actual saccade start time recorded as the time that a direction change actually occured (look back in time)
            sacData.sacStartTimeMS = timeAtWhichDirectionChangeDetectedMS;
            sacData.sacStartLocDeg.h = bufferLoc[lookBackSamplesToActual].h;	// location at time of change direction
            sacData.sacStartLocDeg.v = bufferLoc[lookBackSamplesToActual].v;	// location at time of change direction
            */
        
            // compute piecewise amplitude from "actual" start location (time)  to (last eye loc sample - 1)
            sacAmplitudePiecewise = 0;
            /* TODO -- put this back in
            for (i=lookBackSamplesToActual;i>=2;i--) {		// sample 1 == EyeLocLast, sample 0 = eyeLoc (current0 
                sacAmplitudePiecewise = sacAmplitudePiecewise + magnitude(bufferLoc[i],bufferLoc[i-1]);
            }
            */
            sacPeakSpeed = 0;
            sacBeyondVelocityPeak = false;
            getPeak = true;
            down = 0;
            inSaccade = true;
            checkForSaccadeStart = false;
            checkForSaccadeEnd = true;
            
        }
    }
    
    if (inSaccade) {		// still in saccade -- start looking for the peak in the speed
        sacAmplitudePiecewise = sacAmplitudePiecewise + magnitude(eyeLocDegLast,eyeLocDeg);
        if (eyeSpeedDegPerSec.value>=sacPeakSpeed) {
            sacPeakSpeed = eyeSpeedDegPerSec.value;
            peakLoc = eyeLocDeg;
        }
        if (eyeSpeedDegPerSec.value<eyeSpeedDegPerSecLast.value) 
            down++;
        else
            down = 0;
        if ( (down>=3) && getPeak) {		// need 3 consecutive decreases in speed to decide peak has passed
            sacBeyondVelocityPeak = true;
            sacData.peakDetected = true;
            xx.h = sacData.sacStartLocDeg.h;
            xx.v = sacData.sacStartLocDeg.v;
            lengthToPeak = magnitude(xx,eyeLocDeg);
            sacData.peakBasedPredictedLengthDeg =  lengthToPeak +  (lengthToPeak * PEAK_BASED_PREDICTION_MULTIPLIER); 
            // crude predicted landing location
            sacData.peakBasedPredictedEndLocDeg.h = peakLoc.h + 
                                ((peakLoc.h - sacData.sacStartLocDeg.h) * PEAK_BASED_PREDICTION_MULTIPLIER); 
            sacData.peakBasedPredictedEndLocDeg.v = peakLoc.v + 
                                ((peakLoc.v - sacData.sacStartLocDeg.v)* PEAK_BASED_PREDICTION_MULTIPLIER);												
            getPeak = false;
        }
    }
    
    if (checkForSaccadeEnd) {	
    
        if ( (computedEyeStatus == M_EYE_STATUS_FIXATING) && (computedEyeStatusLast == M_EYE_STATUS_SACCADING)) {		// saccade end
        
            //goCheckResponseWindows = true;			// will check if next sample has triggered any response windows
            sacData.sacEndTimeMS =  currentDataTimeMS;	
            sacData.durationMS = sacData.sacEndTimeMS - sacData.sacStartTimeMS;
            sacData.sacEndLocDeg.h = eyeLocDeg.h;
            sacData.sacEndLocDeg.v = eyeLocDeg.v;
            
            if (!sacBeyondVelocityPeak) {			// peak not detected
                //rprintf("WARNING: missed peak");
                sacData.peakBasedPredictedEndLocDeg.h = 0;		// -- no prediction available: task system will effectively ignore this saccade
                sacData.peakBasedPredictedEndLocDeg.v = 0;
                sacData.peakBasedPredictedLengthDeg = 0;
                sacBeyondVelocityPeak = true;		// allows task system to continue !!! -- DO NOT REMOVE !!!
                sacData.peakDetected = false;
            }
            sacData.peakSpeedDegPerSec = sacPeakSpeed*1000.;		// now in Deg per Sec
            
            // prepare to compute "final" position
            sacData.sacTrueEndLocDeg.h = sacData.sacTrueEndLocDeg.v = 0;
            checkForSaccadeEnd = false;
            inSaccade = false;
            computeSacTrueEnd = true;
            nSamplesAveraged = 0;
                            
        }	
    }
        
    if (computeSacTrueEnd) {		// this will carry over several samples (compute average loc over DURATION msec)
        sacAmplitudePiecewise = sacAmplitudePiecewise + magnitude(eyeLocDegLast,eyeLocDeg); // continue computing amp
        timeSinceSacDetectedEnd = currentDataTimeMS - sacData.sacEndTimeMS;
        if ((timeSinceSacDetectedEnd >= TRUE_END_DELAY_MS) &&
             (timeSinceSacDetectedEnd <= (TRUE_END_DELAY_MS + TRUE_END_DURATION_MS) )) {   // assume DELAY msec after detected end is "true" end
            nSamplesAveraged++;
            sacData.sacTrueEndLocDeg.h = sacData.sacTrueEndLocDeg.h + eyeLocDeg.h;
            sacData.sacTrueEndLocDeg.v = sacData.sacTrueEndLocDeg.v + eyeLocDeg.v;
        }
        if (timeSinceSacDetectedEnd > (TRUE_END_DELAY_MS + TRUE_END_DURATION_MS) ) {		
                computeSacTrueEnd = false;
                checkForSaccadeStart = true;
                sacData.sacTrueEndLocDeg.h = sacData.sacTrueEndLocDeg.h/(double)nSamplesAveraged;
                sacData.sacTrueEndLocDeg.v = sacData.sacTrueEndLocDeg.v/(double)nSamplesAveraged;
                sacData.distanceDegPiecewise = sacAmplitudePiecewise;
                sacData.distanceDeg = magnitude(sacData.sacTrueEndLocDeg, sacData.sacStartLocDeg);
                
                //overshootDeg.h = (sacData.sacTrueEndLoc.h - sacData.sacDetectedEndLoc.h);
                //overshootDeg.v = (sacData.sacTrueEndLoc.v - sacData.sacDetectedEndLoc.v);
                                
                if ((sacData.distanceDeg >=  (double)(*minSacAmplitudeDeg)) && 
                    (sacData.durationMS > (double)(*minSacDurationMS) ) ) {	
                    
                    /*
                    // ISI computation (ISI = inter-saccadic interval)
                    if (sacDetectedStartTimeLast != 0) {
                        sacData.timeSinceLastMS = sacData.sacDetectedStartTime - sacDetectedStartTimeLast;
                        sacData.latencyMS = sacData.sacDetectedStartTime - sacDetectedEndTimeLast;
                    }
                    else {
                        sacData.timeSinceLastMS = 0;		// unknown -- current saccade is the first since last reset.
                    }
                    sacDetectedStartTimeLast = sacData.sacDetectedStartTime;
                    sacDetectedEndTimeLast = sacData.sacDetectedEndTime;
                    durationLast =  sacData.durationMS;
                    */
                    
                    // TODO -- a valid saccade was detected, and all data computed
                    //  post the saccade data now
                    
                }	
                
        }				
    }
	
		
	// prepare for next call
	computedEyeStatusLast = computedEyeStatus;
	eyeLocDegLast = eyeLocDeg;			
	eyeVelocityDegPerSecLast = eyeVelocityDegPerSec;
	eyeSpeedDegPerSecLast = eyeSpeedDegPerSec;
                                  
	return true;        

}


      

	

/*
// TODO -- should replace these with circular buffers
void EyeStatusComputer::eyeLocBuffer(DOUBLE_POINT eyeLoc)
{
	register short i;
	for (i=lookBackSamples;i>=1;i--) {
		bufferLoc[i] = bufferLoc[i-1];
	}
	bufferLoc[0] = eyeLoc;		// 0 is current eye sample
}		
// TODO -- should replace these with circular buffers
void EyeStatusComputer::eyeDirectionBuffer(float eyeDirection)
{
	register short i;
	for (i=lookBackSamples;i>=1;i--) {
		bufferDirection[i] = bufferDirection[i-1];
	}
	bufferDirection[0] = eyeDirection;
}		
*/


/*
// TODO -- put saccade compute back in ------------------------------------

// for saccade calculations, we need:
// eyeVelocityFiltered, eyeSpeed
// eyeState

// we need:
buffered locations and times
buffered directions

params:
allEyeParams. xxx



 
    
      
        }


*/

// Original code follows:
/*

// updates current eye State based on current and previous eye samples
// all computations done in degrees and msec !!!

static void eyeCompute(float JJD_sampleTimeMS, float sampleIntervalMS, DOUBLE_POINT eyeLocDeg, Boolean markEvents)
{
	
	register float  deltaMS;
 
	static Boolean 	prediction = false;   // no prediction generated
	DOUBLE_POINT 	overshoot;
	short			lookBackSamplesToActual;
	double  		eyeDirection;
	short i;
	static short boxWidthSamples;
	static Boolean first = true;
	static Boolean getPeak;
	static short	down;
	float lengthToPeak;
	static DOUBLE_POINT peakLoc;
	DOUBLE_POINT  DeyeLocFiltered;
	
	double 				eyeSpeedUnfiltered;
	DOUBLE_POINT 		eyeVelocityUnfiltered;
	static DOUBLE_POINT eyeLocLastUnfiltered;
	DOUBLE_POINT		xx;
	
	
	// checking for door open
	static short nConsecutiveInvalid = 0;
	static short nConsecutiveValid = 0;
	
	static float sampleIntervalMSlast;
	Boolean newSampleInterval = false;
	
	if (sampleIntervalMS != sampleIntervalMSlast) {
		rprintf("new sample interval detected");
		newSampleInterval = true;
	}
	sampleIntervalMSlast = 	sampleIntervalMS;
		
	deltaMS = (JJD_sampleTimeMS-timeLastMS);		/// floats (ms)
	if (deltaMS <=0) return;						// JJD_sampleTime not running yet
	
	if ((first) || (boxFilterChanged) || (newSampleInterval)) {		// boxFilterChanged = true when user alters filter size in dialog
							 //TODO !!! -- this is not correct for eye tracker -- deltaMS too variable!
			
		// need to set up the boxcar samples even if digital filter is being used, because velocity is still filtered with a boxcar
		boxWidthSamples = max(1,(short)(allEyeParams.eyeFilterBoxcarWidthMS/sampleIntervalMS));
		if (boxWidthSamples >= MAX_LOOK_BACK_SAMPLES) {
			boxWidthSamples = MAX_LOOK_BACK_SAMPLES-1;
			rprintf("BOXCAR FILTER width too wide for array: clipped at %d samples = %.1f msec", boxWidthSamples,boxWidthSamples*sampleIntervalMS); 
		}
		else
			rprintf("BOXCAR FILTER width set at %d samples = %.1f msec", boxWidthSamples,boxWidthSamples*sampleIntervalMS);
		lookBackSamples = max( ((short)((float)LOOK_BACK_MS/deltaMS)),  MAX_LOOK_BACK_SAMPLES-1);  
			// LOOK_BACK_MS is time when may want to check for earlier values (e.g. direction change, etc.)
							
		boxFilterChanged = false;
		first = false;
		newSampleInterval = false;
	}

	// apply eye sample filter and update the eye filtered value
	if (useDigitalFilterOnEyeSamples) {
		// each digital filter will maintain its own buffer of previous input and output values
		DeyeLocFiltered.h = (double)(eyeLocDeg.h); 	
		DeyeLocFiltered.v = (double)(eyeLocDeg.v);
		DeyeLocFiltered = digital_filter(B_LP125, A_LP125, LP125_ORDER, DeyeLocFiltered, eyeLocStoreInputLP125,eyeLocStoreOutputLP125 );
		if (useNotch60)  DeyeLocFiltered = digital_filter(B_N60, A_N60, N60_ORDER, DeyeLocFiltered, eyeLocStoreInputN60,eyeLocStoreOutputN60);
		if (useNotch75)  DeyeLocFiltered = digital_filter(B_N75, A_N75, N75_ORDER, DeyeLocFiltered,  eyeLocStoreInputN75,eyeLocStoreOutputN75);
		if (useNotch180) DeyeLocFiltered = digital_filter(B_N180, A_N180, N180_ORDER, DeyeLocFiltered, eyeLocStoreInputN180,eyeLocStoreOutputN180);
		eyeLocFiltered.h = (float)(DeyeLocFiltered.h); 	
		eyeLocFiltered.v = (float)(DeyeLocFiltered.v);
	}	
	else {// do boxcar
		eyeLocFiltered = boxcar_filter(boxWidthSamples, eyeLocDeg, eyeLocStore);  	// eyeLocStore is a buffer of RAW eye samples,updated by this routine
							// (this imposes a lag = allEyeParams.eyeFilterBoxcarWidthMS/2)
	}
	
	eyeLocBuffer(eyeLocFiltered);			// will buffer last (lookBackSamples) samples
	
	
	// compute VELOCITY and eye state------------------------------------------
		
		if (deltaMS>0) {
			eyeVelocity.h = (eyeLocFiltered.h-eyeLocLast.h)/deltaMS;  // positive = rightward
			eyeVelocity.v = (eyeLocFiltered.v-eyeLocLast.v)/deltaMS;  // positive = upward
			
			// unfiltered velocity (for testing of filter)
			eyeVelocityUnfiltered.h = (eyeLocDeg.h-eyeLocLastUnfiltered.h)/deltaMS;  // positive = rightward
			eyeVelocityUnfiltered.v = (eyeLocDeg.v-eyeLocLastUnfiltered.v)/deltaMS;  // positive = upward
		}
		else {
			eyeVelocity.h = eyeVelocity.v = eyeVelocityUnfiltered.h = eyeVelocityUnfiltered.v = 0;
		}
		
		//update filtered velocity and return current filtered value (this imposes a lag = allEyeParams.eyeFilterBoxcarWidthMS/2)
		eyeVelocityFiltered = boxcar_filter(boxWidthSamples,eyeVelocity, eyeVelocityStore);
		// note: the eye velocity that is used to detect saccades has been filtered twice:
			// 1 = filtering of "raw" eye samples (after conversion to degs) --> these are used to estimate eye Velocity
			// 2 = filtering of eyeVelocity
		
		// determine current eye velocity parameters  
		eyeSpeed = magnitude(eyeVelocityFiltered,fpointNull);			// absolute mag (i.e. mag relativew to 0)
		eyeSpeedUnfiltered = magnitude(eyeVelocityUnfiltered, fpointNull);		// completely unfiltered eye velocity
		
		eyeDirection = (float)(phase(eyeVelocityFiltered,eyeVelocityLast));		// direction from last sample
		eyeDirectionBuffer(eyeDirection);			// will buffer last (lookBackSamples) direction samples

		
		// determine current eye state  		
		if (resetMS>0) {			// note -- a reset of sorts is built in to the saccade compute end 
			eyeState = FIXATED; 
			resetMS = resetMS - deltaMS;
		}
		else		// reset from last saccade end has completed and not at trial start -- return to monitoring eyeState changes
		{
			// hysteresis built in here
			if ( eyeSpeed <= sacEndSpeedDegPerMS) {			// 5,3  units per msec is about right ! = approx 14,9 deg / sec
				eyeState = FIXATED;
			}	
			else if  (eyeSpeed > sacStartSpeedDegPerMS){
				eyeState = SACCADING;	
			}
			// else eye state is same as last state.
		}
		
		if (trialStart) {
			if  ((JJD_sampleTimeMS > LOOK_BACK_MS ) && (eyeState == FIXATED)) {		// eye is fixated after trial start, begin checking
				trialStart = false;
				checkForSaccadeStart = true;
			}
		}
		
		
		if (checkForSaccadeStart) {		
		
			if ((eyeState == SACCADING) && (eyeStateLast == FIXATED)) {		// saccade start
				
				// determine "actual" start time and location by looking back for direction change
				// if no direction change detected, will look back to lookBackSamples = time of LOOK_BACK_MS (hard msec limit)
				for (i=1;i<=lookBackSamples;i++) {		// sample 1 == Last sample, sample 0 = current sample 
					lookBackSamplesToActual = i;	
					if (fabs(bufferDirection[i] - bufferDirection[0]) > sacStartDirectionChangeDeg) {
						i=lookBackSamples+1;	// break look
					}
				}
				
				sacData.sacDetectedStartTime = (short)(JJD_sampleTimeMS - ((float)lookBackSamplesToActual)*deltaMS);
				sacData.sacDetectedStartLoc.h = bufferLoc[lookBackSamplesToActual].h;	// location (10 msec) before current time
				sacData.sacDetectedStartLoc.v = bufferLoc[lookBackSamplesToActual].v;	// location (10 msec) before current time
				//sacData.statusSaccadeStart = bufferStatus[lookBackSamplesToActual];	// save the status (when sac started)
				
				// compute piecewise amplitude from "actual" start location (time)  to (last eye loc sample - 1)
				sacAmplitudePiecewise = 0;
				for (i=lookBackSamplesToActual;i>=2;i--) {		// sample 1 == EyeLocLast, sample 0 = eyeLoc (current0 
					sacAmplitudePiecewise = sacAmplitudePiecewise + magnitude(bufferLoc[i],bufferLoc[i-1]);
				}
				sacPeakSpeed = 0;
				sacBeyondVelocityPeak = false;
				//samplesPastPeak = 0;
				//timer = 0;
				//timerRunning = false;
				getPeak = true;
				down = 0;
				inSaccade = true;
				checkForSaccadeStart = false;
				checkForSaccadeEnd = true;
					
				if (markEvents) {
						putEvent5("SaccadeStart", NULL, NULL);
				}	
				//ITC_DigitalBitsOn(0x04);
				
			}
		}
		
		if (inSaccade) {		// still in saccade
			sacAmplitudePiecewise = sacAmplitudePiecewise + magnitude(eyeLocLast,eyeLocFiltered);
			if (eyeSpeed>=sacPeakSpeed) {
				sacPeakSpeed = eyeSpeed;
				peakLoc = eyeLocFiltered;
			}
			if (eyeSpeed<eyeSpeedLast) 
				down++;
			else
				down = 0;
			if ( (down>=3) && getPeak) {		// need 3 consecutive decreases in speed to decide peak has passed
				sacBeyondVelocityPeak = true;
				sacData.peakDetected = true;
				xx.h = sacData.sacDetectedStartLoc.h;
				xx.v = sacData.sacDetectedStartLoc.v;
				lengthToPeak = magnitude(xx,eyeLocFiltered);
				sacData.peakBasedPredictedLengthDeg =  lengthToPeak +  (lengthToPeak * PEAK_BASED_PREDICTION_MULTIPLIER); 
				// crude predicted landing location
				sacData.peakBasedPredictedEyeLoc.h = peakLoc.h + 
															((peakLoc.h - sacData.sacDetectedStartLoc.h) * PEAK_BASED_PREDICTION_MULTIPLIER); 
				sacData.peakBasedPredictedEyeLoc.v = peakLoc.v + 
															((peakLoc.v - sacData.sacDetectedStartLoc.v)* PEAK_BASED_PREDICTION_MULTIPLIER);												
				getPeak = false;
			}
		}
		
		if (checkForSaccadeEnd) {	
		
			if ( (eyeState == FIXATED) && (eyeStateLast == SACCADING)) {		// saccade end
			
				goCheckResponseWindows = true;			// will check if next sample has triggered any response windows
				//printf(" sac ENDING: sacNum = %d  end time = %ld \n",sacNum,JJD_sampleTime);
				sacData.sacDetectedEndTime =  (long)JJD_sampleTimeMS;	//TODO -- these times should be same as pEvent times !!!
				sacData.durationMS = sacData.sacDetectedEndTime - sacData.sacDetectedStartTime;
				sacData.sacDetectedEndLoc.h = eyeLocFiltered.h;
				sacData.sacDetectedEndLoc.v = eyeLocFiltered.v;
				
				if (!sacBeyondVelocityPeak) {			// peak not detected
					//rprintf("WARNING: missed peak");
					sacData.peakBasedPredictedEyeLoc.h = 0;		// -- no prediction available: task system will effectively ignore this saccade
					sacData.peakBasedPredictedEyeLoc.v = 0;
					sacData.peakBasedPredictedLengthDeg = 0;
					sacBeyondVelocityPeak = true;		// allows task system to continue !!! -- DO NOT REMOVE !!!
					sacData.peakDetected = false;
				}
				sacData.peakSpeedDegPerSec = sacPeakSpeed*1000.;		// now in Deg per Sec
				
				// prepare to compute "final" position
				sacData.sacTrueEndLoc.h = sacData.sacTrueEndLoc.v = 0;
				checkForSaccadeEnd = false;
				inSaccade = false;
				computeSacTrueEnd = true;
				timeSinceSacDetectedEnd = 0;
				c = 0;
				resetMS = RESET_MS;			// assume another sac cannot occur in next RESET_MS msec;
				if (markEvents) {
						putEvent5("SaccadeEnd", NULL, NULL);
				}
				//ITC_DigitalBitsOff(0x04);
				
				
				
				
			}	
		}
			
		if (computeSacTrueEnd) {		// this will carry over several samples (compute average loc over DURATION msec)
			sacAmplitudePiecewise = sacAmplitudePiecewise + magnitude(eyeLocLast,eyeLocFiltered); // continue computing amp
			timeSinceSacDetectedEnd = timeSinceSacDetectedEnd + deltaMS;
			if ((timeSinceSacDetectedEnd >= TRUE_END_DELAY_MS) &&
				 (timeSinceSacDetectedEnd <= (TRUE_END_DELAY_MS + TRUE_END_DURATION_MS) )) {   // assume DELAY msec after detected end is "true" end
				c++;
				sacData.sacTrueEndLoc.h = sacData.sacTrueEndLoc.h + eyeLocFiltered.h;
				sacData.sacTrueEndLoc.v = sacData.sacTrueEndLoc.v + eyeLocFiltered.v;
			}
			if (timeSinceSacDetectedEnd > (TRUE_END_DELAY_MS + TRUE_END_DURATION_MS) ) {		
					computeSacTrueEnd = false;
					checkForSaccadeStart = true;
					sacData.sacTrueEndLoc.h = sacData.sacTrueEndLoc.h/(float)c;
					sacData.sacTrueEndLoc.v = sacData.sacTrueEndLoc.v/(float)c;
					sacData.distancePieceDeg = sacAmplitudePiecewise;
					sacData.distanceDeg = magnitudeF(sacData.sacTrueEndLoc, sacData.sacDetectedStartLoc);
					
					overshoot.h = (sacData.sacTrueEndLoc.h - sacData.sacDetectedEndLoc.h);
					overshoot.v = (sacData.sacTrueEndLoc.v - sacData.sacDetectedEndLoc.v);
					if (sacPeakVelocity.h < 0) overshoot.h = -1.*overshoot.h; 
					if (sacPeakVelocity.v < 0) overshoot.v = -1.*overshoot.v; 
					sacData.overshootDistDeg.h = overshoot.h;
					sacData.overshootDistDeg.v = overshoot.v;
						
					if ((sacData.distanceDeg <  allEyeParams.minSacAmplitudeDeg) || 
						(sacData.durationMS < allEyeParams.minSacDurationMS ) ){ // -- not counted as a saccade
						if (markEvents) {
								putEvent5("SaccadeInvalid", NULL, NULL);	// will end plotting
						}
					
					}
					else { 	// "valid" saccade	
						
						// ISI computation
						if (sacDetectedStartTimeLast != 0) {
							sacData.timeSinceLastMS = sacData.sacDetectedStartTime - sacDetectedStartTimeLast;
							sacData.latencyMS = sacData.sacDetectedStartTime - sacDetectedEndTimeLast;
						}
						else {
							sacData.timeSinceLastMS = 0;		// unknown -- current saccade is first in trial
						}
									
						if (markEvents) {
							putEvent5("SaccadeData", (char *)&sacData, NULL);
						}
						
						
						sacDetectedStartTimeLast = sacData.sacDetectedStartTime;
						sacDetectedEndTimeLast = sacData.sacDetectedEndTime;
						durationLast =  sacData.durationMS;
					}	
					
			}				
		}
	
		
	// prepare for next call
	eyeStateLast = eyeState;
	eyeLocLast = eyeLocFiltered;			// saves last filtered value
	eyeLocLastUnfiltered = eyeLocDeg;		// saves last raw value
	timeLastMS = JJD_sampleTimeMS;
	eyeVelocityLast = eyeVelocityFiltered;
	eyeSpeedLast = eyeSpeed;
	if (prediction) {	
		eyeAccelLast = eyeAccel;
	}
}

*/


// object to buffer and insure pairing of eye data
        
    PairedEyeData::PairedEyeData(int buffer_size) {
        // setup buffers to hold eye data -- we assume that these are collected at the
        //  same rate, and we can check this occasionally    
        
        eyeH_buffer = new FloatDataTimeStampedRingBuffer(buffer_size); 
        eyeV_buffer = new FloatDataTimeStampedRingBuffer(buffer_size);
        eyeH_buffer_reader = new FloatDataTimeStampedBufferReader(eyeH_buffer);       
        eyeV_buffer_reader = new FloatDataTimeStampedBufferReader(eyeV_buffer);         
    }
    
    PairedEyeData::~PairedEyeData() {
        delete eyeH_buffer;
        delete eyeV_buffer;
        delete eyeH_buffer_reader;
        delete eyeV_buffer_reader;
    }
    
    void PairedEyeData::reset() {
        eyeH_buffer->reset();
        eyeV_buffer->reset();
        eyeH_buffer_reader->seek(0);
        eyeV_buffer_reader->seek(0);
    }
    
    
    void PairedEyeData::putDataH(double eyeH, MonkeyWorksTime timeUS) {
                eyeH_buffer->putData(eyeH, timeUS);
    }
    void PairedEyeData::putDataV(double eyeV, MonkeyWorksTime timeUS) {
                eyeV_buffer->putData(eyeV, timeUS);
    }
 
    // need to be sophisticated about waiting for pairs of H and V before transforming to final output
    // ideally, all data should be paired (ITC does this)
    // here, we just insure that the pairs that are sent for processing have stamped times within M_WARN_EYE_PAIR_SEPARATION_US of each other.
 
    bool PairedEyeData::getAvailable(double *pEyeH, double *pEyeV, MonkeyWorksTime *pEyeTimeUS) {
    
    if ( (eyeH_buffer_reader->getNItemsUnserviced()==0) || 
                (eyeV_buffer_reader->getNItemsUnserviced()==0) ) {
       return false;        //  no data in one or more buffers
    }
    
    bool diffWarned = false;
    
    // keep trying to find paired data
    long diff;
    MonkeyWorksTime eyeVtimeUS, eyeHtimeUS;
    while ( (eyeH_buffer_reader->getNItemsUnserviced()>0) && 
                (eyeV_buffer_reader->getNItemsUnserviced()>0) ) {
                
        *pEyeV = (double)(eyeV_buffer_reader->getData());
        eyeVtimeUS = eyeV_buffer_reader->getTime();
                 
        *pEyeH = (double)(eyeH_buffer_reader->getData());
        eyeHtimeUS = eyeH_buffer_reader->getTime();
        
        diff = (long)(eyeHtimeUS-eyeVtimeUS);
        
        if (abs(diff) <= M_MAXIMAL_ALLOWED_EYE_PAIR_SEPARATION_US) {   // data alignment acceptable to use 
            *pEyeTimeUS = eyeHtimeUS;
            eyeH_buffer_reader->advance();
            eyeV_buffer_reader->advance();
            return true;  // data should be used by object
        }
    
        if (!diffWarned) {
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                "mPairedEyeData:  Eye H and V data are not paired in time within required limit of %d us. They are separated by %d us  (+ means old V data has no paired H). Correcting by discarding unpaired data.", 
                M_MAXIMAL_ALLOWED_EYE_PAIR_SEPARATION_US, diff);
            diffWarned = true;
        }
            
        // try to align data by discarding old data that has no possibility of being paired
        //  (note:  this assumes that the data arrive to the object in chronological order !)
        
        if (eyeHtimeUS>eyeVtimeUS) {  // eyeV is too old -- kill it
            //mprintf("Ignoring one element of eye V data");
            eyeV_buffer_reader->advance();
        }
        else {
            //mprintf("Ignoring one element of eye H data");
            eyeH_buffer_reader->advance();
        }
    }
     
    // if we exit the while loop, one or more buffers is empty               
    return false;
    
}


