/*
 *  FitableFunctions.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 9/2/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 * JJD created these for eye calibration
 * They are supposed to be a base class for any function that can be fit.
 * and a few special cases that we implement for eye calibration
 */

#ifndef _FITABLE_FUNCTION
#define _FITABLE_FUNCTION

#include "Lockable.h"
#include "GenericData.h"
#include "ExpandableList.h"
#include "LinkedList.h"
#include "Experiment.h"
#include "SVDfit.h"

#define VERBOSE_FITABLE_FUNCTION 1
namespace mw {
	
	class BasisFunction: public Lockable {
    public:
        BasisFunction();
        virtual ~BasisFunction();
        virtual double applyBasis(double *inputVector);
	};        
	
	class ZeroOrderBasisFunction : public BasisFunction {
    public:
        ZeroOrderBasisFunction();
        virtual ~ZeroOrderBasisFunction();
        virtual double applyBasis(double *inputVector);
	};
	
	
	class FirstOrderBasisFunction : public BasisFunction {
    protected:
        int useThisInput;
    public:
        FirstOrderBasisFunction(int _useThisInput);
        virtual ~FirstOrderBasisFunction();
        virtual double applyBasis(double *inputVector); 
	};
	
	class SecondOrderBasisFunction : public BasisFunction {
    protected:
        int *useThisInput;
    public:
        SecondOrderBasisFunction(int _useThisInput0, int _useThisInput1);
        virtual ~SecondOrderBasisFunction();
        virtual double applyBasis(double *inputVector);
	};
	
	
	class ThirdOrderBasisFunction : public BasisFunction {
    protected:
        int *useThisInput;
    public:
        ThirdOrderBasisFunction(int _useThisInput0, int _useThisInput1, int _useThisInput2) {
            useThisInput = new int [3];
            useThisInput[0] = _useThisInput0;
            useThisInput[1] = _useThisInput1;
            useThisInput[2] = _useThisInput2;
        }
        virtual ~ThirdOrderBasisFunction(){
            if(useThisInput) { delete useThisInput; useThisInput = NULL; }
        };
        virtual double applyBasis(double *inputVector) {
            double x = (inputVector[useThisInput[0]]) * (inputVector[useThisInput[1]]) *
			(inputVector[useThisInput[2]]);
            return x;
        }
	};
	
	
	class FitableSample : public LinkedListNode<FitableSample> {
		
    protected:
        Data           *pDataInputVector;
        Data           desiredOutputData;
        MonkeyWorksTime timeUS;
        
    public:
        FitableSample(Data *_pDataInputVector, Data _desiredOutputData, MonkeyWorksTime _timeUS);
        virtual ~FitableSample();
        Data           *getInputVector();
        Data           getOutputData();
        MonkeyWorksTime getTime();
	};
	
	
	
	// the base class handles the storage and flushing of data used for the fit
	class FitableFunction : public Lockable {
        
    protected:
        int         numInputs;    
        float       *Parameters;        // pointer to a parameter set of some type (e.g. just an real vector for linear basis)
        LinkedList<FitableSample> *allSamplesToFit;   // all the data "samples" (input, output, and time in each)
        virtual bool setParametersProtected(Data _parameters) {return false;}
        virtual bool setParametersToDefaultsProtected() {return false;} 
        
    public:
        FitableFunction(int _numInputs);
        virtual ~FitableFunction();
        virtual void acceptDataForFit(Data *pInputData, Data desiredOutputData, MonkeyWorksTime timeUS);
        virtual bool fitTheFunction() {return false;}
        //virtual bool applyTheFunction(Data *pInputData, Data *outputData) {return false;} // DDC fix
		virtual bool applyTheFunction(const Data& pInputData, Data *outputData) {return false;}
        virtual bool setParametersToDefaults();
        virtual bool setParameters(Data _parameters); 
        virtual Data getParameters(); 
        virtual int getNumParameters();
        virtual void flushOldData(); // flush all
        virtual void flushOldData(MonkeyWorksTime _flushDataOlderThanThisTimeUS); 
		
	};
	
	
	// this function should be able to fit anything that is of the form (y = Xb)
	// if there is more than one output, your calibrator will take on more than one fitable function.
	// (one fitable function for each output).  It is fine if they use the same input data.
	
	class LinearFitableFunction : public FitableFunction {
		
    protected:
        ExpandableList<BasisFunction> *basisSet;
		ExpandableList<double> *defaultParameters;
        virtual bool setParametersProtected(Data _parameters);
		virtual bool setParametersToDefaultsProtected();
		
		
    public:
        LinearFitableFunction(int _numInputs);
        ~LinearFitableFunction();
        virtual bool fitTheFunction();
        //virtual bool applyTheFunction(Data *pInputData, Data *outputData); // DDC fix
		virtual bool applyTheFunction(const Data& pInputData, Data *outputData);
        virtual Data getParameters();
        virtual int getNumParameters();
        
	};
	
	
	class SecondOrderPolynomialFitableFunction : public LinearFitableFunction {
    public:
        SecondOrderPolynomialFitableFunction(int _numInputs, 
											 bool _includeAnOffsetParameter, int _defaultInput); 
	};
	
	class FirstOrderPolynomialFitableFunction : public LinearFitableFunction {
    public:
        FirstOrderPolynomialFitableFunction(int _numInputs, 
											bool _includeAnOffsetParameter, int _defaultInput); 
	};
}


#endif
