//
//  AudioPCMBufferSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/4/22.
//

#include "AudioPCMBufferSound.hpp"

#include <boost/scope_exit.hpp>

#include "MachUtilities.h"


BEGIN_NAMESPACE_MW


const std::string AudioPCMBufferSound::LOOP("loop");
const std::string AudioPCMBufferSound::REPEATS("repeats");
const std::string AudioPCMBufferSound::ENDED("ended");


void AudioPCMBufferSound::describeComponent(ComponentInfo &info) {
    AudioEngineSound::describeComponent(info);
    info.addParameter(LOOP, "NO");
    info.addParameter(REPEATS, "0");
    info.addParameter(ENDED, false);
}


AudioPCMBufferSound::AudioPCMBufferSound(const ParameterValueMap &parameters) :
    AudioEngineSound(parameters),
    loop(parameters[LOOP]),
    repeats(parameters[REPEATS]),
    ended(optionalVariable(parameters[ENDED])),
    clock(Clock::instance()),
    playerNode(nil),
    buffer(nil),
    currentLoop(false),
    currentRepeats(0),
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


id<AVAudioMixing> AudioPCMBufferSound::load(AVAudioEngine *engine) {
    buffer = loadBuffer(engine);
    
    [engine attachNode:playerNode];
    [engine connect:playerNode to:engine.mainMixerNode format:buffer.format];
    
    return playerNode;
}


void AudioPCMBufferSound::unload(AVAudioEngine *engine) {
    [engine disconnectNodeOutput:playerNode];
    [engine detachNode:playerNode];
    
    buffer = nil;
}


bool AudioPCMBufferSound::startPlaying(MWTime startTime) {
    // Reset the player node
    [playerNode stop];
    
    currentLoop = loop->getValue().getBool();
    currentRepeats = repeats->getValue().getInteger();
    
    AVAudioTime *when = nil;
    if (startTime > 0) {
        static const MachTimebase timebase;
        when = [AVAudioTime timeWithHostTime:timebase.nanosToAbsolute(startTime * 1000 /*us to ns*/ +
                                                                      clock->getSystemBaseTimeNS())];
    }
    
    if (currentLoop) {
        //
        // Schedule the buffer to loop indefinitely.  No completion handler
        // is needed, because playback can only be stopped explicitly.
        //
        [playerNode scheduleBuffer:buffer
                            atTime:when
                           options:AVAudioPlayerNodeBufferLoops
                 completionHandler:nil];
    } else {
        //
        // Schedule all play-throughs except the last
        //
        for (auto repeatCount = currentRepeats; repeatCount > 0; repeatCount--) {
            [playerNode scheduleBuffer:buffer
                                atTime:when
                               options:0
                     completionHandler:nil];
            when = nil;  // The next play-through should start immediately after this one
        }
        
        //
        // Schedule the final play-through
        //
        boost::weak_ptr<AudioPCMBufferSound> weakThis(component_shared_from_this<AudioPCMBufferSound>());
        auto completionHandler = [weakThis](AVAudioPlayerNodeCompletionCallbackType callbackType) {
            if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) {
                if (auto sharedThis = weakThis.lock()) {
                    sharedThis->handlePlaybackCompleted();
                }
            }
        };
        [playerNode scheduleBuffer:buffer
                            atTime:when
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


void AudioPCMBufferSound::setCurrentAnnounceData(Datum::dict_value_type &announceData) const {
    AudioEngineSound::setCurrentAnnounceData(announceData);
    if (currentLoop) {
        announceData[LOOP] = true;
    } else if (currentRepeats > 0) {
        announceData[REPEATS] = currentRepeats;
    }
}


void AudioPCMBufferSound::handlePlaybackCompleted() {
    if (stopping) {
        // If this handler is being called because stopPlaying() is executing, then do nothing,
        // because (1) we'll cause a deadlock if we try to acquire the lock while the thread
        // executing stopPlaying() holds it; (2) didStopPlaying() will be called after
        // stopPlaying() returns, so we don't need to call it here; and (3) playback was stopped
        // explicitly, rather than ending naturally, so "ended" shouldn't be set.
        return;
    }
    auto lock = acquireLock();
    didStopPlaying();
    if (ended && !(ended->getValue().getBool())) {
        ended->setValue(Datum(true));
    }
}


END_NAMESPACE_MW
