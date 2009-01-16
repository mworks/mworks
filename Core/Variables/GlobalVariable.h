/*
 *  GlobalVariable.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 *
 *	The constant variable is a subclass of the Variable object
 *  that carries it's own Data internal to itself.  This is in contrast
 *  to "normal" Variable objects which are merely symbolic tokens whose
 *  meaning may depend upon what context they reside in.
 *
 *  One function of the constant variable is to allow for truely "constant" 
 *  values (like "4") to be represented as Variables so that other objects in
 *  the system can flexibly use either constant or variable quantities by
 *  refering to the same objects.
 *
 *  The "constant"-ness of constant variables is not absolute, however, and
 *  they can be changed as any other variable would be.  Such changes
 *  actually change the internal Data of the object, where a "regular" 
 *  variable does not change when the value it symbolically references is 
 *  changed.  Thus, the GlobalVariable primarily provides a different
 *  (and variously useful) means of interacting with the larger variable
 *  handling system.
 */

#ifndef	GLOBAL_VARIABLE_H_
#define GLOBAL_VARIABLE_H_

#include "GenericVariable.h"
#include "boost/shared_ptr.hpp"
namespace mw {
class GlobalVariable : public Variable{
    protected:
		shared_ptr<Data> value;
		shared_ptr<Lockable> valueLock;

	public:
        /**
         * Sets the data value to '_value'
         */
        GlobalVariable(Data _value, VariableProperties *interface=NULL);
		        
		/**
         * Constructs a value object from an interface setting object
         */
        GlobalVariable(VariableProperties *_interface);
        
		
        /**
         * Constructs a value object from a datum
         */
        //mGlobalVariable(ScarabDatum *package);
        
		
		/**
		 *  A polymorphic copy constructor
		 */
		virtual Variable *clone();
		
        virtual ~GlobalVariable();
		
        /**
         * Returns the value data or returns a new data object initialized
         * to zero if the old data object type was M_UNDEFINED.
         */
        virtual Data getValue();
        
		
        /**
         * Sets the data value of this parameter
         */
		void setValue(Data newval);
		void setValue(Data newval, MonkeyWorksTime time);
		void setSilentValue(Data newval);
		void setSilentValue(Data newval, MonkeyWorksTime time);
        
        /**
         * Returns true, always
         */
		bool isValue();
        
        /**
         * Returns a dictionary package of this object used to send it
         * over a network connection
         */
        //ScarabDatum *toScarabPackage();
        
        /**
         * Prints this objects data to the stderr stream.
         */
        virtual void printToSTDERR();
};
}
#endif

