//
//  AudioEngineSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#include "AudioEngineSound.hpp"

#if !TARGET_OS_OSX
#  include "IOSAudioEngineManager.hpp"
#else
#  include "MacOSAudioEngineManager.hpp"
#endif


BEGIN_NAMESPACE_MW


const std::string AudioEngineSound::VOLUME("volume");
const std::string AudioEngineSound::PAN("pan");


void AudioEngineSound::describeComponent(ComponentInfo &info) {
    Sound::describeComponent(info);
    info.addParameter(VOLUME, "1.0");
    info.addParameterAlias(VOLUME, "amplitude");
    info.addParameter(PAN, "0.0");
}


AudioEngineSound::AudioEngineSound(const ParameterValueMap &parameters) :
    Sound(parameters),
    volume(parameters[VOLUME]),
    pan(parameters[PAN]),
    engineManager(getEngineManager()),
    mixer(nil),
    currentVolume(1.0),
    currentPan(0.0),
    loaded(false),
    running(false),
    playing(false),
    paused(false),
    pausedWithStateSystem(false)
{
    @autoreleasepool {
        // Install a callback to handle volume changes
        {
            auto callback = [this](const Datum &data, MWorksTime time) { volumeCallback(data, time); };
            volumeNotification = boost::make_shared<VariableCallbackNotification>(callback);
            volume->addNotification(volumeNotification);
        }
        
        // Install a callback to handle pan changes
        {
            auto callback = [this](const Datum &data, MWorksTime time) { panCallback(data, time); };
            panNotification = boost::make_shared<VariableCallbackNotification>(callback);
            pan->addNotification(panNotification);
        }
        
        // Install a callback to respond to state system mode changes
        {
            auto callback = [this](const Datum &data, MWorksTime time) { stateSystemModeCallback(data, time); };
            stateSystemModeNotification = boost::make_shared<VariableCallbackNotification>(callback);
            if (auto stateSystemMode = state_system_mode) {
                stateSystemMode->addNotification(stateSystemModeNotification);
            }
        }
    }
}


AudioEngineSound::~AudioEngineSound() {
    @autoreleasepool {
        stateSystemModeNotification->remove();
        panNotification->remove();
        volumeNotification->remove();
        
        mixer = nil;
    }
}


void AudioEngineSound::load() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (loaded) {
            return;
        }
        
        // Perform subclass-specific loading tasks
        {
            unique_lock engineLock;
            auto engine = engineManager->getEngine(engineLock);
            mixer = load(engine);
        }
        
        // Initialize volume and pan to their current values
        setCurrentVolume(volume->getValue());
        setCurrentPan(pan->getValue());
        
        loaded = true;
    }
}


void AudioEngineSound::unload() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (!loaded) {
            return;
        } else if (playing) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot unload sound while it is playing");
            return;
        }
        
        mixer = nil;
        
        // Perform subclass-specific unloading tasks
        {
            unique_lock engineLock;
            auto engine = engineManager->getEngine(engineLock);
            unload(engine);
        }
        
        loaded = false;
    }
}


void AudioEngineSound::play() {
    play(0);
}


void AudioEngineSound::play(MWTime startTime) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (!loaded) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot play: sound is not loaded");
        } else if (!running) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot play sound while experiment is not running");
        } else if (!playing) {
            // Although it isn't documented, it appears that the audio engine must have
            // been started in order for changes to volume and pan to take effect.
            // Therefore, always apply the current values immediately before playing, as
            // we know the audio engine will be started by then.
            applyCurrentVolume();
            applyCurrentPan();
            if (startPlaying(startTime)) {
                playing = true;
                announceAction(Action::Play, startTime);
            }
        } else if (paused) {
            if (startTime > 0) {
                mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Start time is ignored when resuming a paused sound");
            }
            if (endPause()) {
                paused = false;
                announceAction(Action::Resume);
            }
        }
    }
}


void AudioEngineSound::pause() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (!loaded) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot pause: sound is not loaded");
        } else if (!running) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot pause sound while experiment is not running");
        } else if (playing && !paused) {
            if (beginPause()) {
                paused = true;
                announceAction(Action::Pause);
            }
        }
    }
}


void AudioEngineSound::stop() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (!loaded) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot stop: sound is not loaded");
        } else if (!running) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot stop sound while experiment is not running");
        } else if (playing) {
            if (stopPlaying()) {
                didStopPlaying();
                announceAction(Action::Stop);
            }
        }
    }
}


auto AudioEngineSound::getEngineManager() -> boost::shared_ptr<EngineManager> {
    static boost::weak_ptr<EngineManager> weakManager;
    static mutex_type mutex;
    
    lock_guard lock(mutex);
    
    auto manager = weakManager.lock();
    if (!manager) {
#if !TARGET_OS_OSX
        manager = boost::make_shared<IOSAudioEngineManager>();
#else
        manager = boost::make_shared<MacOSAudioEngineManager>();
#endif
        weakManager = manager;
    }
    return manager;
}


void AudioEngineSound::volumeCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        setCurrentVolume(data);
    }
}


void AudioEngineSound::panCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        setCurrentPan(data);
    }
}


void AudioEngineSound::stateSystemModeCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        switch (data.getInteger()) {
            case RUNNING:
                running = true;
                if (playing && pausedWithStateSystem) {
                    if (!endPause()) {
                        // Still paused, but no longer with the state system
                        paused = true;
                    }
                    pausedWithStateSystem = false;
                }
                break;
                
            case PAUSED:
                running = false;
                if (playing && !paused && !pausedWithStateSystem) {
                    if (beginPause()) {
                        pausedWithStateSystem = true;
                    }
                }
                break;
                
            case STOPPING:
                // Stop on STOPPING, rather than IDLE, so that the sound stops playing
                // before the audio engine pauses
                running = false;
                if (playing) {
                    if (stopPlaying()) {
                        didStopPlaying();
                    }
                }
                break;
        }
    }
}


void AudioEngineSound::setCurrentVolume(const Datum &data) {
    auto value = data.getFloat();
    if (value < 0.0 || value > 1.0) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound volume must be between 0 and 1 (inclusive)");
    } else {
        currentVolume = value;
        if (playing) {
            applyCurrentVolume();
        }
    }
}


void AudioEngineSound::setCurrentPan(const Datum &data) {
    auto value = data.getFloat();
    if (value < -1.0 || value > 1.0) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound pan must be between -1 and 1 (inclusive)");
    } else {
        currentPan = value;
        if (playing) {
            applyCurrentPan();
        }
    }
}


const char * AudioEngineSound::getActionName(Action action) {
    switch (action) {
        case Action::Play:
            return "play";
        case Action::Pause:
            return "pause";
        case Action::Resume:
            return "resume";
        case Action::Stop:
            return "stop";
    }
}


void AudioEngineSound::announceAction(Action action, MWTime startTime) const {
    Datum::dict_value_type announceData;
    
    announceData[SOUND_NAME] = getTag();
    announceData[SOUND_ACTION] = getActionName(action);
    
    // Include detailed info only when playing
    if (action == Action::Play) {
        announceData[VOLUME] = currentVolume;
        
        if (currentPan != 0.0) {
            announceData[PAN] = currentPan;
        }
        
        if (startTime > 0) {
            announceData["start_time"] = startTime;
        }
        
        // Add subclass-specific data
        setCurrentAnnounceData(announceData);
    }
    
    announce(Datum(announceData));
}


END_NAMESPACE_MW
