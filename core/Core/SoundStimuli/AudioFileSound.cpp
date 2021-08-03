//
//  AudioFileSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#include "AudioFileSound.hpp"


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
    playerNode(nil),
    mixerNode(nil),
    paused(false)
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
        mixerNode = [[AVAudioMixerNode alloc] init];
        
        unique_lock engineLock;
        auto engine = getEngine(engineLock);
        [engine attachNode:playerNode];
        [engine attachNode:mixerNode];
        [engine connect:playerNode to:mixerNode format:nil];
        [engine connect:mixerNode to:engine.mainMixerNode format:nil];
    }
}


AudioFileSound::~AudioFileSound() {
    @autoreleasepool {
        mixerNode = nil;
        playerNode = nil;
        file = nil;
    }
}


void AudioFileSound::play() {
    lock_guard lock(mutex);
    
    if (!playerNode.playing) {
        if (!paused) {
            [playerNode scheduleFile:file atTime:nil completionHandler:nil];
            // TODO: support dynamic volume changes
            mixerNode.outputVolume = amplitude->getValue().getFloat();
        }
        [playerNode play];
        paused = false;
    }
}


void AudioFileSound::pause() {
    lock_guard lock(mutex);
    
    if (playerNode.playing) {
        [playerNode pause];
        paused = true;
    }
}


void AudioFileSound::stop() {
    lock_guard lock(mutex);
    
    if (playerNode.playing || paused) {
        [playerNode stop];
        paused = false;
    }
}


END_NAMESPACE_MW
