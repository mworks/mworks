//
//  AudioFileSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#include "AudioFileSound.hpp"

#include <AVFoundation/AVAudioMixerNode.h>


BEGIN_NAMESPACE_MW


const std::string AudioFileSound::PATH("path");
const std::string AudioFileSound::AMPLITUDE("amplitude");


void AudioFileSound::describeComponent(ComponentInfo &info) {
    AudioEngineSound::describeComponent(info);
    
    info.setSignature("sound/audio_file");
    
    info.addParameter(PATH);
    info.addParameter(AMPLITUDE, "1.0");
}


AudioFileSound::AudioFileSound(const ParameterValueMap &parameters) :
    AudioEngineSound(parameters),
    path(pathFromParameterValue(variableOrText(parameters[PATH]))),
    amplitude(parameters[AMPLITUDE]),
    file(nil),
    playerNode(nil)
{
    @autoreleasepool {
        auto fileURL = [NSURL fileURLWithPath:@(path.string().c_str()) isDirectory:NO];
        NSError *error = nil;
        file = [[AVAudioFile alloc] initForReading:fileURL error:&error];
        if (!file) {
            throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                                  "Cannot open audio file",
                                  error.localizedDescription.UTF8String);
        }
        
        playerNode = [[AVAudioPlayerNode alloc] init];
        
        unique_lock engineLock;
        auto engine = getEngine(engineLock);
        [engine attachNode:playerNode];
        [engine connect:playerNode to:engine.mainMixerNode format:nil];
    }
}


AudioFileSound::~AudioFileSound() {
    @autoreleasepool {
        playerNode = nil;
        file = nil;
    }
}


bool AudioFileSound::startPlaying() {
    // Reset the player node
    [playerNode stop];
    
    boost::weak_ptr<AudioFileSound> weakThis(component_shared_from_this<AudioFileSound>());
    auto completionHandler = [weakThis](AVAudioPlayerNodeCompletionCallbackType callbackType) {
        if (callbackType == AVAudioPlayerNodeCompletionDataPlayedBack) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                sharedThis->didStopPlaying();
            }
        }
    };
    [playerNode scheduleFile:file
                      atTime:nil
      completionCallbackType:AVAudioPlayerNodeCompletionDataPlayedBack
           completionHandler:completionHandler];
    
    // TODO: support dynamic volume changes
    playerNode.volume = amplitude->getValue().getFloat();
    
    [playerNode play];
    return true;
}


bool AudioFileSound::stopPlaying() {
    [playerNode stop];
    return true;
}


bool AudioFileSound::beginPause() {
    [playerNode pause];
    return true;
}


bool AudioFileSound::endPause() {
    [playerNode play];
    return true;
}


END_NAMESPACE_MW
