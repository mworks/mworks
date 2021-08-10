//
//  AudioEngineSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#include "AudioEngineSound.hpp"


BEGIN_NAMESPACE_MW


const std::string AudioEngineSound::AMPLITUDE("amplitude");


void AudioEngineSound::describeComponent(ComponentInfo &info) {
    Sound::describeComponent(info);
    info.addParameter(AMPLITUDE, "1.0");
}


AudioEngineSound::AudioEngineSound(const ParameterValueMap &parameters) :
    Sound(parameters),
    amplitude(parameters[AMPLITUDE]),
    engineManager(getEngineManager()),
    mixerNode(nil),
    playing(false),
    paused(false)
{
    @autoreleasepool {
        // Create and connect the mixer node
        {
            mixerNode = [[AVAudioMixerNode alloc] init];
            unique_lock engineLock;
            auto engine = getEngine(engineLock);
            [engine attachNode:mixerNode];
            [engine connect:mixerNode to:engine.mainMixerNode format:nil];
        }
        
        // Install a callback to handle amplitude changes
        {
            auto callback = [this](const Datum &data, MWorksTime time) { amplitudeCallback(data, time); };
            amplitudeNotification = boost::make_shared<VariableCallbackNotification>(callback);
            amplitude->addNotification(amplitudeNotification);
        }
        
        // Initialize amplitude to the current value
        setCurrentAmplitude(amplitude->getValue());
        
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
        amplitudeNotification->remove();
        
        mixerNode = nil;
    }
}


void AudioEngineSound::play() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (!playing) {
            if (startPlaying()) {
                playing = true;
            }
        } else if (paused) {
            if (endPause()) {
                paused = false;
            }
        }
    }
}


void AudioEngineSound::pause() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (playing && !paused) {
            if (beginPause()) {
                paused = true;
            }
        }
    }
}


void AudioEngineSound::stop() {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        if (playing) {
            if (stopPlaying()) {
                didStopPlaying();
            }
        }
    }
}


void AudioEngineSound::amplitudeCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        setCurrentAmplitude(data);
    }
}


void AudioEngineSound::setCurrentAmplitude(const Datum &data) {
    auto value = data.getFloat();
    if (value < 0.0 || value > 1.0) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound amplitude must be between 0 and 1 (inclusive)");
    } else {
        mixerNode.outputVolume = value;
    }
}


void AudioEngineSound::stateSystemModeCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        switch (data.getInteger()) {
            case RUNNING:
                if (playing && paused) {
                    if (endPause()) {
                        paused = false;
                    }
                }
                break;
                
            case PAUSED:
                if (playing && !paused) {
                    if (beginPause()) {
                        paused = true;
                    }
                }
                break;
                
            case IDLE:
                if (playing) {
                    if (stopPlaying()) {
                        didStopPlaying();
                    }
                }
                break;
        }
    }
}


AudioEngineSound::EngineManager::EngineManager() :
    engine(nil)
{
    @autoreleasepool {
        engine = [[AVAudioEngine alloc] init];
        
        auto callback = [this](const Datum &data, MWorksTime time) { stateSystemModeCallback(data, time); };
        stateSystemModeNotification = boost::make_shared<VariableCallbackNotification>(callback);
        if (auto stateSystemMode = state_system_mode) {
            stateSystemMode->addNotification(stateSystemModeNotification);
        }
    }
}


AudioEngineSound::EngineManager::~EngineManager() {
    @autoreleasepool {
        stateSystemModeNotification->remove();
        
        if (engine) {
            if (engine.running) {
                [engine stop];
            }
            engine = nil;
        }
    }
}


void AudioEngineSound::EngineManager::stateSystemModeCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        switch (data.getInteger()) {
            case RUNNING:
                if (!engine.running) {
                    NSError *error = nil;
                    if (![engine startAndReturnError:&error]) {
                        merror(M_SYSTEM_MESSAGE_DOMAIN,
                               "Cannot start audio engine: %s",
                               error.localizedDescription.UTF8String);
                    }
                }
                break;
                
            case IDLE:
                if (engine.running) {
                    [engine pause];
                }
                break;
        }
    }
}


auto AudioEngineSound::getEngineManager() -> boost::shared_ptr<EngineManager> {
    static boost::weak_ptr<EngineManager> weakManager;
    static mutex_type mutex;
    
    lock_guard lock(mutex);
    
    auto manager = weakManager.lock();
    if (!manager) {
        manager = boost::make_shared<EngineManager>();
        weakManager = manager;
    }
    return manager;
}


END_NAMESPACE_MW
