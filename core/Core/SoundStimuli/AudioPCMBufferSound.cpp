//
//  AudioPCMBufferSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/4/22.
//

#include "AudioPCMBufferSound.hpp"


BEGIN_NAMESPACE_MW


const std::string AudioPCMBufferSound::LOOP("loop");


void AudioPCMBufferSound::describeComponent(ComponentInfo &info) {
    AudioEngineSound::describeComponent(info);
    info.addParameter(LOOP, "NO");
}


AudioPCMBufferSound::AudioPCMBufferSound(const ParameterValueMap &parameters) :
    AudioEngineSound(parameters),
    loop(parameters[LOOP]),
    playerNode(nil),
    buffer(nil)
{
    @autoreleasepool {
        playerNode = [[AVAudioPlayerNode alloc] init];
    }
}


AudioPCMBufferSound::~AudioPCMBufferSound() {
    @autoreleasepool {
        buffer = nil;
        playerNode = nil;
    }
}


void AudioPCMBufferSound::load(AVAudioEngine *engine, AVAudioMixerNode *mixerNode) {
    AudioEngineSound::load(engine, mixerNode);
    
    buffer = loadBuffer();
    
    [engine attachNode:playerNode];
    [engine connect:playerNode to:mixerNode format:buffer.format];
}


bool AudioPCMBufferSound::startPlaying() {
    // Reset the player node
    [playerNode stop];
    
    if (loop->getValue().getBool()) {
        [playerNode scheduleBuffer:buffer
                            atTime:nil
                           options:AVAudioPlayerNodeBufferLoops
                 completionHandler:nil];
    } else {
        boost::weak_ptr<AudioPCMBufferSound> weakThis(component_shared_from_this<AudioPCMBufferSound>());
        auto completionHandler = [weakThis](AVAudioPlayerNodeCompletionCallbackType callbackType) {
            if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) {
                if (auto sharedThis = weakThis.lock()) {
                    auto lock = sharedThis->acquireLock();
                    sharedThis->didStopPlaying();
                }
            }
        };
        [playerNode scheduleBuffer:buffer
                            atTime:nil
                           options:0
            completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack
                 completionHandler:completionHandler];
    }
    
    [playerNode play];
    return true;
}


bool AudioPCMBufferSound::stopPlaying() {
    [playerNode stop];
    return true;
}


bool AudioPCMBufferSound::beginPause() {
    [playerNode pause];
    return true;
}


bool AudioPCMBufferSound::endPause() {
    [playerNode play];
    return true;
}


END_NAMESPACE_MW
