/*
 *  StandardSounds.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "StandardSounds.h"
#include <boost/filesystem/operations.hpp>
#include "Announcers.h"
#include "StandardVariables.h"


BEGIN_NAMESPACE_MW


const std::string OpenALSound::AMPLITUDE("amplitude");


void OpenALSound::describeComponent(ComponentInfo &info) {
    Sound::describeComponent(info);
    info.addParameter(AMPLITUDE, "1.0");
}


OpenALSound::OpenALSound(const ParameterValueMap &parameters) :
    Sound(parameters),
    amplitude(parameters[AMPLITUDE])
{
	alGenBuffers(1, &buffer);
	
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(
			(boost::format("Failed to generate OpenAL buffer for sound stimulus (error code %d)")%error).str());
		return; 
	}
	 
	alGenSources(1, &source);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(
			(boost::format("Failed to generate OpenAL source for sound stimulus (error code %d)")% error).str());
		return; 
	}
}


void OpenALSound::play(){
//	mprintf("Playing sound!");

	alSourcef (source, AL_GAIN, (float)(amplitude->getValue()) );

	alSourcePlay(source);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to play OpenAL sound stimulus.  Error code",
			   (boost::format("%d") % error).str());
		return; 
	} else {
		announceSoundPlayed();
	}
}
	
void OpenALSound::announceSoundPlayed(){
	
 Datum data(M_DICTIONARY, 0);
	data.addElement(SOUND_NAME, getTag());
	
	announce(data);
}
	

void OpenALSound::pause(){
	alSourcePause(source);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to pause OpenAL sound stimulus.  Error code",
			   (boost::format("%d") % error).str());
		return; 
	}

}

void OpenALSound::stop(){
	alSourceStop(source);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to stop OpenAL sound stimulus.  Error code",
			   (boost::format("%d") % error).str());
		return; 
	}

	alSourceRewind(source);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
			   "Failed to rewind OpenAL sound stimulus.  Error code",
			   (boost::format("%d") % error).str());
		return; 
	}

}


const std::string WavFileSound::PATH("path");


void WavFileSound::describeComponent(ComponentInfo &info) {
    OpenALSound::describeComponent(info);
    info.setSignature("sound/wav_file");
    info.addParameter(PATH);
}


WavFileSound::WavFileSound(const ParameterValueMap &parameters) :
    OpenALSound(parameters)
{
    namespace bf = boost::filesystem;
    bf::path filename(parameters[PATH].as<bf::path>());
	
    if (bf::is_directory(filename)) {
        throw SimpleException("Path is a directory", filename.string());
    }
	
	format = 0;
	data = NULL;
	size = 0;
	freq = 0;
	
	path = filename.string();
					
	char *fn = new char[(filename.string().length() + 1)];
	memcpy(fn, filename.string().c_str(), filename.string().length() + 1);

	alutInternalLoadWAVFile(fn, &format, &data, &size, &freq);
	delete [] fn;
//	alutInternalLoadWAVFile(filename, &format, &data, &size, &freq);
	if ((error = alGetError()) != AL_NO_ERROR || data == NULL) { 
		
		throw SimpleException(
			(boost::format("Failed to load WAV file for sound stimulus (error code %d; filename: %s)") %
			   error % filename.string()).str());
		return; 
	} else {
		mprintf("Loaded wav file (%s; format=%d, size=%d, freq=%d",
					filename.string().c_str(), format, size, freq);
	}

	alBufferData(buffer, format, data, size, freq);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(
			(boost::format("Failed to load WAV into AL buffer (error code %d; data %d, format %d, size %d, freq %d)") %
			   error % data % format % size % freq).str());
		if(error == AL_INVALID_ENUM){
			throw SimpleException("Unknown audio format", (boost::format("%d") % format).str());
		}
		
		if(error == AL_INVALID_VALUE){
			throw SimpleException(
				(boost::format("Invalid buffer size (%d)") % size).str());
		}
		return; 
	}
	
	
	alSourcei (source, AL_BUFFER, buffer );
	alSourcef (source, AL_PITCH, 1.0f );
	alSourcef (source, AL_GAIN, 1.0f );
//		alSourcefv(source, AL_POSITION, SourcePos);
//		alSourcefv(source, AL_VELOCITY, SourceVel);
//		alSourcei (source, AL_LOOPING, AL_TRUE );
}

	
WavFileSound::~WavFileSound(){
	alutInternalUnloadWAV(format, data, size, freq);
	if ((error = alGetError()) != AL_NO_ERROR) { 
		throw SimpleException(
			(boost::format("Failed to unload wavfile for sound stimulus (error code %d)") %
			   error).str());
		return; 
	}

}


void WavFileSound::announceSoundPlayed(){
	
 Datum data(M_DICTIONARY, 2);
	data.addElement(SOUND_NAME, getTag());
	data.addElement(SOUND_FILENAME, path); 
	announce(data);
}


END_NAMESPACE_MW























