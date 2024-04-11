//
//  AudioEngineManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 4/11/24.
//

#include "AudioEngineManager.hpp"

#include <AVFAudio/AVAudioPlayerNode.h>


BEGIN_NAMESPACE_MW


AudioEngineSound::EngineManager::EngineManager() :
    engine(nil),
    dummyNode(nil)
{
    @autoreleasepool {
        engine = [[AVAudioEngine alloc] init];
        
        // Attempting to prepare the engine with no nodes attached crashes the application.
        // Therefore, we attach and connect a dummy node, which is never played, to prevent
        // this.
        dummyNode = [[AVAudioPlayerNode alloc] init];
        [engine attachNode:dummyNode];
        [engine connect:dummyNode to:engine.mainMixerNode format:nil];
        
        // Calling prepare seems to be important.  On macOS, if we don't call prepare, but
        // we do set the audio I/O buffer duration, then switching audio output devices
        // sometimes breaks audio output completely.  (This doesn't happen if we *don't*
        // set the buffer duration, so prepare must do some internal configuration that
        // impacts the results of the buffer duration change.)
        [engine prepare];
        
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
        
        if (engine.running) {
            [engine stop];
        }
        
        [engine disconnectNodeOutput:dummyNode];
        [engine detachNode:dummyNode];
        dummyNode = nil;
        
        engine = nil;
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


END_NAMESPACE_MW
