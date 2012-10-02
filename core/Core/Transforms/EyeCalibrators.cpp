/*
 *  EyeCalibrators.cpp
 *  MWorksCore
 *
 *  Created by dicarlo on 8/30/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "EyeCalibrators.h"
#include "Averagers.h"
#include "StandardVariables.h"
#include "ComponentFactory.h"
#include <boost/regex.hpp>
#include "ParsedColorTrio.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


#define VERBOSE_EYE_CALIBRATORS 0

GoldStandard::GoldStandard() {
    goldStandardValues = new ExpandableList<Datum>();
}

GoldStandard::~GoldStandard() {
    delete goldStandardValues;
}

ExpandableList<Datum> *GoldStandard::getGoldStandardValues() {
    return goldStandardValues;
}


const std::string FixationPoint::TRIGGER_WIDTH("trigger_width");
const std::string FixationPoint::TRIGGER_WATCH_X("trigger_watch_x");
const std::string FixationPoint::TRIGGER_WATCH_Y("trigger_watch_y");
const std::string FixationPoint::TRIGGER_FLAG("trigger_flag");


void FixationPoint::describeComponent(ComponentInfo &info) {
    PointStimulus::describeComponent(info);
    info.setSignature("stimulus/fixation_point");
    info.addParameter(TRIGGER_WIDTH);
    info.addParameter(TRIGGER_WATCH_X);
    info.addParameter(TRIGGER_WATCH_Y);
    info.addParameter(TRIGGER_FLAG);
}


FixationPoint::FixationPoint(const ParameterValueMap &parameters):
    PointStimulus(parameters),
    SquareRegionTrigger(parameters[X_POSITION],
                        parameters[Y_POSITION],
                        parameters[TRIGGER_WIDTH],
                        parameters[TRIGGER_WATCH_X],
                        parameters[TRIGGER_WATCH_Y],
                        parameters[TRIGGER_FLAG])
{ }


ExpandableList<Datum> *FixationPoint::getGoldStandardValues() {

    // return the center location of the fixation point
    goldStandardValues->clear();
    
    Datum h_loc = xoffset->getValue();    // unitless screen units    
    goldStandardValues->addElement(h_loc);      // copy
    
    Datum v_loc = yoffset->getValue();      // unitless screen units 
    goldStandardValues->addElement(v_loc);
	
    return goldStandardValues;
}    


// override of PointStimulus announce method -- allows trigger info to also be announced
Datum FixationPoint::getCurrentAnnounceDrawData() {
    
    //if (VERBOSE_EYE_CALIBRATORS> 1) mprintf("getting announce DRAW data for fixation point stimulus %s",tag );
    
    Datum announceData(M_DICTIONARY, 14);
    announceData.addElement(STIM_NAME,getTag());        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_POINT);
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
    announceData.addElement(STIM_COLOR_R,last_r);  
    announceData.addElement(STIM_COLOR_G,last_g);  
    announceData.addElement(STIM_COLOR_B,last_b);  
    
    // stuff from the trigger that is not in point stimulus ...
    announceData.addElement("center_x", (double)(*centerx));
    announceData.addElement("center_y", (double)(*centery));
    announceData.addElement("width", (double)(*width));

    return (announceData);
}


// =====================================================


Calibrator::Calibrator(std::string _tag):Announcable(ANNOUNCE_CALIBRATOR_TAGNAME), 
                                     Requestable(REQUEST_CALIBRATOR_TAGNAME),
                                     PrivateDataStorable(PRIVATE_CALIBRATOR_TAGNAME,_tag) {
    initialized = false;
    
    //save the unique user tag  
    uniqueCalibratorName = _tag;
}

Calibrator::~Calibrator() {}

void Calibrator::initialize() {

    pUncalibratedData = new Datum(M_LIST, this->getNumInputs() );
    pSampledData = new Datum(M_LIST, this->getNumInputs() );
    
    pCalibratedData = new Datum(M_LIST, this->getNumOutputs() );
    fitableFunctions = new ExpandableList<FitableFunction>( this->getNumOutputs() );
    
    // set vectors to 0 as defaults
    for (int i=0;i<pUncalibratedData->getMaxElements();i++) {
        pUncalibratedData->setElement(i, (double)0);
        pSampledData->setElement(i, (double)0);
    }
    for (int i=0;i<pCalibratedData->getMaxElements();i++) {
        pCalibratedData->setElement(i, (double)0);
    }
    
    // create an averager for each input line (also feeds off the input variable)
    averagers = new ExpandableList<Averager>( this->getNumInputs() );
    shared_ptr<Variable> inputVariable;
    for (int inputIndex=0;inputIndex<this->getNumInputs();inputIndex++) { 
        if (this->getInputVariable(inputIndex, &inputVariable)) {
            shared_ptr<Averager> averager(new Averager(inputVariable)); 
            averagers->addReference(inputIndex, averager);
        }
    }
    
    // announce the presence of the calibrator
    Datum announceData(M_DICTIONARY, 2);
    announceData.addElement(CALIBRATOR_NAME,uniqueCalibratorName);    // char
    announceData.addElement(CALIBRATOR_ACTION,CALIBRATOR_ACTION_IDENTIFY_SELF);    
    announce(announceData);    // announce things here using method from Announcable

    initialized = true;
}


// go through all the fitable functions and set parameters to defaults
bool Calibrator::setParametersToDefaults() {
    
    bool noErr = true;
    bool paramsChanged = false;
    
    for (int outputIndex = 0; outputIndex<this->getNumOutputs(); outputIndex++) {
        shared_ptr<FitableFunction> fitFunction = (fitableFunctions->getElement(outputIndex));
        if (fitFunction->setParametersToDefaults()) {
            paramsChanged = true;
        }
        else {
            noErr = false;
        }
    }
    
    if (paramsChanged) reportParameterUpdate();
    
    return noErr;
}


// PUBLIC METHOD
// this is the common entry method for all "input" variables of any calibrator
// thus, it is invoked every time any one of the input variables is updated
// this happens through a notification object that communicates with this class
//  at the level of the base class "mVarTransformAdaptor".
// the main jobs of this method are to:
//    1) locally "save" the data the is brought in on the input variable.
//    2) apply the current calibration function (operates over "saved" values of potentially ALL input variables) 
void Calibrator::newDataReceived(int inputIndex, const Datum& data, 
                                                MWTime timeUS) {
    
	lock(); // DDC added
    if (!initialized){
		unlock();
		return;
    }
	
	
    // for now, just keep the most recent value (for calibration)
    pUncalibratedData->setElement(inputIndex,data);   // this is used for filtering
    pSampledData->setElement(inputIndex,data);        // this is what is used for calibration samples
    timeOfMostRecentUncalibratedDataUS = timeUS;
        
    
    // if the averager is running, take the sample  
    // (this happens in the background -- the averager is instatiated when this object is instantiated)
        
        
    // compute the calibrated values if possible
    // -- note -- this will compute ALL of the outputs given the inputs
    //Ffor calibrators with more than one output (e.g. eye calibrators), this
    //  means that the calibrated eye values will be posted (e.g.) twice as
    // often as the uncalibrated eye samples.  
    // I think this is the right behavior, and it can easily be overriden if desired.
    
    bool noErr = true;
    Datum calibData;
    for (int outputIndex=0;outputIndex<this->getNumOutputs();outputIndex++) {
        noErr = (fitableFunctions->getElement(outputIndex))->
                        //applyTheFunction(pUncalibratedData, &calibData);  // DDC fix
						applyTheFunction(*pUncalibratedData, &calibData);
        if (noErr) {
            postResults(outputIndex, calibData, timeUS);               
            pCalibratedData->setElement(outputIndex,calibData);   // keep a copy of the last outs
        }
    }
    
    unlock(); // DDC added
}

// PROTECTED METHOD    
bool Calibrator::takeSample(shared_ptr<GoldStandard> goldStandardObject) {

    bool noErr = false;
    if (initialized) {
     
        // go get the true calibration values from the gold standard object
        // mprintf("Calibrator is calling goldStandard object to get gold standard values.");
        ExpandableList<Datum> *gold_standard_values = goldStandardObject->getGoldStandardValues();
        
        if (gold_standard_values->getNElements() !=  this->getNumOutputs()) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, 
				"Number of gold standard values does not match number of outputs of calibrator.");
            return (noErr);  
        }
        if (fitableFunctions->getNElements() !=  this->getNumOutputs()) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, 
				"Number of fitable functions does not match number of outputs of calibrator.");
           return (noErr);  
        }
        
    
        // send the input and desired output data to each of the fitable functions
        for (int outputIndex = 0; outputIndex<this->getNumOutputs(); outputIndex++) {
        
             shared_ptr<Datum> pDesiredOutputData = gold_standard_values->getElement(outputIndex);
             
            // prompt fit funciton with most recent set of uncalibrated data and desired output data
            //if (VERBOSE_EYE_CALIBRATORS) mprintf("mCalibrator::taking sample now.  outputIndex: %d.",outputIndex);
            shared_ptr<FitableFunction> fitFunction = (fitableFunctions->getElement(outputIndex));
            fitFunction->acceptDataForFit(
                            pSampledData,
                            *pDesiredOutputData,        // pass Datum by value
                            timeOfMostRecentUncalibratedDataUS);
                            
                            
            // announce sample that was taken
            //if (VERBOSE_EYE_CALIBRATORS) mprintf("mCalibrator::announcing take sample.  outputIndex: %d.",outputIndex);
            Datum CalibratedOutputData; 
            //noErr = fitFunction->applyTheFunction(pSampledData, &CalibratedOutputData); // DDC fix
			noErr = fitFunction->applyTheFunction(*pSampledData, &CalibratedOutputData);
            announceCalibrationSample(outputIndex,*pSampledData,*pDesiredOutputData, CalibratedOutputData, timeOfMostRecentUncalibratedDataUS);
        }
        noErr = true;                                                   
        if (VERBOSE_EYE_CALIBRATORS) mprintf("mCalibrator::takeSample method ran successfully.");
        
    }
	
    return (noErr);  
}    


// PUBLIC METHOD
bool Calibrator::sampleNow(shared_ptr<GoldStandard> goldStandardObject) {

	lock(); // DDC added
	
    bool noErr = false;
    if (initialized) {
        
        // set the sample values to the last input values
        for (int inputIndex = 0; inputIndex<this->getNumInputs(); inputIndex++) {
            Datum data = pUncalibratedData->getElement(inputIndex);
            pSampledData->setElement(inputIndex,data); 
        }
        if (VERBOSE_EYE_CALIBRATORS) mprintf("mCalibrator::sampleNow method ran successfully.");
         
        // call the method to fit
        noErr = this->takeSample(goldStandardObject);
    }
    
	unlock(); // DDC added
	return (noErr);
}


// PUBLIC METHOD
// start taking the average
void Calibrator::startAverage() {

	lock(); // DDC added

    for (int inputIndex = 0; inputIndex<this->getNumInputs(); inputIndex++) {
        shared_ptr<Averager> averager = averagers->getElement(inputIndex);
        averager->reset();
        averager->start();
    }
	
	unlock(); // DDC added
}


// PUBLIC METHOD
// stop taking the average and use this as a sample
bool Calibrator::endAverageAndSample(shared_ptr<GoldStandard> goldStandardObject) {

	lock(); // DDC added

    bool noErr = false;
    if (initialized) {
        
        // set the sample values to the average values
        for (int inputIndex = 0; inputIndex<this->getNumInputs(); inputIndex++) {
            shared_ptr<Averager> averager = averagers->getElement(inputIndex);
            averager->stop();
            Datum data = averager->getAverage();
            pSampledData->setElement(inputIndex,data); 
            averager->reset();
        }
        
        // call the method to fit
        noErr = this->takeSample(goldStandardObject);
    }
	
	unlock(); // DDC added
    return (noErr);
}


// PUBLIC METHOD
// stop taking the average and use this as a sample
bool Calibrator::endAverageAndIgnore() {
    
	lock(); // DDC added
	
	for (int inputIndex = 0; inputIndex<this->getNumInputs(); inputIndex++) {
        shared_ptr<Averager> averager = averagers->getElement(inputIndex);
        averager->stop();
        averager->reset();
    }
	unlock(); // DDC added
	return true;
}
 
// PUBLIC METHOD      
// fit the function using the currentlhy available data
bool Calibrator::calibrateNow(){
    
	lock(); // DDC added
	
	bool noErr = true;
   
    if (!initialized) {
		unlock(); // DDC added
		return (false); 
	}
        
    for (int outputIndex = 0; outputIndex<this->getNumOutputs(); outputIndex++) {
        noErr = (fitableFunctions->getElement(outputIndex))->fitTheFunction();
        if (!noErr) {
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,"Warning:  calibration not successful.");
        }
    }

    // the parameters have just changed (probably).
    reportParameterUpdate();
    
	unlock(); // DDC added
    return (noErr);     // return false if any function fit fails  
}
  
void Calibrator::reportParameterUpdate() {
	announceCalibrationUpdate();             // speak params. typically overriden
    setPrivateParameters();                 // change values of private variable (used for saving)
}


// method to announce details about each sample that is acquired for calibration  
void Calibrator::announceCalibrationSample(int outputIndex, Datum SampledData, 
                     Datum DesiredOutputData, Datum CalibratedOutputData, MWTime timeOfSampleUS) {
    
    Datum announceData(M_DICTIONARY, 3);
    announceData.addElement(CALIBRATOR_NAME,uniqueCalibratorName);    // char
    announceData.addElement(CALIBRATOR_ACTION,CALIBRATOR_ACTION_SAMPLE);
    announceData.addElement("outputIndex",(long)outputIndex);
    
    if (VERBOSE_EYE_CALIBRATORS) mprintf("mCalibrator::announceCalibrationSample  Announcing now");
    announce(announceData);    // announce things here using method from Announcable
}
  

void Calibrator::announceCalibrationUpdate() {
    Datum announceData(M_DICTIONARY, 2);
    announceData.addElement(CALIBRATOR_NAME,uniqueCalibratorName);        // char
    announceData.addElement(CALIBRATOR_ACTION,CALIBRATOR_ACTION_UPDATE_PARAMS);
    // TODO announce parameters in base class ?? too messy.
    announce(announceData);    // announce things here using method from Announcable
}
   
void Calibrator::setPrivateParameters() { }  
   
   
// PUBLIC METHOD   
// clear all the data in the fitable function
bool Calibrator::clearCalibrationData(){
    
	lock(); // DDC added
    if (!initialized) {
		unlock(); // DDC added
		return false; 
	}
    
    for (int outputIndex = 0; outputIndex<this->getNumOutputs(); outputIndex++) {
       (fitableFunctions->getElement(outputIndex))->flushOldData();     
    }
    
	unlock(); // DDC added
    return true;
}     

   
// clear all the data in the fitable function
// here -- we only clear things that are more mature than the specified age
// TODO the method in the fitable funciton is not yet correct -- need to understand how to pull elements out of the linked list
// also need to understand how to interface this with the editor (Variable?)
bool Calibrator::clearCalibrationData(MWTime ageAllowedUS){
    
	lock(); // DDC added
    if (!initialized) {
		unlock(); // DDC added
		return false; 
	}
    
	shared_ptr <Clock> clock = Clock::instance();
    MWTime oldestTimeAllowedToStayInSampleListUS = 
                        clock->getCurrentTimeUS()-ageAllowedUS;
    
    //mprintf("OldestTimeAllowedToStayInSampleListUS = %d", (long)oldestTimeAllowedToStayInSampleListUS);
    
    for (int outputIndex = 0; outputIndex<this->getNumOutputs(); outputIndex++) {
       (fitableFunctions->getElement(outputIndex))->flushOldData(oldestTimeAllowedToStayInSampleListUS);      
    }
    
	unlock(); // DDC added
    return true;
}     
   
   
// this routine checks that the request is a dictionary and that it contains a name that matches the calibrator
bool Calibrator::checkRequest(Datum dictionaryData) {
      
    
    Datum data; // to hold field data for checking

    // check if this is a dictionary
    if (!(dictionaryData.getDataType() == M_DICTIONARY)) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Request sent to calibrator %s that was not expected dictionary type was ignored.", uniqueCalibratorName.c_str());
        return(false);
    }
    
    // check if there is a name field and if this calibrator should respond (i.e. if it has the correct name)
    if (!(dictionaryData.hasKey(R_CALIBRATOR_NAME))) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Request sent to calibrator %s that did not contain name field was ignored.", uniqueCalibratorName.c_str());
         return(false);
    }
    Datum nameData = dictionaryData.getElement(R_CALIBRATOR_NAME);

    if  (!(nameData.getDataType() == M_STRING)) {       // check if name field is a string
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Request sent to calibrator %s that did not contain a string in the name field was ignored.", uniqueCalibratorName.c_str());
         return(false);
    }
    
    if (uniqueCalibratorName == nameData.getString()) {       // check is name field matches the name of this calibrator
        if (VERBOSE_EYE_CALIBRATORS) mprintf("Calibrator %s successfully received a properly named request.", uniqueCalibratorName.c_str());
    }
    else {
        if (VERBOSE_EYE_CALIBRATORS) mprintf("Calibrator %s received a request, but name did not match.", uniqueCalibratorName.c_str());
         return(false);     // request not meant for this calibrator (can be normal behavior -- no warnings)
    }
        
        
    return true;
}

// overridable base class method
// assumes data have already been checked for proper dictionary, name. 
CalibratorRequestedAction Calibrator::getRequestedAction(Datum dictionaryData) {  
          
    // check what action is requested (e.g. update parameters)
    if (!(dictionaryData.hasKey(R_CALIBRATOR_ACTION))) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Request sent to calibrator %s that did not contain an action field was ignored.", uniqueCalibratorName.c_str());
         return(CALIBRATOR_NO_ACTION);
    }
    Datum actionData = dictionaryData.getElement(R_CALIBRATOR_ACTION);

    if  (!(actionData.getDataType() == M_STRING)) {       // check if name field is a string
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Request sent to calibrator %s that did not contain a string in the action field was ignored.", uniqueCalibratorName.c_str());
         return(CALIBRATOR_NO_ACTION);
    }
    
    if (strcmp(R_CALIBRATOR_ACTION_SET_PARAMETERS, actionData.getString()) == 0) {       // check is name field matches the name of this calibrator
        if (VERBOSE_EYE_CALIBRATORS>1) mprintf(
                "Calibrator %s successfully received request for to update its parameters to contained values.", uniqueCalibratorName.c_str());
        return CALIBRATOR_ACTION_SET_PARAMS_TO_CONTAINED;
    }
    else if (strcmp(R_CALIBRATOR_ACTION_SET_PARAMETERS_TO_DEFAULTS, actionData.getString()) == 0) {
        if (VERBOSE_EYE_CALIBRATORS>1) mprintf("Calibrator %s successfully received request for to update its parameters to defaults.", uniqueCalibratorName.c_str());
        return CALIBRATOR_ACTION_SET_PARAMS_TO_DEFAULTS;
    }
    else {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Calibrator %s received a request, but action was unknown. Request ignored", uniqueCalibratorName.c_str());
        return CALIBRATOR_NO_ACTION;     
    }
    
    return (CALIBRATOR_NO_ACTION);
        
}    
    

// PUBLIC METHOD   
void Calibrator::notifyRequest(const Datum& original_data, MWTime timeUS) {
    
    // base class -- not clear what to do.
    lock();
    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Notification of calibrator base class should have been overriden. Request ignored");
    unlock();
}


// PUBLIC METHOD
void Calibrator::notifyPrivate(const Datum& original_data, MWTime timeUS) {

    // base class -- not clear what to do.
    lock();
    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Notification of calibrator base class should have been overriden. Private variable change ignored");
    unlock();
}



   
// here we define a particular calibrator class by 
//  1) register a particular set of input and out vars
//  2) define a fittable function that is meant to calibrate between them
//  
// in principle, one can make a class just like this one for ANY set of vars
// and ANY fitable funciton (as long as the fitable function class is also written)
EyeCalibrator::EyeCalibrator(std::string _tag, shared_ptr<Variable> _eyeHraw, shared_ptr<Variable> _eyeVraw,
                        shared_ptr<Variable> _eyeHcalibrated, shared_ptr<Variable> _eyeVcalibrated, const int order):Calibrator(_tag) {
    
    nextTimeToWarnUS = 0;
    lastHtimeUS = 0;
                                    
    if (VERBOSE_EYE_CALIBRATORS) mprintf("mEyeCalibrator constructor has been called.");
                                                                                     
    // 1)  register inputs and outputs
    inputIndexH = (this->registerInput(_eyeHraw));
    inputIndexV = (this->registerInput(_eyeVraw));
    outputIndexH = (this->registerOutput(_eyeHcalibrated));
    outputIndexV = (this->registerOutput(_eyeVcalibrated));
    this->initialize();     // create memory for the expected data, averagers, and fitable functions

	// 2) create and assign a specific fitableFunction for each output 
	shared_ptr<FitableFunction> fitableFunctionH;
	shared_ptr<FitableFunction> fitableFunctionV;
    //  In this case, they are identical, but they do not need to be the same. (any fitable function object will work here)
    // default values for parameters are established in here.
	switch(order) {
		case 1:
			fitableFunctionH = shared_ptr<FitableFunction>(new FirstOrderPolynomialFitableFunction(this->getNumInputs(),true,inputIndexH)); 
			fitableFunctionV = shared_ptr<FitableFunction>(new FirstOrderPolynomialFitableFunction(this->getNumInputs(),true,inputIndexV)); 
			break;
		case 2:
			fitableFunctionH = shared_ptr<FitableFunction>(new SecondOrderPolynomialFitableFunction(this->getNumInputs(),true,inputIndexH)); 
			fitableFunctionV = shared_ptr<FitableFunction>(new SecondOrderPolynomialFitableFunction(this->getNumInputs(),true,inputIndexV)); 
			break;
		default:
			merror(M_PARSER_MESSAGE_DOMAIN,"%s has an unsupported order (%d)", _tag.c_str(), order);
			break;
	}
	
    HfunctionIndex = (fitableFunctions->addReference(outputIndexH, fitableFunctionH))-1;
    VfunctionIndex = (fitableFunctions->addReference(outputIndexV, fitableFunctionV))-1;
	
//     // 2) create and assign a specific fitableFunction for each output 
//    //  In this case, they are identical, but they do not need to be the same. (any fitable function object will work here)
//    // default values for parameters are established in here.
//    // first output (h)
//    shared_ptr<FitableFunction> fitableFunctionH(new SecondOrderPolynomialFitableFunction(this->getNumInputs(),true,inputIndexH)); 
//    HfunctionIndex = (fitableFunctions->addReference(outputIndexH, fitableFunctionH))-1;
//
//    // second output (v)
//    shared_ptr<FitableFunction> fitableFunctionV(new SecondOrderPolynomialFitableFunction(this->getNumInputs(),true,inputIndexV)); 
//    VfunctionIndex = (fitableFunctions->addReference(outputIndexV, fitableFunctionV))-1;
    
    // make sure fitable functions parameters are set to defaults (they will be, but nice form)           
    this->setParametersToDefaults();            
                                        
    // 3) if the private variable was found, then we should use its values to set the parameters (i.e. replace defaults)
	if(privateVariableNameAlreadyInstantiated)	{
		tryToUseDataToSetParameters(privateVariable->getValue()); 
	} 
    
    // 4) announce our current paramters  (JJD added Sept 13, 2006)
    announceCalibrationUpdate();
    
    
    // the eye calibrator uses this object to buffer and pair eye samples together.
    int buffer_size = M_ASSUMED_EYE_SAMPLES_PER_MS * M_MAX_EYE_BUFFER_SIZE_MS; 
    pairedEyeData = new PairedEyeData(buffer_size);

    
}

    


EyeCalibrator::~EyeCalibrator() {
    delete fitableFunctions;
    delete pairedEyeData;
}
    

// JJD overrode the base class function on Nov 2, 2006, so that the eye calibrator 
// will wait for paired input from BOTH channels before posting
void EyeCalibrator::newDataReceived(int inputIndex, const Datum& data, 
                                                MWTime timeUS) {
    
	lock(); 
    if (!initialized){
		unlock();
		return;
    }
	
	
    //pUncalibratedData->setElement(inputIndex,data);   // this is used for filtering
    pSampledData->setElement(inputIndex,data);        // this is what is used for calibration samples
    timeOfMostRecentUncalibratedDataUS = timeUS;
        
    // if the averager is running, take the sample  
    // (this happens in the background -- the averager is instatiated when this object is instantiated)
    
    
    // check to see if we have a correct pair of values that we should compute calibration on
    // process and post all such pairs now
    
    if (inputIndex==inputIndexH) {
        pairedEyeData->putDataH(data, timeUS);
	}
	else if (inputIndex==inputIndexV) {
        pairedEyeData->putDataV(data, timeUS);
	}
    else {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			" **** EyeStatusMonitor::newDataReceived  Unknown input index");
        unlock();
        return;
    }
	
     

    // if a pair is ready, pull it out and process it
    MWTime eyeTimeUS;
    double eyeH, eyeV;
    bool noErr = true;
    Datum calibData;
    
    while (pairedEyeData->getAvailable(&eyeH, &eyeV, &eyeTimeUS)) {
    
        // put the paired values in the input vector for the calibration function
        pUncalibratedData->setElement(inputIndexH,(Datum)eyeH);
        pUncalibratedData->setElement(inputIndexV,(Datum)eyeV);
        
        
        for (int outputIndex=0;outputIndex<this->getNumOutputs();outputIndex++) {
            noErr = (fitableFunctions->getElement(outputIndex))->
						applyTheFunction(*pUncalibratedData, &calibData);
            if (noErr) {
                postResults(outputIndex, calibData, timeUS);               
                pCalibratedData->setElement(outputIndex,calibData);   // keep a copy of the last outs
            }
        }
    }
    
    unlock(); 
}




// method to announce details about each sample that is acquired for calibration  
void EyeCalibrator::announceCalibrationSample(int outputIndex, Datum SampledData, 
                    Datum DesiredOutputData, Datum CalibratedOutputData, MWTime timeOfSampleUS) {
    
    // this method expects the H sample to arrive first and then the V sample
    if (outputIndex == outputIndexH) {  // store data and wait for v (announce as pair)
        desiredH = DesiredOutputData;
        calibratedH = CalibratedOutputData;
        sampledH = (&SampledData)->getElement(inputIndexH);
        HsampleTime = timeOfSampleUS;
        return;
    }
    if (outputIndex == outputIndexV) {  // store data and wait for v (announce as pair)
        desiredV = DesiredOutputData;
        calibratedV = CalibratedOutputData;
        sampledV = (&SampledData)->getElement(inputIndexV);
        if ( (abs(timeOfSampleUS-HsampleTime)) > 10000) {
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Calibrator sample announce detected large time differential between h and v samples.  Values likely inaccurate.");
        }
    }
    
    
    Datum announceData(M_DICTIONARY, 5);
    announceData.addElement(CALIBRATOR_NAME,uniqueCalibratorName);    // char
    announceData.addElement(CALIBRATOR_ACTION,CALIBRATOR_ACTION_SAMPLE);
    
    Datum temp(M_LIST, 2);
    temp.setElement(0,sampledH);
    temp.setElement(1,sampledV);
    announceData.addElement(CALIBRATOR_SAMPLE_SAMPLED_HV,temp);    // input values
    
    temp.setElement(0,desiredH);
    temp.setElement(1,desiredV);
    announceData.addElement(CALIBRATOR_SAMPLE_DESIRED_HV,temp);    // gold standard values
    
    temp.setElement(0,calibratedH);
    temp.setElement(1,calibratedV);
    announceData.addElement(CALIBRATOR_SAMPLE_CALIBRATED_HV,temp); // values produced from input values using current calibration
    
    //announceData.addElement("JJDtest",desiredH); // values produced from input values using current calibration
    
    if (VERBOSE_EYE_CALIBRATORS) mprintf("mCalibrator::announceCalibrationSample  Announcing now");
    announce(announceData);    // announce things here using method from Announcable

}

    
void EyeCalibrator::announceCalibrationUpdate() {

    Datum announceData(M_DICTIONARY, 4);
    announceData.addElement(CALIBRATOR_NAME,uniqueCalibratorName);
    announceData.addElement(CALIBRATOR_ACTION,CALIBRATOR_ACTION_UPDATE_PARAMS);
    
    Datum paramsH = (fitableFunctions->getElement(HfunctionIndex))->getParameters();
    announceData.addElement(CALIBRATOR_PARAMS_H,paramsH);   // M_LIST
    
    Datum paramsV = (fitableFunctions->getElement(VfunctionIndex))->getParameters();
    announceData.addElement(CALIBRATOR_PARAMS_V,paramsV);   // M_LIST
    
    announce(announceData);    // announce things here using method from Announcable (will set values in announce variable)

}
  
void EyeCalibrator::setPrivateParameters() { 
  
    Datum privateData(M_DICTIONARY, 2);
    
    Datum paramsH = (fitableFunctions->getElement(HfunctionIndex))->getParameters();
    privateData.addElement(R_CALIBRATOR_PARAMS_H,paramsH);   // M_LIST
    
    Datum paramsV = (fitableFunctions->getElement(VfunctionIndex))->getParameters();
    privateData.addElement(R_CALIBRATOR_PARAMS_V,paramsV);   // M_LIST
    
    storePrivateData(privateData);   // base class method 
    
    // updated Feb 2007  -- DDC may suggest a better way, but works for now
    // Two potential problems that are delt with by the PrivateStorable base class:
    // 1) if we load saved data (in a private variable), then this causes us to come in on a notification
    //      in that case, we will NOT update the private variable (because the VARIABLE is locked and we don't need to update it!)
    //      (but, no matter what, we will keep track of the private variable values (in the PrivateStorable class) to deal with the following problem:
    // 2) if we change the value of the eye calibrator data by any other method, we will update the private variable
    //      and that will trigger a notification back to us, but we will IGNORE that notification (because the CALIBRATOR is locked and we don't need it!)
    //

}

// PUBLIC METHOD
// triggered by any change to the calibrator request variable  
void EyeCalibrator::notifyRequest(const Datum& dictionaryData, MWTime timeUS) {
    
	lock(); // DDC added
	
    // check if this calibrator should respond
	bool validRequest = checkRequest(dictionaryData);     // base class method (minimal checking)
    if (!validRequest){
		unlock(); // DDC added
		return;                          // not meant for this calibrator or improperly formatted
    }
	
    // determine the requested action

    CalibratorRequestedAction requestedAction = getRequestedAction(dictionaryData);  // overridable base class method
         
    switch (requestedAction) {
    
        case CALIBRATOR_NO_ACTION:
            mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                    "Request sent to calibrator %s resulted in no action.", uniqueCalibratorName.c_str());
                unlock(); // DDC added
				return;
            break;
    
        case CALIBRATOR_ACTION_SET_PARAMS_TO_DEFAULTS:
                //mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                //    "Request to update params sent to calibrator %s, BUT METHOD NOT YET WRITTEN. Request ignored.", uniqueCalibratorName);
                if (VERBOSE_EYE_CALIBRATORS) mprintf("An eye calibrator is processing a request to set its parameters to defaults."); 
                setParametersToDefaults(); 
                break;
            break;

        case CALIBRATOR_ACTION_SET_PARAMS_TO_CONTAINED:
            if (VERBOSE_EYE_CALIBRATORS) mprintf("An eye calibrator is processing a request to set its parameters to specific values."); 
            tryToUseDataToSetParameters(dictionaryData);      
            break;
        
    }
	
	unlock(); // DDC added
}

// PUBLIC METHOD -- this means the private variable is locked -- DO NOT UPDATE IT!
void EyeCalibrator::notifyPrivate(const Datum& dictionaryData, MWTime timeUS) {
    lock();
    tryToUseDataToSetParameters(dictionaryData); 
    unlock();
}



// this routine handles both "requests" and loading of private data (stored params)
// if a request, then priuvate values are probably in need of update
// if a load of private, then private values are OK, but I can check this.
void EyeCalibrator::tryToUseDataToSetParameters(Datum dictionaryData) {

    // check if this is a dictionary
    if (!(dictionaryData.getDataType() == M_DICTIONARY)) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Data processed by calibrator %s that was not expected dictionary type was ignored.", uniqueCalibratorName.c_str());
        return;
    }

    // try to perform the requested action
    bool paramsChanged = false; 
    Datum paramData;

    // if appropriate param fields are present and have expected length, then use the data 
    //     to try to update the parameters

    //  H params ================================================
    if (!(dictionaryData.hasKey(R_CALIBRATOR_PARAMS_H))) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Data processed to update params of calibrator %s without proper params filed was ignored.", uniqueCalibratorName.c_str());
        return;
    }
    paramData = dictionaryData.getElement(R_CALIBRATOR_PARAMS_H);

    // check if vector and correct length
    if  (paramData.getDataType() != M_LIST) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Data processed to update params of calibrator %s that did not contain vector in params field was ignored.", uniqueCalibratorName.c_str());
        return;
    }
    Datum paramsH = paramData;
    if (paramsH.getNElements() != (fitableFunctions->getElement(HfunctionIndex))->getNumParameters() ) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Data processed to update params of calibrator %s that did not contain expected number of params was ignored.", uniqueCalibratorName.c_str());
        return;
    }
    bool noErr = (fitableFunctions->getElement(HfunctionIndex))->setParameters(paramsH); 
    if (noErr) paramsChanged = true;     // params have been updated


    //  V params ================================================
    if (!(dictionaryData.hasKey(R_CALIBRATOR_PARAMS_V))) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Data processed to update params of calibrator %s without proper params filed was ignored.", uniqueCalibratorName.c_str());
        return;
    }
    paramData = dictionaryData.getElement(R_CALIBRATOR_PARAMS_V);

    // check if vector and correct length
    if  (paramData.getDataType() != M_LIST) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Data processed to update params of calibrator %s that did not contain vector in params field was ignored.", uniqueCalibratorName.c_str());
        return;
    }
    Datum paramsV = paramData;
    if (paramsV.getNElements() != (fitableFunctions->getElement(VfunctionIndex))->getNumParameters() ) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
            "Data processed to update params of calibrator %s that did not contain expected number of params was ignored.", uniqueCalibratorName.c_str());
        return;
    }
    noErr = (fitableFunctions->getElement(VfunctionIndex))->setParameters(paramsV); 
    if (noErr) paramsChanged = true;     // params have been updated

    // if any params were updated, announce the full set of current parameters
    // if this change was triggered by an external change to the private variable, then we do not need to update that private variable (and we cannot anyway --  it is locked)
    if (paramsChanged) reportParameterUpdate();

}

shared_ptr<mw::Component> EyeCalibratorFactory::createObject(std::map<std::string, std::string> parameters,
															 ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "tag", "eyeh_raw", "eyeh_calibrated", "eyev_raw", "eyev_calibrated");
	
	std::string tagname(parameters.find("tag")->second);
	shared_ptr<Variable> eyeh_raw = reg->getVariable(parameters.find("eyeh_raw")->second);	
	shared_ptr<Variable> eyev_raw = reg->getVariable(parameters.find("eyev_raw")->second);	
	shared_ptr<Variable> eyeh_calibrated = reg->getVariable(parameters.find("eyeh_calibrated")->second);	
	shared_ptr<Variable> eyev_calibrated = reg->getVariable(parameters.find("eyev_calibrated")->second);	
	
	checkAttribute(eyeh_raw, parameters["reference_id"], "eyeh_raw", parameters.find("eyeh_raw")->second);
	checkAttribute(eyev_raw, parameters["reference_id"], "eyev_raw", parameters.find("eyev_raw")->second);
	checkAttribute(eyeh_calibrated, parameters["reference_id"], "eyeh_calibrated", parameters.find("eyeh_calibrated")->second);
	checkAttribute(eyev_calibrated, parameters["reference_id"], "eyev_calibrated", parameters.find("eyev_calibrated")->second);
	
	shared_ptr <mw::Component> newEyeCalibrator = shared_ptr<mw::Component>(new EyeCalibrator(tagname, 
																							  eyeh_raw, 
																							  eyev_raw,
																							  eyeh_calibrated,
																							  eyev_calibrated,
																							  2));
	return newEyeCalibrator;
}

shared_ptr<mw::Component> LinearEyeCalibratorFactory::createObject(std::map<std::string, std::string> parameters,
															 ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "tag", "eyeh_raw", "eyeh_calibrated", "eyev_raw", "eyev_calibrated");
	
	std::string tagname(parameters.find("tag")->second);
	shared_ptr<Variable> eyeh_raw = reg->getVariable(parameters.find("eyeh_raw")->second);	
	shared_ptr<Variable> eyev_raw = reg->getVariable(parameters.find("eyev_raw")->second);	
	shared_ptr<Variable> eyeh_calibrated = reg->getVariable(parameters.find("eyeh_calibrated")->second);	
	shared_ptr<Variable> eyev_calibrated = reg->getVariable(parameters.find("eyev_calibrated")->second);	
	
	checkAttribute(eyeh_raw, parameters["reference_id"], "eyeh_raw", parameters.find("eyeh_raw")->second);
	checkAttribute(eyev_raw, parameters["reference_id"], "eyev_raw", parameters.find("eyev_raw")->second);
	checkAttribute(eyeh_calibrated, parameters["reference_id"], "eyeh_calibrated", parameters.find("eyeh_calibrated")->second);
	checkAttribute(eyev_calibrated, parameters["reference_id"], "eyev_calibrated", parameters.find("eyev_calibrated")->second);
	
	shared_ptr <mw::Component> newEyeCalibrator = shared_ptr<mw::Component>(new EyeCalibrator(tagname, 
																							  eyeh_raw, 
																							  eyev_raw,
																							  eyeh_calibrated,
																							  eyev_calibrated,
																							  1));
	return newEyeCalibrator;
}


END_NAMESPACE_MW
























