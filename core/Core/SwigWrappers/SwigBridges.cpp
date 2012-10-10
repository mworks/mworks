/*
 *  SwigBridges.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on 10/11/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "SwigBridges.h"


BEGIN_NAMESPACE_MW


VariableProperties *createNewVariableProperties(long defaultval, char *_tagname, char *_shortname, 
							char *_longname, int _editable, bool _viewable, int _domain, int _logging, char *groups){
							
	 Datum data = Datum(defaultval);
//		return new VariableProperties(data, _tagname, _shortname, 
//							_longname, (WhenType)_editable, _viewable, (DomainType)_domain, (WhenType) _logging);
    return NULL;
}
	
	
Datum createNewDataObject(long data){
 Datum thedata = Datum(data);
	return thedata;
}

Datum createNewDataObject(double data){
 Datum thedata = Datum(data);
	return thedata;
}


END_NAMESPACE_MW
