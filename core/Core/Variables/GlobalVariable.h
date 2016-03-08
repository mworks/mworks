/*
 *  GlobalVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 *
 *	The constant variable is a subclass of the Variable object
 *  that carries it's own Datum internal to itself.  This is in contrast
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
 *  actually change the internal Datum of the object, where a "regular" 
 *  variable does not change when the value it symbolically references is 
 *  changed.  Thus, the GlobalVariable primarily provides a different
 *  (and variously useful) means of interacting with the larger variable
 *  handling system.
 */

#ifndef	GLOBAL_VARIABLE_H_
#define GLOBAL_VARIABLE_H_

#include <mutex>

#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


class GlobalVariable : public Variable {
    
public:
    /**
     * Sets the data value to '_value'
     */
    explicit GlobalVariable(Datum _value, VariableProperties *interface = nullptr);
    
    /**
     * Constructs a value object from an interface setting object
     */
    explicit GlobalVariable(VariableProperties *_interface);
    
    /**
     * Returns the value data or returns a new data object initialized
     * to zero if the old data object type was M_UNDEFINED.
     */
    Datum getValue() override;
    
    /**
     * Sets the data value of this parameter
     */
    void setSilentValue(Datum newval, MWTime time) override;
    
    /**
     * The value can be modified
     */
    bool isWritable() const override { return true; }
    
private:
    Datum value;
    std::mutex valueMutex;
    using lock_guard = std::lock_guard<decltype(valueMutex)>;
    
};


END_NAMESPACE_MW


#endif


























