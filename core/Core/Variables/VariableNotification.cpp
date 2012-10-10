/*
 *  VariableNotification.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 8/3/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "VariableNotification.h"


BEGIN_NAMESPACE_MW


/*******************************************************************
*                   ParameterNotification member functions
*******************************************************************/
VariableNotification::VariableNotification() {
	//param = _param;
}
VariableNotification::~VariableNotification() { 
	//remove(); // remove it from the notification chain 
}
// called when a new value is set
void VariableNotification::notify(const Datum& data, MWTime time){ } 


END_NAMESPACE_MW
