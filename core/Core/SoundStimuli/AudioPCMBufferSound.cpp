//
//  AudioPCMBufferSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/4/22.
//

#include "AudioPCMBufferSound.hpp"

#include <boost/scope_exit.hpp>


BEGIN_NAMESPACE_MW


const std::string AudioPCMBufferSound::LOOP("loop");
const std::string AudioPCMBufferSound::REPEATS("repeats");


void AudioPCMBufferSound::describeComponent(ComponentInfo &info) {
    AudioEngineSound::describeComponent(info);
    info.addParameter(LOOP, "NO");
    info.addParameter(REPEATS, "0");
}


AudioPCMBufferSound::AudioPCMBufferSound(const ParameterValueMap &parameters) :
    AudioEngineSound(parameters),
    loop(parameters[LOOP]),
    repeats(parameters[REPEATS]),
    playerNode(nil),
    buffer(nil),
    stopping(false)
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
        //
        // Schedule the buffer to loop indefinitely.  No completion handler
        // is needed, because playback can only be stopped explicitly.
        //
        [playerNode scheduleBuffer:buffer
                            atTime:nil
                           options:AVAudioPlayerNodeBufferLoops
                 completionHandler:nil];
    } else {
        //
        // Schedule all play-throughs except the last
        //
        auto repeatCount = repeats->getValue().getInteger();
        while (repeatCount > 0) {
            [playerNode scheduleBuffer:buffer
                                atTime:nil
                               options:0
                     completionHandler:nil];
            repeatCount--;
        }
        
        //
        // Schedule the final play-through
        //
        boost::weak_ptr<AudioPCMBufferSound> weakThis(component_shared_from_this<AudioPCMBufferSound>());
        auto completionHandler = [weakThis](AVAudioPlayerNodeCompletionCallbackType callbackType) {
            if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) {
                if (auto sharedThis = weakThis.lock()) {
                    if (sharedThis->stopping) {
                        // If this handler is being called because stopPlaying() is executing, then do nothing,
                        // because (1) we'll cause a deadlock if we try to acquire the lock while the thread
                        // executing stopPlaying() holds it, and (2) didStopPlaying() will be called after
                        // stopPlaying() returns, so we don't need to call it here.
                    } else {
                        auto lock = sharedThis->acquireLock();
                        sharedThis->didStopPlaying();
                    }
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
    stopping = true;
    BOOST_SCOPE_EXIT(&stopping) {
        stopping = false;
    } BOOST_SCOPE_EXIT_END
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
