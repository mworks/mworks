/*
 *  FitableFunctions.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 9/2/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */


// JJD updated March 2006.  
// All Fitable functions are assumed to have a single output.
// Calibrators that want more than one output should take on more than one fitable function
//  (the outputs do not interact at the level of fitting).
//
//

#include "FitableFunctions.h"
using namespace mw;


BasisFunction::BasisFunction() {};
BasisFunction::~BasisFunction() {};
double BasisFunction::applyBasis(double *inputVector) {
    lock();
    unlock();
    return 0.0;     // should be overriden
};

// zero order
ZeroOrderBasisFunction::ZeroOrderBasisFunction() {};
ZeroOrderBasisFunction::~ZeroOrderBasisFunction() {};
double ZeroOrderBasisFunction::applyBasis(double *inputVector) {
    lock();
    double x = 1.0;
    unlock();
    return x;
}

// first order
FirstOrderBasisFunction::FirstOrderBasisFunction(int _useThisInput) {
    useThisInput = _useThisInput;
}
FirstOrderBasisFunction::~FirstOrderBasisFunction(){};
double FirstOrderBasisFunction::applyBasis(double *inputVector) {
    lock();
    double x = inputVector[useThisInput];
    unlock();
    return x;
}

// second order
SecondOrderBasisFunction::SecondOrderBasisFunction(int _useThisInput0, int _useThisInput1) {
    useThisInput = new int [2];
    useThisInput[0] = _useThisInput0;
    useThisInput[1] = _useThisInput1;
}
SecondOrderBasisFunction::~SecondOrderBasisFunction() {
    if(useThisInput) { delete useThisInput; useThisInput = NULL; }
}
double SecondOrderBasisFunction::applyBasis(double *inputVector) {
    lock();
    double x = (inputVector[useThisInput[0]]) * (inputVector[useThisInput[1]]);
    unlock();
    return x;
}



FitableSample::FitableSample(Data *_pDataInputVector, Data _desiredOutputData, MonkeyWorksTime _timeUS) {
    pDataInputVector = _pDataInputVector;
    desiredOutputData = _desiredOutputData;
    timeUS = _timeUS;
}

FitableSample::~FitableSample() {
    delete pDataInputVector;        // will clear this memory
}

Data *FitableSample::getInputVector() {
    return pDataInputVector;
}

Data FitableSample::getOutputData() {
    return desiredOutputData;
}

MonkeyWorksTime FitableSample::getTime() {
    return timeUS;
}



FitableFunction::FitableFunction(int _numInputs) {
    
    numInputs = _numInputs;
    Parameters = NULL;
    
    // list to keep all the data samples provided to the object to support the fit
    allSamplesToFit = new LinkedList<FitableSample>();
 
}

FitableFunction::~FitableFunction() {

    // release all the memory held by the samples
    // TODO -- how to check that htis is all deleted?
    delete allSamplesToFit;         // this is a linked list
    
}

void FitableFunction::acceptDataForFit(Data *pInputData, 
                            Data desiredOutputData, MonkeyWorksTime timeUS) {
      
    lock();      
    
    // package all the data up into an object (this is a data "sample") ========
    
    // put a copy of the data pointed at by the expandable list into an M_LIST
    Data data;
    Data *pDataInputVector = new Data(M_LIST, numInputs);  
    for (int i = 0;i<numInputs; i++ ) {
        Data ddd = pInputData->getElement(i);
        pDataInputVector->setElement(i,ddd); 
    }
    shared_ptr<FitableSample> sampleToFit(new FitableSample(pDataInputVector, 
                                                     desiredOutputData, timeUS));
    // =========================================================================
    
    // add to the linked list of availbleSamples
    allSamplesToFit->addToFront(sampleToFit);
    
    unlock();
}


void FitableFunction::flushOldData() {

    lock();  
    if (VERBOSE_FITABLE_FUNCTION) {
        mprintf("Fitable function: Clearing %d calibration samples", allSamplesToFit->getNElements());
    }

    
    delete allSamplesToFit; 
    allSamplesToFit = new LinkedList<FitableSample>();
    unlock();    
}

void FitableFunction::flushOldData(MonkeyWorksTime flushDataOlderThanThisTimeUS) {


    lock();    
    
//    int nStart = allSamplesToFit->getNElements();
    
    // TODO -- this code not yet working
    mprintf("Fitable function: stale clear not yet implemented.");
    
  /*  
    // remove samples that are old
    FitableSample *sampleToFit = (FitableSample *)(allSamplesToFit->getFrontmost());
    for (int n=0; n<nStart;n++) {
        long staleUS = (sampleToFit->getTime()) - flushDataOlderThanThisTimeUS;
        if (staleUS<0) sampleToFit->remove(); popElement
        mprintf("Fitable function: staleUS = %d", staleUS);
        sampleToFit = (FitableSample *)(sampleToFit->getNext());
    }
    
    int nEnd = allSamplesToFit->getNElements();
    if (VERBOSE_FITABLE_FUNCTION) {
        mprintf("Fitable function: Cleared %d stale calibration samples, now %d remaining",(nStart-nEnd),nEnd );
    } 

  */
    
    unlock();   
       
}

// TODO -- this does not seem right?
Data FitableFunction::getParameters() {
    
    lock();
    merror(M_SYSTEM_MESSAGE_DOMAIN, 
                        "Calling for params from fitable function base class is not appropriate.");

	Data undefined;
    unlock();
    return undefined;
}

int FitableFunction::getNumParameters() {

    lock();
    merror(M_SYSTEM_MESSAGE_DOMAIN, 
                        "Calling for number of params from fitable function base class is not appropriate.");
    unlock();
    return 0;
}


bool FitableFunction::setParameters(Data _params) {

    lock();
    bool success = setParametersProtected(_params);      // non-locking method
    unlock();
    return success;
}

bool FitableFunction::setParametersToDefaults() {

    lock();
    bool success = setParametersToDefaultsProtected();      // non-locking method
    unlock();
    return success;
}





LinearFitableFunction::LinearFitableFunction
        (int _numInputs) :  
        FitableFunction( _numInputs){
        
    basisSet = new ExpandableList<BasisFunction>(); 
        
}


LinearFitableFunction::~LinearFitableFunction() {
    basisSet->clear();      // delete the memory for the basis
    delete basisSet;
    if (Parameters != NULL) delete Parameters;
}   


bool LinearFitableFunction::setParametersProtected(Data _params) {

    Data *_parameters = &_params;
    
    if ( (_parameters->getMaxElements()) != (basisSet->getNElements())) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Linear calibrator error:  number of parameters not as expected.");
        unlock();
        return false;
    }
    
    if (Parameters != NULL) delete Parameters;
    Parameters = new float [basisSet->getNElements()];
    for (int p=0; p<basisSet->getNElements(); p++) {
        Parameters[p]= (float)(_parameters->getElement(p));
        if (VERBOSE_FITABLE_FUNCTION) {
            mprintf("Fitable function param: %d value: %f", p,Parameters[p]); 
        }    
    }
    
    return true;
}



Data LinearFitableFunction::getParameters() {

    Data _parameters(M_LIST, (basisSet->getNElements()));
    for (int p=0; p< (basisSet->getNElements()); p++) {
        (&_parameters)->addElement(Parameters[p]);
    }
    return _parameters;
    
}


int LinearFitableFunction::getNumParameters() {
    
    if (basisSet != NULL) {
        return (basisSet->getNElements());
    }
    else {
        return 0;
    }
}

        
bool LinearFitableFunction::fitTheFunction() {

    if (basisSet == NULL) return false;
    
    lock();
    
    int numParams = basisSet->getNElements();
    int numData = allSamplesToFit->getNElements();
    if (numData < numParams) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"WARNING: Attempted fit without enough data.");
        unlock();
        return false;
    }

    if (VERBOSE_FITABLE_FUNCTION) {
        mprintf("Fitable function: n data=%d n params=%d", numData, numParams);
    }

    float *B = new float [numParams];
    float *Y = new float [numData];
    
    // 2D array creation
    float **X = new2DfloatArray(numData,numParams);
    
    // create space to hold all Parameters
    if (Parameters != NULL) delete Parameters;
    Parameters = new float [numParams];

    // get data back in correct format
    double *temp = new double [numInputs];
    
    
    // unfold the data into the prescribed basis.
    // the format here is Y = Xb 
    
    // we start with the most recent data, but order does not matter
    shared_ptr<FitableSample> sampleToFit = allSamplesToFit->getFrontmost(); 
    
    for (int n=0; n<numData;n++) {
        
        Data *inputVector = sampleToFit->getInputVector();
        
        if (VERBOSE_FITABLE_FUNCTION>1) {
            MonkeyWorksTime timeUS = sampleToFit->getTime();        // testing only
            mprintf("Fitable function: datum %d timeUS = %d",n, (long)timeUS);
        }

        for (int i=0;i<numInputs;i++) {
            temp[i] = inputVector->getElement(i);
        }
        for (int p=0; p<numParams;p++) {
            X[n][p] = (basisSet->getElement(p))->applyBasis(temp);
        }
        
        Y[n] = (float)(sampleToFit->getOutputData());
        
        sampleToFit = sampleToFit->getNext();  // go to next sample on the list
    }


    // linear regression to find Parameters (SVD pseudo-inverse)
    // B = inv(XtX) XtY
    int tempNumParams;
    float chisq;
    SVDfit *svdfitter = new SVDfit(X,Y,NULL,numData,numParams);    // no weighting for now
    svdfitter->doFit(B, &tempNumParams, &chisq);
    if (tempNumParams != numParams) 
                merror(M_SYSTEM_MESSAGE_DOMAIN, 
                        "Unexpected number of parameters");
        
    
    // save for later application
    for (int p=0; p<numParams;p++) {
        Parameters[p] = B[p];
        if (VERBOSE_FITABLE_FUNCTION) {
            mprintf("Fitable function: final fit param %d = %f5", p, Parameters[p]);
        }
    }
    
    delete B;
	
	// DDC: fixed leak
	for(int i = 0; i < numData; i++){
		delete [] X[i];
	}
    delete [] X;  
    delete Y;
    
    delete temp;
    
    unlock();
    
    return true;
    
}

// compute the function value(s) with the current parameters
//bool LinearFitableFunction::applyTheFunction(Data *pInputData, Data *outputData){ // DDC fix
bool LinearFitableFunction::applyTheFunction(const Data& pInputData, Data *outputData){ 

    lock();
    
    if (Parameters == NULL) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"WARNING: Prompted for calibrated value but parameters not yet set.");
        unlock(); return false;
    }
    
    //if (pInputData->getNElements() != numInputs) { // DDC fix
	if (pInputData.getNElements() != numInputs) {
    
//        //int xxx0 = pInputData->getNElements(); // DDC fix
//		int xxx0 = pInputData.getNElements();
//        //double xxx1 = (double)(pInputData->getElement(0)); // DDC fix
//        //double xxx2 = (double)(pInputData->getElement(1));
//        double xxx1 = (double)(pInputData.getElement(0)); // DDC fix
//        double xxx2 = (double)(pInputData.getElement(1));

    
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"WARNING: Prompted for calibrated value but not all input data exists.");
        unlock(); return false;
    }
    
    
    double *inputDataDouble = new double [numInputs];
    for (int i=0;i<numInputs;i++) {
        
		//inputDataDouble[i] = (double)(pInputData->getElement(i));  // copy data here // DDC fix
        inputDataDouble[i] = (double)(pInputData.getElement(i));  // copy data here // DDC fix
		
		//xxx = inputDataDouble[i];   //TODO -- remove
    }
    
    double temp = 0;
    for (int p=0; p< (basisSet->getNElements()); p++) {
        temp = temp + ((Parameters[p])*((basisSet->getElement(p))->applyBasis(inputDataDouble)));
        //ppp = Parameters[p];    //TODO -- remove
    }
    *outputData = Data(temp);//DDC edit
	//*outputData = (Data)temp;
    
	delete [] inputDataDouble; // DDC added
	
    unlock(); 
    return true;
    
}


bool LinearFitableFunction::setParametersToDefaultsProtected() {
	
    if (VERBOSE_FITABLE_FUNCTION) {
        mprintf("Fitable Function: setting paremeters to defaults");
    }
    
    // move default parameters into a M_LIST
    if ((defaultParameters->getNElements()) != (basisSet->getNElements())) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "ERROR: Number of parameters does not match number of basis functions.");
        unlock();
        return false;    
    } 
    Data _parameters(M_LIST, defaultParameters->getNElements());
    for (int i = 0; i<basisSet->getNElements(); i++) {
        if (VERBOSE_FITABLE_FUNCTION) {
            double temp = *(defaultParameters->getElement(i));
            if (VERBOSE_FITABLE_FUNCTION) mprintf("basis: %d param value: %f",i,temp);
        }
        _parameters.setElement(i, *(defaultParameters->getElement(i)));
    }
    this->setParametersProtected(_parameters);
	
    return true;
}




// -- now you can easily create any (decoupled) linear mapping you want ...
// all we need to do is establish the basis functions.
// the base classes handle the rest   
// in this example, we use a second order polynomial and we set default params
FirstOrderPolynomialFitableFunction::FirstOrderPolynomialFitableFunction
(int _numInputs, bool _includeAnOffsetParameter, int _defaultInput)  
: LinearFitableFunction ( _numInputs) {
    
	if (VERBOSE_FITABLE_FUNCTION) mprintf("Instantiating a FIRST order polynomial fitable function");
    /*
	 // testing -----
	 mExpandableList<double> *testParameters = new mExpandableList<double>();
	 testParameters->addElement((double)1);
	 testParameters->addElement((double)2);
	 testParameters->addElement((double)3);
	 for (int i = 0; i<testParameters->getNElements(); i++) {
	 mprintf("test: %d param value: %f",i,testParameters->getElement(i));
	 }
	 //  end testing
	 */
    
    // build the basis functions and defult params
    defaultParameters = new ExpandableList<double>();
    shared_ptr<BasisFunction> basis;
    if (_includeAnOffsetParameter) {
        basis = shared_ptr<BasisFunction>(new ZeroOrderBasisFunction());
        basisSet->addReference(basis);
        defaultParameters->addElement((double)0);
    }
    for (int i = 0; i<numInputs; i++) {
        basis = shared_ptr<BasisFunction>(new FirstOrderBasisFunction(i));
        basisSet->addReference(basis);
        if (i == _defaultInput) {
            defaultParameters->addElement((double)1);
        } else {
            defaultParameters->addElement((double)0);
        }
    }
	
    // now set the params    
    setParametersToDefaultsProtected();
    
}


// in this example, we use a second order polynomial and we set default params
SecondOrderPolynomialFitableFunction::SecondOrderPolynomialFitableFunction
    (int _numInputs, bool _includeAnOffsetParameter, int _defaultInput)  
    : LinearFitableFunction ( _numInputs) {
    
    /*
    // testing -----
    ExpandableList<double> *testParameters = new ExpandableList<double>();
    testParameters->addElement((double)1);
    testParameters->addElement((double)2);
    testParameters->addElement((double)3);
    for (int i = 0; i<testParameters->getNElements(); i++) {
        mprintf("test: %d param value: %f",i,testParameters->getElement(i));
    }
    //  end testing
    */
    
    // build the basis functions and defult params
    defaultParameters = new ExpandableList<double>();
    shared_ptr<BasisFunction> basis;
    if (_includeAnOffsetParameter) {
        basis = shared_ptr<BasisFunction>(new ZeroOrderBasisFunction());
        basisSet->addReference(basis);
        defaultParameters->addElement((double)0);
    }
    for (int i = 0; i<numInputs; i++) {
        basis = shared_ptr<BasisFunction>(new FirstOrderBasisFunction(i));
        basisSet->addReference(basis);
        if (i == _defaultInput) {
            defaultParameters->addElement((double)1);
        } else {
            defaultParameters->addElement((double)0);
        }
    }
    for (int i = 0; i<numInputs; i++) {
        for (int j = i; j<numInputs; j++) {
            basis = shared_ptr<BasisFunction>(new SecondOrderBasisFunction(i,j));
            basisSet->addReference(basis);
            defaultParameters->addElement((double)0);
        }
    }

    // now set the params    
    setParametersToDefaultsProtected();
    
}



