/**
 * ScopedVariableContext.h
 *
 * Description: 
 * An ScopedVariableContext object is a context for variables.  Within the larger MWorks 
 * framework, variables are made up of two parts, a lexical element (analogous to the
 * name of a C variable, e.g. "i"), and an actual value (e.g. "4").  A given lexical
 * token can have the value that it is associated with change (e.g. "i = 4; i = 5"),
 * without changing the lexical token itself (i.e. "i").  One additional wrinkle is that
 * a given token (e.g. "i") can mean different things depending on context.  For instance,
 * it we call a function, a local declaration of "i" will shadow any global definition of
 * a variable of the same name.  Such shadowing is particularly handy when defining an
 * experiment, since it is often natural to define a variable for all branches of a
 * hierarchy below a certain point.  Thus, it might be convenient to defined a default
 * value of "fixation_location" to be [0,0] for the experiment as a whole, and then 
 * just override it with different values only for those trials (and their sub-states) 
 * that need it to be something different (e.g. for calibration trials).  When those 
 * trials are done, we'd like fixation_location to return to whatever it used to be, just
 * as the meaning of "i" returns to it's global meaning after you return from a function.
 * If you don't see the utility of this, just trust me for now.  Example experiments will
 * demonstrate why this is good.
 *
 * ScopedVariableContexts  either define what a variables
 * value is within the present context (each state has it's own ScopedVariableContext object), or they
 * point back through the hierarchy to wherever the value was last defined.
 *
 * This object used to be called a "mTrialInfo," so references to "TrialInfos" may occur
 * throughout the comments in this project
 *
 * History:
 * ??/??/?? ??? - Created.
 * 04/14/04 PAJ - added accessor methods to return list pointers for simple 
 *                list traversal used in Experiment Browser window.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 */	
 
#ifndef _VARIABLE_CONTEXT_HEADER
#define _VARIABLE_CONTEXT_HEADER

#include "ScopedVariableEnvironment.h"


BEGIN_NAMESPACE_MW


enum Transparency { M_TRANSPARENT, M_OPAQUE };


class ScopedVariableContext {
    
private:
    std::map<int, Datum> data;  // a list of data
    std::map<int, Transparency> transparency;  // a list of flags whether the object
                                               // is actually defined here or is inherited
    
public:
    // Contexts should be created *after* all variables are added to the environment
    explicit ScopedVariableContext(const boost::shared_ptr<ScopedVariableEnvironment> &environment);
    
    void inheritFrom(const boost::shared_ptr<ScopedVariableContext> &parent);  // inherit from a parent, unless already
                                                                               // defined opaque
    
    int getNFields() const { return data.size(); };
    
    Transparency getTransparency(const boost::shared_ptr<ScopedVariable> &var) const;
    Transparency getTransparency(int i) const;
    
    Datum get(int index) const;
    
    void set(int index, const Datum &newdata);
    void setWithTransparency(int index, const Datum &newdata);
    
};


END_NAMESPACE_MW


#endif
