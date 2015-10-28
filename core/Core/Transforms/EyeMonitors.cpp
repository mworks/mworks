/*
 *  EyeStatus.cpp
 *  MWorksCore
 *
 *  Created by dicarlo on 8/9/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "EyeMonitors.h"
#include "ComponentRegistry.h"
#include <boost/lexical_cast.hpp>


BEGIN_NAMESPACE_MW


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

void EyeStatusMonitor::newDataReceived(int inputIndex, const Datum& data, MWTime timeUS) {

	// DDC: be careful
    //lock();
    
	double eyeDataIn = (double)data;        
	double eyeDataOut;
	MWTime postFilterTimeUS;
    
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
    MWTime eyeTimeUS;
    double eyeH, eyeV;
    while (pairedEyeData->getAvailable(&eyeH, &eyeV, &eyeTimeUS)) {
        processAndPostEyeData(eyeH,eyeV,eyeTimeUS); 
    }    
    
    //unlock();
				
}


const std::string EyeStatusMonitorVer1::EYEH_CALIBRATED("eyeh_calibrated");
const std::string EyeStatusMonitorVer1::EYEV_CALIBRATED("eyev_calibrated");
const std::string EyeStatusMonitorVer1::EYE_STATE("eye_state");
const std::string EyeStatusMonitorVer1::EYE_VELOCITY_H("eye_velocity_h");
const std::string EyeStatusMonitorVer1::EYE_VELOCITY_V("eye_velocity_v");
const std::string EyeStatusMonitorVer1::WIDTH_SAMPLES("width_samples");
const std::string EyeStatusMonitorVer1::SACCADE_ENTRY_SPEED("saccade_entry_speed");
const std::string EyeStatusMonitorVer1::SACCADE_EXIT_SPEED("saccade_exit_speed");


void EyeStatusMonitorVer1::describeComponent(ComponentInfo &info) {
    EyeStatusMonitor::describeComponent(info);
    
    info.setSignature("filter/basic_eye_monitor");
    
    info.addParameter(EYEH_CALIBRATED);
    info.addParameter(EYEV_CALIBRATED);
    info.addParameter(EYE_STATE);
    info.addParameter(EYE_VELOCITY_H, false);
    info.addParameter(EYE_VELOCITY_V, false);
    info.addParameter(WIDTH_SAMPLES);
    info.addParameter(SACCADE_ENTRY_SPEED);
    info.addParameter(SACCADE_EXIT_SPEED);
}


EyeStatusMonitorVer1::EyeStatusMonitorVer1(const ParameterValueMap &parameters) :
    EyeStatusMonitor(VariablePtr(parameters[EYEH_CALIBRATED]),
                     VariablePtr(parameters[EYEV_CALIBRATED]),
                     VariablePtr(parameters[EYE_STATE]),
                     int(parameters[WIDTH_SAMPLES])),
    eyeVelocityHIndex(-1),
    eyeVelocityVIndex(-1)
{
    eyeStatusComputer = new EyeStatusComputer(int(parameters[WIDTH_SAMPLES]),
                                              VariablePtr(parameters[SACCADE_ENTRY_SPEED]),
                                              VariablePtr(parameters[SACCADE_EXIT_SPEED]));
    
    if (!(parameters[EYE_VELOCITY_H].empty())) {
        eyeVelocityHIndex = registerOutput(VariablePtr(parameters[EYE_VELOCITY_H]));
    }
    if (!(parameters[EYE_VELOCITY_V].empty())) {
        eyeVelocityVIndex = registerOutput(VariablePtr(parameters[EYE_VELOCITY_V]));
    }
}

	
EyeStatusMonitorVer1::~EyeStatusMonitorVer1() {
    delete eyeStatusComputer;
}


void EyeStatusMonitorVer1::processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MWTime _eyeTimeUS) {
    
    // call a transform object to do the computation of eye status right away
    //mprintf("  *** calling eyeStatus computer...");
    eyeStatusComputer->input(_eyeHdeg, _eyeVdeg, _eyeTimeUS);
    
    if (eyeStatusComputer->output(eyeStatus, eyeStatusTimeUS, eyeVelocity)) {
        // base class method to post the new eye status to the variable
        //      --> this will trigger any notifications for these vars
        //mprintf("  *** posting results from eyeStatus computer:  eyeStatus = %d", eyeStatus);
        postResults(eyeStatusIndex, (Datum)((long)eyeStatus), eyeStatusTimeUS);
        if (-1 != eyeVelocityHIndex) {
            postResults(eyeVelocityHIndex, eyeVelocity.h, eyeStatusTimeUS);
        }
        if (-1 != eyeVelocityVIndex) {
            postResults(eyeVelocityVIndex, eyeVelocity.v, eyeStatusTimeUS);
        }
    }
    
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
void EyeStatusComputer::input(float _eyeHdeg, float _eyeVdeg, MWTime _eyeTimeUS) {

    // not that only one time is passed in, and it is carried with the H channel
    //  (it is assumed that the H and V values passed in have the same time)

    eyeLocDeg.h = _eyeHdeg;
    eyeLocDeg.v = _eyeVdeg;

   
    eyeSpeedDegPerSec.valid = false;
    MWTime timeUS, timeUS_2, dummyTime;
        	

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
	

bool EyeStatusComputer::output(EyeStatusEnum &eyeState, MWTime &eyeStatusTimeUS, DOUBLE_POINT &eyeVelocity) {

    if (eyeInformationComputedSuccessfully) {
        eyeState = computedEyeStatus;
        eyeStatusTimeUS = computedEyeStatusTimeUS;
        eyeVelocity = eyeVelocityDegPerSec;
        return true;
    } else {
        eyeState = M_EYE_STATUS_UNKNOWN;
		shared_ptr <Clock> clock = Clock::instance();
        eyeStatusTimeUS = clock->getCurrentTimeUS();
        eyeVelocity = { std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() };
        return false;
    }
}

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
    
    
    void PairedEyeData::putDataH(double eyeH, MWTime timeUS) {
                eyeH_buffer->putData(eyeH, timeUS);
    }
    void PairedEyeData::putDataV(double eyeV, MWTime timeUS) {
                eyeV_buffer->putData(eyeV, timeUS);
    }
 
    // need to be sophisticated about waiting for pairs of H and V before transforming to final output
    // ideally, all data should be paired (ITC does this)
    // here, we just insure that the pairs that are sent for processing have stamped times within M_WARN_EYE_PAIR_SEPARATION_US of each other.
 
    bool PairedEyeData::getAvailable(double *pEyeH, double *pEyeV, MWTime *pEyeTimeUS) {
    
    if ( (eyeH_buffer_reader->getNItemsUnserviced()==0) || 
                (eyeV_buffer_reader->getNItemsUnserviced()==0) ) {
       return false;        //  no data in one or more buffers
    }
    
    bool diffWarned = false;
    
    // keep trying to find paired data
    long diff;
    MWTime eyeVtimeUS, eyeHtimeUS;
    while ( (eyeH_buffer_reader->getNItemsUnserviced()>0) && 
                (eyeV_buffer_reader->getNItemsUnserviced()>0) ) {
                
        *pEyeV = (double)(eyeV_buffer_reader->getData());
        eyeVtimeUS = eyeV_buffer_reader->getTime();
                 
        *pEyeH = (double)(eyeH_buffer_reader->getData());
        eyeHtimeUS = eyeH_buffer_reader->getTime();
        
        diff = (long)(eyeHtimeUS-eyeVtimeUS);
        
        if (std::abs(diff) <= M_MAXIMAL_ALLOWED_EYE_PAIR_SEPARATION_US) {   // data alignment acceptable to use 
            *pEyeTimeUS = eyeHtimeUS;
            eyeH_buffer_reader->advance();
            eyeV_buffer_reader->advance();
            return true;  // data should be used by object
        }
    
        if (!diffWarned) {
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                "mPairedEyeData:  Eye H and V data are not paired in time within required limit of %d us. They are separated by %ld us  (+ means old V data has no paired H). Correcting by discarding unpaired data.", 
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


END_NAMESPACE_MW
