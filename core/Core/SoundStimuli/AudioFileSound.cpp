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
    AudioPCMBufferSound::describeComponent(info);
    info.setSignature("sound/audio_file");
    info.addParameter(PATH);
}


AudioFileSound::AudioFileSound(const ParameterValueMap &parameters) :
    AudioPCMBufferSound(parameters),
    path(variableOrText(parameters[PATH]))
{ }


AVAudioPCMBuffer * AudioFileSound::loadBuffer(AVAudioEngine *engine) {
    auto filePath = pathFromParameterValue(path).string();
    auto fileURL = [NSURL fileURLWithPath:@(filePath.c_str()) isDirectory:NO];
    NSError *error = nil;
    auto file = [[AVAudioFile alloc] initForReading:fileURL error:&error];
    if (!file) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                              "Cannot open audio file",
                              error.localizedDescription.UTF8String);
    }
    
    auto buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:file.processingFormat
                                                frameCapacity:file.length];
    if (!buffer) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create buffer for reading audio file");
    }
    if (![file readIntoBuffer:buffer error:&error]) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                              "Cannot read audio file in to buffer",
                              error.localizedDescription.UTF8String);
    }
    
    return buffer;
}


END_NAMESPACE_MW
