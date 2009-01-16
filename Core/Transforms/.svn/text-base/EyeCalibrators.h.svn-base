/*
 *  EyeCalibrators.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 8/30/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#ifndef _EYE_CALIBRATORS
#define _EYE_CALIBRATORS

#include "GenericData.h"
#include "VariableTransformAdaptors.h"
#include "FilterTransforms.h"
#include "FitableFunctions.h"
#include "Averagers.h"

// need these for the calibraiton point
#include "StandardStimuli.h"
#include "Trigger.h"
#include "ExpandableList.h"	

#include "Announcers.h"
#include "EyeMonitors.h"    

namespace mw {
	
	enum CalibratorRequestedAction {   CALIBRATOR_NO_ACTION = 0, 
		CALIBRATOR_ACTION_SET_PARAMS_TO_DEFAULTS, 
	CALIBRATOR_ACTION_SET_PARAMS_TO_CONTAINED};
	
	
	
	// interface object to hold the gold standard values
	// any object that might be used for calibration should inherit from this base
	// so that any Calibrator objectcan use a standard method for returning 
	// the "gold standard" values for the calibration.
	
	class GoldStandard {
		
	protected:
		ExpandableList<Data> *goldStandardValues;
		
	public:
		GoldStandard();
		virtual ~GoldStandard();
		// this is the standard intervace method:
		virtual ExpandableList<Data> *getGoldStandardValues(); 
	};
	
	
	// an FixationPoint object can do three things:
	// 1) it can use stimulus methods to draw itself
	// 2) it can use SquareRegionTrigger methods to detect when it is triggered (e.g. eye is close)
	// 3) it can provide "gold standard" values to any calibrator object when asked
	//    (for this particular class, the gold standard values are derived from the stimulus location)
	class FixationPoint : public PointStimulus, public SquareRegionTrigger, public GoldStandard {
		
	protected:
		//shared_ptr<Variable> xoffset;      // keep track of location of the point
		//shared_ptr<Variable> yoffset;
		
		
	public:
		FixationPoint(std::string _tag, 
					  shared_ptr<Variable> _xoffset, 
					  shared_ptr<Variable> _yoffset, 
					  shared_ptr<Variable> _xscale, 
					  shared_ptr<Variable> _yscale, 
					  shared_ptr<Variable> _rot,
					  shared_ptr<Variable> _alpha,
					  shared_ptr<Variable> _r,
					  shared_ptr<Variable> _g, 
					  shared_ptr<Variable> _b,
					  shared_ptr<Variable> _triggerWidth, 
					  shared_ptr<Variable> _triggerWatchx, 
					  shared_ptr<Variable> _triggerWatchy,
					  shared_ptr<Variable> _triggerVariable);
		
		FixationPoint(const FixationPoint& tocopy);
		virtual ~FixationPoint();
		
		
		virtual ExpandableList<Data> *getGoldStandardValues();
		virtual Data getCurrentAnnounceDrawData();     // override of PointStimulus method to announce when drawn
		
		virtual Stimulus *frozenClone();
	};
	
	class FixationPointFactory : public ComponentFactory {
		virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
													   mwComponentRegistry *reg);
	};
	
	
	// ===========================================================================
	
	
	// basic calibrator -- it handles storage of data and calls to fit function
	//  all forms of calibrators should derive from this.
	//  they all take some number of vars as input and update some number of vars as output
	//  They all require actions to get the data in (newDataReceived)
	//  They all need to know when data to make calibration is avaialble and
	//      what the should use as the values to calibrato to ("GoldStandard")
	//  They all need some way to try to update the calibration surface (calibrateNow)
	// TODO -- some way to only use the most recent data in the calibration?
	
	class Calibrator : public VarTransformAdaptor, public Announcable, public Requestable, public PrivateDataStorable {
		
	protected:
		ExpandableList<FitableFunction> *fitableFunctions;
		ExpandableList<Averager> *averagers;
		Data *pUncalibratedData;
		Data *pSampledData;
		Data *pCalibratedData; 
		MonkeyWorksTime timeOfMostRecentUncalibratedDataUS;  
		shared_ptr<Variable> parameterVariable;
		bool initialized;  
		void initialize();
		std::string uniqueCalibratorName;
		void reportParameterUpdate();
		virtual void announceCalibrationUpdate();
		virtual void setPrivateParameters();  
		virtual void announceCalibrationSample(int outputIndex, Data SampledData, 
											   Data DesiredOutputData, Data CalibratedOutputData, MonkeyWorksTime timeOfSampleUS);
		virtual bool checkRequest(Data original_data);
		virtual CalibratorRequestedAction getRequestedAction(Data dictionaryData);
		virtual bool setParametersToDefaults();
		virtual bool takeSample(shared_ptr<GoldStandard> goldStandardObject);
		
	public:
        Calibrator(std::string _tag);
		virtual ~Calibrator();
		
		// this is the main "workhorse" method (called through notifications)
		virtual void newDataReceived(int inputIndex, const Data& data, 
									 MonkeyWorksTime timeUS);
		
		// these are methods that give the user experimental control of the calibrator (Actions)                                        
		virtual bool sampleNow(shared_ptr<GoldStandard> goldStandardObject); 
		virtual bool calibrateNow();
		virtual bool clearCalibrationData();
		virtual bool clearCalibrationData(MonkeyWorksTime ageAllowedUS);
		virtual void startAverage();
		virtual bool endAverageAndSample(shared_ptr<GoldStandard> goldStandardObject);
		virtual bool endAverageAndIgnore();
		
		// these are methods by which variables interact with the calibrator
		//  e.g. client sets request variable to set defaults, etc.
		// private is the way that saved variables find their way in (parameters)
		virtual void notifyRequest(const Data& original_data, MonkeyWorksTime timeUS);
		virtual void notifyPrivate(const Data& original_data, MonkeyWorksTime timeUS);
		
	};
	
	
	
	// for eye calibrator, warn if incoming samples are separated more than this
#define SAMPLE_SEPARATION_TIME_TO_WARN_MS   2
	// time to wait before warning again
#define WARN_INTERVAL_TIME_S    1
	
	
	class EyeCalibrator : public Calibrator {
		
	protected:
		int inputIndexH, inputIndexV, outputIndexH, outputIndexV;
		int HfunctionIndex, VfunctionIndex;
		MonkeyWorksTime HsampleTime;
		MonkeyWorksTime lastHtimeUS;
		MonkeyWorksTime nextTimeToWarnUS;
		PairedEyeData *pairedEyeData;
		
		Data desiredH, desiredV;
		Data sampledH, sampledV;
		Data calibratedH, calibratedV;
		virtual void announceCalibrationUpdate();
		virtual void setPrivateParameters(); 
		virtual void announceCalibrationSample(int outputIndex, Data SampledData, 
											   Data DesiredOutputData, Data CalibratedOutputData, MonkeyWorksTime timeOfSampleUS);
		virtual void tryToUseDataToSetParameters(Data original_data);
		
		
	public:
        EyeCalibrator(std::string _tag, shared_ptr<Variable> _eyeHraw, shared_ptr<Variable> _eyeVraw,
					  shared_ptr<Variable> _eyeHcalibrated, shared_ptr<Variable> _eyeVcalibrated, const int order=2);
		virtual ~EyeCalibrator();
		virtual void notifyRequest(const Data& original_data, MonkeyWorksTime timeUS);
		virtual void notifyPrivate(const Data& original_data, MonkeyWorksTime timeUS);
		
		// override base class so that we can wait for paired samples
		virtual void newDataReceived(int inputIndex, const Data& data, 
									 MonkeyWorksTime timeUS);
		
		
		
	};
	
	
	class EyeCalibratorFactory : public ComponentFactory {
		
	public:
		
		virtual shared_ptr<mw::Component> createObject(std::map<std::string, 
													   std::string> parameters,
													   mwComponentRegistry *reg);
	};
	
	class LinearEyeCalibratorFactory : public ComponentFactory {
		
	public:
		
		virtual shared_ptr<mw::Component> createObject(std::map<std::string, 
													   std::string> parameters,
													   mwComponentRegistry *reg);
	};
	
}

#endif

