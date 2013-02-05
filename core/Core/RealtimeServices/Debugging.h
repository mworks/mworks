/*
 *  Debugging.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef	_DEBUGGING_H_
#define _DEBUGGING_H_

#include "VariableNotification.h"


BEGIN_NAMESPACE_MW


	extern bool debugger_enabled;
	
	extern void debuggerCheck();
	
	
	class DebuggerActiveNotification : public VariableNotification {
		
		virtual void notify(const Datum& data, MWTime timeUS){
			
			debugger_enabled = (bool)data;
		}
		
	};


END_NAMESPACE_MW


#endif
