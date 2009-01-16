/*
 *  EyeMonitors.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 8/9/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

// summary:
//  eye monitors:  base version.  Ver1 and ver2 are dervired from base.
//    the base version handles basic eye data pairing and filtering
//    the derived version sub in differnt eye computers 
//     because of this, ver 2 provides outputs about saccades that ver1 does not.
//
//  eye computers:  The basic version just computed eye state based on eye speed
//              The derived version computes saccades as well.
//
// As of August 20, the eyeComputer with saccades was still in development


#ifndef _EYE_MONITORS
#define _EYE_MONITORS

#include "VariableTransformAdaptors.h"
#include "FilterTransforms.h"
namespace mw {

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
	MonkeyWorksTime timeUS;
} DataWithTime;


// saccade data 
typedef struct  {
	DOUBLE_POINT	sacStartLocDeg;
    MonkeyWorksTime sacStartTimeMS;
	DOUBLE_POINT    sacEndLocDeg;               // point at which changed to fixated
	MonkeyWorksTime sacEndTimeMS;
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
        virtual void putDataH(double eyeH, MonkeyWorksTime timeUS);
        virtual void putDataV(double eyeV, MonkeyWorksTime timeUS);
        virtual bool getAvailable(double *pEyeH, double *pEyeV, MonkeyWorksTime *pEyeTimeUS);
        virtual void reset();
        
};



class EyeStatusComputer;
class EyeStatusComputerWithSaccades;

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
        MonkeyWorksTime  eyeStatusTimeUS;
        
        // computer (transform derived object to do the work)
        EyeStatusComputer  *eyeStatusComputer;
        virtual void processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MonkeyWorksTime _eyeTimeUS);
        
	public:
        EyeStatusMonitor(shared_ptr<Variable> _eyeHCalibratedVar, shared_ptr<Variable> _eyeVCalibratedVar, 
                                shared_ptr<Variable> _eyeStatusVar, int _filterWidthSamples);
		virtual ~EyeStatusMonitor();
		
		// override of VarTransformAdaptor class
		virtual void newDataReceived(int inputIndex, const Data& data, MonkeyWorksTime timeUS);
        virtual void reset();
        
};

class EyeStatusMonitorVer1 : public EyeStatusMonitor {
      
    protected:
        virtual void processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MonkeyWorksTime _eyeTimeUS);
    
    public:                                    
        EyeStatusMonitorVer1(shared_ptr<Variable> _eyeHCalibratedVar, 
                    shared_ptr<Variable> _eyeVCalibratedVar, shared_ptr<Variable> _eyeStatusVar, 
                    int _filterWidthSamples, 
                    shared_ptr<Variable> _saccadeEntrySpeedDegPerSec,
                    shared_ptr<Variable> _saccadeExitSpeedDegPerSec);
                                
        virtual ~EyeStatusMonitorVer1();                                                                                          
};

class EyeStatusMonitorVer1Factory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


class EyeStatusMonitorVer2 : public EyeStatusMonitor {
   
         
    protected:
        virtual void processAndPostEyeData(double _eyeHdeg, double _eyeVdeg, MonkeyWorksTime _eyeTimeUS);
    
    public:    
        EyeStatusMonitorVer2(shared_ptr<Variable> _eyeHCalibratedVar, 
                    shared_ptr<Variable> _eyeVCalibratedVar, shared_ptr<Variable> _eyeStatusVar, 
                    int _filterWidthSamples, 
                    shared_ptr<Variable> _saccadeEntrySpeedDegPerSec,
                    shared_ptr<Variable> _saccadeExitSpeedDegPerSec,
                    shared_ptr<Variable> _minimumStartDirectionChangeSeg,
                    shared_ptr<Variable> _minimumAmplitudeDeg,
                    shared_ptr<Variable> _minimumDurationMS);                        
        virtual ~EyeStatusMonitorVer2();               

};

class EyeStatusMonitorVer2Factory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};


// this is the basic model
class EyeStatusComputer : public Transform {
           
    protected:   
    
        // outputs returned
        bool                eyeInformationComputedSuccessfully;
        EyeStatusEnum      computedEyeStatus;
        MonkeyWorksTime     computedEyeStatusTimeUS;
        
        // parameters
        shared_ptr<Variable> sacStartSpeedDegPerSec;
        shared_ptr<Variable> sacEndSpeedDegPerSec;
        
        // intermediate vars
        DOUBLE_POINT        Null_doublePoint;
        
        // these all reflect the latest computed values
        // the time used is the one associated with the speed
        DataWithTime        eyeSpeedDegPerSec; 
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
        virtual void input(float _eyeH, float _eyeV, MonkeyWorksTime _eyeTimeUS);
        virtual bool output(EyeStatusEnum *eyeStatus, MonkeyWorksTime *eyeStatusTimeUS);
        virtual void reset();
       
};

// this model handles saccade detection also and improves its performance on status monitoring because of that
class EyeStatusComputerWithSaccades : public EyeStatusComputer {

     protected: 
        
        // outputs
        bool             saccadeInformationIsAvailable;        
        SaccadeData     computedSaccadeData;
        
        // parameters for detection:
        shared_ptr<Variable> minSacAmplitudeDeg;
        shared_ptr<Variable> sacStartDirectionChangeDeg;
        shared_ptr<Variable> minSacDurationMS;  
        
        // saccade detection stuff
        MonkeyWorksTime     timeofLastReset;
        bool                engageSaccadeDetection;
        bool                checkForSaccadeStart;
        bool                computeSacTrueEnd, inSaccade, checkForSaccadeEnd;
        DataWithTime        eyeSpeedDegPerSecLast; 
        DOUBLE_POINT        eyeVelocityDegPerSecLast;
        DOUBLE_POINT        eyeLocDegLast;
        double              eyeDirectionAngle;
        EyeStatusEnum      computedEyeStatusLast;
        
        // internal vars
        //bool 	prediction;   
        short			lookBackSamplesToActual;
        bool            getPeak;
        short           down;
        float           lengthToPeak;
        DOUBLE_POINT    peakLoc;
        double          sacAmplitudePiecewise;
        double          sacPeakSpeed;
        bool            sacBeyondVelocityPeak;
        short           nSamplesAveraged;
        SaccadeData    sacData;
        MonkeyWorksTime timeSinceSacDetectedEnd;
        DOUBLE_POINT    xx;  
        
        // override its base class method             
        virtual bool    computeEyeInformation();  
        
                      
    public:
        EyeStatusComputerWithSaccades(int _filterWidthSamples,
                shared_ptr<Variable> _sacStartSpeedDegPerSec, shared_ptr<Variable> _sacEndSpeedDegPerSec,
                shared_ptr<Variable> _sacStartDirectionChangeDeg, shared_ptr<Variable> _minSacAmplitudeDeg, shared_ptr<Variable> _minSacDurationMS);
                            
        virtual ~EyeStatusComputerWithSaccades();        
        virtual void reset();           // override
        virtual bool outputSaccade(SaccadeData *saccadeData);  // new method for this class
    
};


        
        /*
        bool 	prediction;   
        DOUBLE_POINT 	overshoot;
        short			lookBackSamplesToActual;
        double  		eyeDirection;
        short i;
        short boxWidthSamples;
        bool first;
        bool getPeak;
        short	down;
        float lengthToPeak;
        DOUBLE_POINT peakLoc;
        DOUBLE_POINT  DeyeLocFiltered;
        
        DOUBLE_POINT 		eyeVelocityUnfiltered;
        DOUBLE_POINT eyeLocLastUnfiltered;
        
    
        DOUBLE_POINT 	eyeVelocityStore[MAX_LOOK_BACK_SAMPLES];
        DOUBLE_POINT 	eyeLocStore[MAX_LOOK_BACK_SAMPLES];
        float 			eyeSpeedStore[MAX_LOOK_BACK_SAMPLES];
        DOUBLE_POINT 	bufferLoc[MAX_LOOK_BACK_SAMPLES];
        float 			bufferDirection[MAX_LOOK_BACK_SAMPLES];
        */




}

#endif

