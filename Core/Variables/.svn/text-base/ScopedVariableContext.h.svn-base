#ifndef _VARIABLE_CONTEXT_HEADER
#define _VARIABLE_CONTEXT_HEADER

#include "GenericData.h"
#include "GenericVariable.h"
#include "ScopedVariable.h"
#include "ScopedVariableEnvironment.h"
#include "Selection.h"
//#include "ExpandableList.h"
#include <vector>

/**
 * ScopedVariableContext.h
 *
 * Description: 
 * An ScopedVariableContext object is a context for variables.  Within the larger MonkeyWorks 
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
 
#include <map>
namespace mw { 
enum  Transparency{ M_TRANSPARENT, M_OPAQUE };

// The base trial info object, containing user defined variables which are needed to run trials

class ScopedVariableContext : public Lockable{

	protected:

		ScopedVariableEnvironment *environment;
		std::map< int, shared_ptr<Data> > data; // a list of pointers to data
		std::map<int, Transparency> transparency; // a list of flags whether the object
                                                              // is actually defined here or is inherited
                                                              
	public:
	
		ScopedVariableContext(ScopedVariableEnvironment *env);
		ScopedVariableContext(shared_ptr<ScopedVariableContext> ownersinfo);  // point through to a parent

		void inheritFrom(shared_ptr<ScopedVariableContext> info_to_inherit);  // inherit from a parent, unless already 
                                                                // defined opaque
		virtual ~ScopedVariableContext();
        
		virtual Transparency getTransparency(ScopedVariable *param);
		virtual Transparency getTransparency(int i);
                                		
		int getNFields(); // how many fields?
		
		ScopedVariableEnvironment *getEnvironment(){ return environment; }
		
		
		Data get(int index);
                
		void set(int index, shared_ptr<Data> newdata);
		void set(int index, const Data& newdata);
		void setWithTransparency(int index, shared_ptr<Data> newdata);
		void setWithTransparency(int index, const Data& newdata);
                
};
}


#endif

