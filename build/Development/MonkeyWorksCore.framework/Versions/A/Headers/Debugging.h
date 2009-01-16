/*
 *  Debugging.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef	_DEBUGGING_H_
#define _DEBUGGING_H_

#include "VariableNotification.h";
namespace mw {
	extern bool debugger_enabled;
	
	extern void debuggerCheck();
	
	
	class DebuggerActiveNotification : public VariableNotification {
		
		virtual void notify(const Data& data, MonkeyWorksTime timeUS){
			
			debugger_enabled = (bool)data;
		}
		
	};
}

#endif
