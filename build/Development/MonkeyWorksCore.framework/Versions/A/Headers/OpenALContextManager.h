#ifndef OPENAL_CONTEXT_MANAGER_H_
#define OPENAL_CONTEXT_MANAGER_H_

/*
 *  OpenALContextManager.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

// shifting sands
//#include <OpenAL/alut.h>
#include "alut.h"

#include "ExpandableList.h"
#include "Utilities.h"

namespace mw {
	class ALContext{
		
	public:
		ALCcontext *context;
		
		ALContext(ALCcontext *_context){
			context = _context;
		}
		
		operator ALCcontext *(){
			return context;
		}
		
		void operator= (ALCcontext *_context){
			context = _context;
		}
		
		ALCcontext *getContext(){ return context; }
	};
	
	class ALDevice{
		
	public:
		
		ALCdevice *device;
		
		ALDevice(ALCdevice *_device){
			device = _device;
		}
		
		operator ALCdevice *(){
			return device;
		}
		
		void operator= (ALCdevice *_device){
			device = _device;
		}
		
		ALCdevice *getDevice(){ return device; }
	};
	
	
	class OpenALContextManager {
		
	protected:
		
		ALCdevice *default_device;
		ExpandableList<ALDevice> *devices;
		
		ALCcontext *default_context;
		ExpandableList<ALContext> *contexts;
		
		ALuint error;
		
	public:
		
		OpenALContextManager();
		~OpenALContextManager();
		
		ALCcontext *getDefaultContext();
		void setCurrent(int i);
		void setDefaultContextCurrent();
		
	};
	
	extern OpenALContextManager *GlobalOpenALContextManager;
}
#endif
