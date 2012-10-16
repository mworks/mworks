#ifndef OPENAL_CONTEXT_MANAGER_H_
#define OPENAL_CONTEXT_MANAGER_H_

/*
 *  OpenALContextManager.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
//#include "alc.h" //customized header for gcc4.2

// shifting sands
//#include <OpenAL/alut.h>
#include "alut.h"

#include "ExpandableList.h"
#include "Utilities.h"

#include "RegisteredSingleton.h"

namespace mw {
	//class ALContext{
//		
//	public:
//		ALCcontext *context;
//		
//		ALContext(ALCcontext *_context){
//			context = _context;
//		}
//		
//		operator ALCcontext *(){
//			return context;
//		}
//		
//		void operator= (ALCcontext *_context){
//			context = _context;
//		}
//		
//		ALCcontext *getContext(){ return context; }
//	};
//	
//	class ALDevice{
//		
//	public:
//		
//		ALCdevice *device;
//		
//		ALDevice(ALCdevice *_device){
//			device = _device;
//		}
//		
//		operator ALCdevice *(){
//			return device;
//		}
//		
//		void operator= (ALCdevice *_device){
//			device = _device;
//		}
//		
//		ALCdevice *getDevice(){ return device; }
//	};
//	
	
	class OpenALContextManager : public Component {
		
	protected:
		
        // not using shared_ptr here because these are C ptrs
		ALCdevice* default_device;
		vector<ALCdevice*> devices;
		
		ALCcontext* default_context;
		vector<ALCcontext*> contexts;
		
		ALuint error;
		
	public:
		
		OpenALContextManager();
		~OpenALContextManager();
		
		ALCcontext* getDefaultContext();
		void setCurrent(int i);
		void setDefaultContextCurrent();
	
        REGISTERED_SINGLETON_CODE_INJECTION(OpenALContextManager)
	};
	
}
#endif
