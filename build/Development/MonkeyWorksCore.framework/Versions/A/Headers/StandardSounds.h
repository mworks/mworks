#ifndef	STANDARD_SOUNDS_H_
#define STANDARD_SOUNDS_H_

/*
 *  StandardSounds.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sound.h"
#include "OpenALContextManager.h"
#include <boost/filesystem/path.hpp>
#include "ComponentRegistry_new.h"
namespace mw {
class OpenALSound : public Sound {

protected:
	ALuint	buffer;
	ALuint  source;
	ALuint error;
	
	shared_ptr<Variable> amplitude;
	
public:

	OpenALSound(shared_ptr<Variable> amplitude);
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
	
	WavFileSound(const boost::filesystem::path &filename, shared_ptr<Variable> amplitude);

	~WavFileSound();
	
	
	virtual void announceSoundPlayed();
};

class WavFileSoundFactory : public ComponentFactory {
	
public:
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}
#endif
