/*
 *  OpenALContextManager.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "OpenALContextManager.h"
using namespace mw;

namespace mw {
	OpenALContextManager *GlobalOpenALContextManager;
}

OpenALContextManager::OpenALContextManager(){
	
	
	
	devices = new ExpandableList<ALDevice>();
	default_device = alcOpenDevice(NULL);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to open OpenAL sound device (error code %d)",
			   error);
		return; 
	}
	devices->addElement(default_device);
	
	contexts = new ExpandableList<ALContext>();
	default_context = alcCreateContext(default_device, NULL);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to generate OpenAL context for sound stimulus (error code %d)",
			   error);
		return; 
	}
	contexts->addElement(default_context);
	
	setDefaultContextCurrent();
}

OpenALContextManager::~OpenALContextManager(){
	// tear down
	for(int i = 0; i < contexts->getNElements(); i++){
		alcDestroyContext(((*contexts)[i])->getContext());
		if ((error = alGetError()) != AL_NO_ERROR) { 
			merror(M_SYSTEM_MESSAGE_DOMAIN,
				   "Failed to destroy OpenAL context (error code %d)",
				   error);
			return; 
		}
	}
	
	delete contexts;
	
	for(int i = 0; i < devices->getNElements(); i++){
		alcCloseDevice(((*devices)[i])->getDevice());
		if ((error = alGetError()) != AL_NO_ERROR) { 
			merror(M_SYSTEM_MESSAGE_DOMAIN,
				   "Failed to close OpenAL sound device (error code %d)",
				   error);
			return; 
		}
	}
	
	delete devices;
}

ALCcontext *OpenALContextManager::getDefaultContext(){
	return default_context;
}

void OpenALContextManager::setCurrent(int i){
	
	ALCcontext *context = (contexts->getElement(i))->getContext();
	if(contexts->getNElements() > i && context != NULL){
		
		alcMakeContextCurrent(context);
		if ((error = alGetError()) != AL_NO_ERROR) { 
			merror(M_SYSTEM_MESSAGE_DOMAIN,
				   "Failed to set indexed OpenAL context current (error code %d)",
				   error);
			return; 
		}
	}
}

void OpenALContextManager::setDefaultContextCurrent(){
	alcMakeContextCurrent(default_context);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to set default OpenAL context current (error code %d)",
			   error);
		return; 
	}
}
