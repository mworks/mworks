//
//  AudioEngineSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#include "AudioEngineSound.hpp"


BEGIN_NAMESPACE_MW


AudioEngineSound::AudioEngineSound(const ParameterValueMap &parameters) :
    Sound(parameters),
    engineManager(getEngineManager()),
    playing(false),
    paused(false)
{
    auto callback = [this](const Datum &data, MWorksTime time) { stateSystemCallback(data, time); };
    stateSystemCallbackNotification = boost::make_shared<VariableCallbackNotification>(callback);
    if (auto stateSystemMode = state_system_mode) {
        stateSystemMode->addNotification(stateSystemCallbackNotification);
    }
}


AudioEngineSound::~AudioEngineSound() {
    stateSystemCallbackNotification->remove();
}


void AudioEngineSound::play() {
    lock_guard lock(mutex);
    
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


void AudioEngineSound::pause() {
    lock_guard lock(mutex);
    
    if (playing && !paused) {
        if (beginPause()) {
            paused = true;
        }
    }
}


void AudioEngineSound::stop() {
    lock_guard lock(mutex);
    
    if (playing) {
        if (stopPlaying()) {
            didStopPlaying();
        }
    }
}


void AudioEngineSound::stateSystemCallback(const Datum &data, MWorksTime time) {
    lock_guard lock(mutex);
    
    switch (data.getInteger()) {
        case RUNNING:
            break;
            
        case PAUSED:
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


AudioEngineSound::EngineManager::EngineManager() :
    engine(nil)
{
    @autoreleasepool {
        engine = [[AVAudioEngine alloc] init];
    }
    
    auto callback = [this](const Datum &data, MWorksTime time) { stateSystemCallback(data, time); };
    stateSystemCallbackNotification = boost::make_shared<VariableCallbackNotification>(callback);
    if (auto stateSystemMode = state_system_mode) {
        stateSystemMode->addNotification(stateSystemCallbackNotification);
    }
}


AudioEngineSound::EngineManager::~EngineManager() {
    stateSystemCallbackNotification->remove();
    
    @autoreleasepool {
        if (engine) {
            if (engine.running) {
                [engine stop];
            }
            engine = nil;
        }
    }
}


void AudioEngineSound::EngineManager::stateSystemCallback(const Datum &data, MWorksTime time) {
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
                
            case PAUSED:
                [[clang::fallthrough]];
                
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
