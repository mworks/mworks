/*
 *  VariableTransformAdaptors.h
 *  MonkeyWorksCore
 *
 *  Created by James DiCarlo on 8/8/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 * description:  This class is meant to encompass things that
 * take some number of variables as input and produce updates
 * to some number of variables as output.
 *
 * It relies on the core mecahnisms for dealing with variables.
 * Particularly, the idea of notifications (to activate the transform
 * when any one of the input variables is updated) and the idea 
 * of variables that automatically update when their Data is changed.
 *
 * The base class will support registering of input and output variables
 * and the activation/deactivation of the transform.
 *
 * We decided to establish a separate calss for actually doing transforms
 * (e.g. memory, filters, etc.)
 *
 */

#ifndef VAR_TRANSFORM_ADAPTORS
#define VAR_TRANSFORM_ADAPTORS

#include "Transforms.h"
#include "GenericVariable.h"
#include "ExpandableList.h"
#include "Experiment.h"

#define VERBOSE_VAR_TRANSFORM_ADAPTOR 0
namespace mw {
// warn compiler that I will declare later.. 
class VarTransformNotification;

//class VarTransformAdaptor {
class VarTransformAdaptor : public Lockable, public mw::Component {

	private:		// only base class can access this
	
		bool active;
		int nInputs;
		int nOutputs;
		ExpandableList<Variable> inputVars;
		ExpandableList<Variable> outputVars;
		ExpandableList<VarTransformNotification> inputNotifications;
		
	protected:		// things called by derived classes
		
        // derived classes should use these methods to 
		// first register things that they want to use as inputs and outputs
		// (should be in constructor of the derived class)
		int registerInput(shared_ptr<Variable> input);
		int registerOutput(shared_ptr<Variable> output);
				
		// outputs:
        // (methods to modify data values in one or more of the registered variables)
		//  the derived class should use one of these calls if it wants to post output variables
		//bool postResults(Data dataVector[]);		
        bool postResults(const Data& dataVector, MonkeyWorksTime timeUS);	
        bool postResults(int outputIndex, const Data& data);
        bool postResults(int outputIndex, const Data& data, MonkeyWorksTime timeUS);
        
        int  getNumInputs() {return nInputs;};
        int  getNumOutputs() {return nOutputs;};
        bool getInputVariable(int inputIndex, shared_ptr<Variable> *inputVariable);    // return pointer to input variable
        
	public:		// things that might be called by the core

		VarTransformAdaptor();
		virtual ~VarTransformAdaptor();
		
		// DDC: This method (and construct) is currently never used.
		//      For now, I'm just going to declare that to the extent is is
		//		ever used, it will not be thread safe.  Woe to he or she
		//		that tries to set it from more than one thread!
		void setActive(bool _active) {
            //lock();
			active = _active;
            //unlock();
		}
		bool getActive() {
			return(active);
		}
		
		// called by the notify method when input changes -- should be overriden
		// the inputIndex will tell the object which of its input changed, 
        //   and the data argument has the new value
		// this is the thing that the derived classes should override
		virtual void newDataReceived(int inputIndex, const Data& data, 
                                                MonkeyWorksTime timeUS);
        virtual void reset() {};  // method to allow reseting of object
        
};


class VarTransformNotification : public VariableNotification{

	protected:

		VarTransformAdaptor *varTransformAdaptor;
		int	inputIndex;
		
	public:

		VarTransformNotification(VarTransformAdaptor *_varTransformAdaptor, 
                                                                int inputIndex);
		~VarTransformNotification();		
		virtual void notify(const Data& data, MonkeyWorksTime timeUS);

};
}

#endif

