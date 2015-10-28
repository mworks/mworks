/*
 *  EyeMonitors.h
 *  MWorksCore
 *
 *  Created by dicarlo on 8/9/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#ifndef _EYE_MONITORS
#define _EYE_MONITORS

#include "VariableTransformAdaptors.h"
#include "FilterTransforms.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


enum EyeStatusEnum{ M_EYE_STATUS_FIXATING=0, M_EYE_STATUS_SACCADING, M_EYE_STATUS_UNKNOWN};

// buffer sizes to hold eye data
// These need to be big enough to allow BOTH channels to get in synch
// (e.g. it is conceivable that a bunch of data can be dumped from one eye chan before
//  the other chans data is dumped)
#define M_ASSUMED_EYE_SAMPLES_PER_MS 1
#define M_MAX_EYE_BUFFER_SIZE_MS     100    

// adjust if the eye H and eye V data streams drift relative to each other by more than this amount.
//  For a device like the itc and the same sampling rate for each channel, 
//      this should NEVER happen, but it could if the channels
//      are (e.g.) collected on different devices or at different sampling rates 
//      or if one channel gets started before another or stops before another.
//  If we exceed this limit, data are discarded.  Only samples within this limit are used.    
#define M_MAXIMAL_ALLOWED_EYE_PAIR_SEPARATION_US   2000    

// ================== very specific instances of var transform adaptors =====


// Sructure used to bind h and v eye data together (old DLAB)
typedef struct {
	double h;
	double v;
} DOUBLE_POINT;

typedef struct {
    bool    valid;
	double  value;
	MWTime timeUS;
} DatumWithTime;


// saccade data 
typedef struct  {
	DOUBLE_POINT	sacStartLocDeg;
    MWTime sacStartTimeMS;
	DOUBLE_POINT    sacEndLocDeg;               // point at which changed to fixated
	MWTime sacEndTimeMS;
    DOUBLE_POINT	sacTrueEndLocDeg;           // averaging post-saccade (more accurate estimate of end point)
	double          distanceDeg;                // "TRUE" vector distance
	double          distanceDegPiecewise;       // "TRUE" piecewise distance
	double          peakSpeedDegPerSec;
	long            durationMS;
	bool            peakDetected;
	double          peakBasedPredictedLengthDeg;
	DOUBLE_POINT	peakBasedPredictedEndLocDeg;	// crude prediction of end loc of this saccade	(based on position at peak)
} SaccadeData;



class PairedEyeData {

	private:
        
        // buffers to hold eye data
        FloatDataTimeStampedRingBuffer *eyeH_buffer;  
        FloatDataTimeStampedRingBuffer *eyeV_buffer; 
        FloatDataTimeStampedBufferReader *eyeH_buffer_reader; // reader
        FloatDataTimeStampedBufferReader *eyeV_buffer_reader; // reader
        
	public:
        PairedEyeData(int buffer_size);
        virtual ~PairedEyeData();
        virtual void putDataH(double eyeH, MWTime timeUS);
        virtual void putDataV(double eyeV, MWTime timeUS);
        virtual bool getAvailable(double *pEyeH, double *pEyeV, MWTime *pEyeTimeUS);
        virtual void reset();
        
};



class EyeStatusComputer;

// output current status of the eyes (FIXATED, SACCADING, UNKNOWN)
// TODO not yet clear if we realy need to inherit from the TransformWithMemory class here
class EyeStatusMonitor : public VarTransformAdaptor {

	private:
        // filters on each of the inputs
		BoxcarFilter1D *filterH;
		BoxcarFilter1D *filterV;

        // buffer to hold and pair filtered eye data    
        PairedEyeData *pairedEyeData; 
    
    
    protected:
        // maping to registered ins and outs    
		int eyeHindex;
		int	eyeVindex;
        int eyeStatusIndex;
        
        // main monitor vars (output)
		EyeStatusEnum   eyeStatus;
        MWTime  eyeStatusTimeUS;
        
        // computer (transform derived object to do the work)
        EyeStatusComputer  *eyeStatusComputer;
        virtual void processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MWTime _eyeTimeUS) = 0;
        
	public:
        EyeStatusMonitor(shared_ptr<Variable> _eyeHCalibratedVar, shared_ptr<Variable> _eyeVCalibratedVar, 
                                shared_ptr<Variable> _eyeStatusVar, int _filterWidthSamples);
		virtual ~EyeStatusMonitor();
		
		// override of VarTransformAdaptor class
		virtual void newDataReceived(int inputIndex, const Datum& data, MWTime timeUS);
        virtual void reset();
        
};

class EyeStatusMonitorVer1 : public EyeStatusMonitor {
      
    protected:
        int eyeVelocityHIndex;
        int eyeVelocityVIndex;
    
        DOUBLE_POINT eyeVelocity;
    
        virtual void processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MWTime _eyeTimeUS);
    
    public:
        static const std::string EYEH_CALIBRATED;
        static const std::string EYEV_CALIBRATED;
        static const std::string EYE_STATE;
        static const std::string EYE_VELOCITY_H;
        static const std::string EYE_VELOCITY_V;
        static const std::string WIDTH_SAMPLES;
        static const std::string SACCADE_ENTRY_SPEED;
        static const std::string SACCADE_EXIT_SPEED;
    
        static void describeComponent(ComponentInfo &info);
    
        explicit EyeStatusMonitorVer1(const ParameterValueMap &parameters);
    
        virtual ~EyeStatusMonitorVer1();                                                                                          
};


class EyeStatusComputer : public Transform {
           
    protected:   
    
        // outputs returned
        bool                eyeInformationComputedSuccessfully;
        EyeStatusEnum      computedEyeStatus;
        MWTime     computedEyeStatusTimeUS;
        
        // parameters
        shared_ptr<Variable> sacStartSpeedDegPerSec;
        shared_ptr<Variable> sacEndSpeedDegPerSec;
        
        // intermediate vars
        DOUBLE_POINT        Null_doublePoint;
        
        // these all reflect the latest computed values
        // the time used is the one associated with the speed
        DatumWithTime        eyeSpeedDegPerSec; 
        DOUBLE_POINT        eyeVelocityDegPerSec;
        DOUBLE_POINT        eyeLocDeg;
        
        VelocityComputer1D *velocityComputerH;
        VelocityComputer1D *velocityComputerV;
        BoxcarFilter1D *velocityFilterH;
        BoxcarFilter1D *velocityFilterV;
    
    
        double	magnitude(DOUBLE_POINT vector1, DOUBLE_POINT vector2);
        double	phase(DOUBLE_POINT vector1, DOUBLE_POINT vector2);     
        virtual bool computeEyeInformation();   // derived classes can improve on this
        
    public:

        EyeStatusComputer(int _filterWidthSamples,
                            shared_ptr<Variable> _sacStartSpeedDegPerSec, shared_ptr<Variable> _sacEndSpeedDegPerSec);
        virtual ~EyeStatusComputer();        
        virtual void input(float _eyeH, float _eyeV, MWTime _eyeTimeUS);
        virtual bool output(EyeStatusEnum &eyeStatus, MWTime &eyeStatusTimeUS, DOUBLE_POINT &eyeVelocity);
        virtual void reset();
       
};


END_NAMESPACE_MW


#endif

