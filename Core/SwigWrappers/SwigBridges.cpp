/*
 *  SwigBridges.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on 10/11/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "SwigBridges.h"
using namespace mw;




VariableProperties *createNewVariableProperties(long defaultval, char *_tagname, char *_shortname, 
							char *_longname, int _editable, bool _viewable, int _domain, int _logging, char *groups){
							
		Data data = Data(defaultval);
//		return new VariableProperties(data, _tagname, _shortname, 
//							_longname, (WhenType)_editable, _viewable, (DomainType)_domain, (WhenType) _logging);
    return NULL;
}
	
	
Data createNewDataObject(long data){
	Data thedata = Data(data);
	return thedata;
}

Data createNewDataObject(double data){
	Data thedata = Data(data);
	return thedata;
}
