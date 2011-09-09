#ifndef	STANDARD_SOUNDS_H_
#define STANDARD_SOUNDS_H_

/*
 *  StandardSounds.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sound.h"
#include "OpenALContextManager.h"
#include <boost/filesystem/path.hpp>
#include "ComponentInfo.h"


BEGIN_NAMESPACE_MW


class OpenALSound : public Sound {

protected:
	ALuint	buffer;
	ALuint  source;
	ALuint error;
	
	shared_ptr<Variable> amplitude;
	
public:
    static const std::string AMPLITUDE;
    
    static void describeComponent(ComponentInfo &info);
    
    OpenALSound(const ParameterValueMap &parameters);

	virtual void play();	
	virtual void pause();	
	virtual void stop();

	virtual void announceSoundPlayed();
	
};


class WavFileSound : public OpenALSound {

protected:

	ALenum format;
	ALvoid *data;
	ALsizei size;
	ALsizei freq;
	string path;

public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    WavFileSound(const ParameterValueMap &parameters);

	~WavFileSound();
	
	
	virtual void announceSoundPlayed();
};


END_NAMESPACE_MW


#endif





















