//
//  AudioFileSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#include "AudioFileSound.hpp"

#include <AVFoundation/AVAudioFile.h>


BEGIN_NAMESPACE_MW


const std::string AudioFileSound::PATH("path");


void AudioFileSound::describeComponent(ComponentInfo &info) {
    AudioEngineSound::describeComponent(info);
    
    info.setSignature("sound/audio_file");
    
    info.addParameter(PATH);
}


AudioFileSound::AudioFileSound(const ParameterValueMap &parameters) :
    AudioEngineSound(parameters),
    path(variableOrText(parameters[PATH])),
    playerNode(nil),
    buffer(nil)
{
    @autoreleasepool {
        playerNode = [[AVAudioPlayerNode alloc] init];
    }
}


AudioFileSound::~AudioFileSound() {
    @autoreleasepool {
        buffer = nil;
        playerNode = nil;
    }
}


void AudioFileSound::load(AVAudioEngine *engine, AVAudioMixerNode *mixerNode) {
    AudioEngineSound::load(engine, mixerNode);
    
    auto filePath = pathFromParameterValue(path).string();
    auto fileURL = [NSURL fileURLWithPath:@(filePath.c_str()) isDirectory:NO];
    NSError *error = nil;
    auto file = [[AVAudioFile alloc] initForReading:fileURL error:&error];
    if (!file) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                              "Cannot open audio file",
                              error.localizedDescription.UTF8String);
    }
    
    buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:file.processingFormat
                                           frameCapacity:file.length];
    if (!buffer) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create buffer for reading audio file");
    }
    if (![file readIntoBuffer:buffer error:&error]) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                              "Cannot read audio file in to buffer",
                              error.localizedDescription.UTF8String);
    }
    
    [engine attachNode:playerNode];
    [engine connect:playerNode to:mixerNode format:buffer.format];
}


bool AudioFileSound::startPlaying() {
    // Reset the player node
    [playerNode stop];
    
    boost::weak_ptr<AudioFileSound> weakThis(component_shared_from_this<AudioFileSound>());
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
