//
//  LegacySounds.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#ifndef LegacySounds_hpp
#define LegacySounds_hpp

#include "Sound.h"
#include "OpenALContextManager.h"
#include <boost/filesystem/path.hpp>
#include "ComponentInfo.h"


BEGIN_NAMESPACE_MW


class LegacySound : public Sound {
    
public:
    explicit LegacySound(const ParameterValueMap &parameters);
    ~LegacySound();
    
    void play() override {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Attempting to play an empty base-class sound");
    }
    
    void pause() override {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Attempting to pause an empty base-class sound");
    }
    
    void stop() override {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Attempting to stop an empty base-class sound");
    }
    
    void stateSystemCallback(const Datum &data, MWorksTime time);
    
protected:
    bool isPlaying;
    bool isPaused;
    
private:
    boost::shared_ptr<VariableCallbackNotification> stateSystemCallbackNotification;
    
};


class OpenALSound : public LegacySound {

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


#endif /* LegacySounds_hpp */
