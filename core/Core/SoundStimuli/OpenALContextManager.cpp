/*
 *  OpenALContextManager.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "OpenALContextManager.h"


BEGIN_NAMESPACE_MW


SINGLETON_INSTANCE_STATIC_DECLARATION(OpenALContextManager);


OpenALContextManager::OpenALContextManager() :
                                default_device(NULL),
                                default_context(NULL){
	
	try {
        
        default_device = alcOpenDevice(NULL);
        if ((error = alGetError()) != AL_NO_ERROR) { 
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Failed to open OpenAL sound device (error code %d)",
                   error);
            return; 
        }
        devices.push_back(default_device);
        
        default_context = alcCreateContext(default_device, NULL);
        if ((error = alGetError()) != AL_NO_ERROR) { 
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Failed to generate OpenAL context for sound stimulus (error code %d)",
                   error);
            return; 
        }
        contexts.push_back(default_context);
        
        setDefaultContextCurrent();
    } catch (std::exception& e){
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Failed to initialize the audio engine.  Sounds may not play correctly");
        return;
    }
}

OpenALContextManager::~OpenALContextManager(){
	// tear down
    vector<ALCcontext*>::iterator i;
	for(i = contexts.begin(); i != contexts.end(); i++){
		
        if( *i != NULL ){
            try{
                alcDestroyContext(*i);
            } catch (std::exception& e) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Problem destroying audio context");
            }
            if ((error = alGetError()) != AL_NO_ERROR) { 
                merror(M_SYSTEM_MESSAGE_DOMAIN,
				   "Failed to destroy OpenAL context (error code %d)",
				   error);
            } 
		}
	}
	
    vector<ALCdevice*>::iterator d;
	for(d = devices.begin(); d != devices.end(); d++){
		
        if( *d != NULL ){
            
            try{
                alcCloseDevice(*d);
            } catch  (std::exception& e) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Problem closing audio device");
            }
            if ((error = alGetError()) != AL_NO_ERROR) { 
                merror(M_SYSTEM_MESSAGE_DOMAIN,
				   "Failed to close OpenAL sound device (error code %d)",
				   error);
			} 
		}
	}
	
}

ALCcontext* OpenALContextManager::getDefaultContext(){
	return default_context;
}

void OpenALContextManager::setCurrent(int i){
	
    try {
        ALCcontext *context = contexts[i];
    
		alcMakeContextCurrent(context);
		if ((error = alGetError()) != AL_NO_ERROR) { 
			merror(M_SYSTEM_MESSAGE_DOMAIN,
				   "Failed to set indexed OpenAL context current (error code %d)",
				   error);
			return; 
		}
	} catch (std::exception& e){
    
        throw SimpleException("Attempt to set an invalid OpenAL (audio) context.  This is probably a bug.");
    }
}

void OpenALContextManager::setDefaultContextCurrent(){
	try{
        alcMakeContextCurrent(default_context);
	} catch  (std::exception& e){
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Problem switching audio contexts");
    }
    if ((error = alGetError()) != AL_NO_ERROR) { 
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to set default OpenAL context current (error code %d)",
			   error);
		return; 
	}
}


END_NAMESPACE_MW
