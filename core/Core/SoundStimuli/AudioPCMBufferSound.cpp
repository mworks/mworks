//
//  AudioPCMBufferSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/4/22.
//

#include "AudioPCMBufferSound.hpp"

#include "MachUtilities.h"


BEGIN_NAMESPACE_MW


const std::string AudioPCMBufferSound::LOOP("loop");
const std::string AudioPCMBufferSound::REPEATS("repeats");


void AudioPCMBufferSound::describeComponent(ComponentInfo &info) {
    AudioSourceNodeSound::describeComponent(info);
    info.addParameter(LOOP, "NO");
    info.addParameter(REPEATS, "0");
    info.addParameter(ENDED, false);  // AudioSourceNodeSound doesn't add this
}


AudioPCMBufferSound::AudioPCMBufferSound(const ParameterValueMap &parameters) :
    AudioSourceNodeSound(parameters),
    loop(parameters[LOOP]),
    repeats(parameters[REPEATS]),
    buffer(nil),
    currentLoop(false),
    currentRepeats(0),
    nextFrame(0),
    remainingRepeats(0)
{ }


AudioPCMBufferSound::~AudioPCMBufferSound() {
    @autoreleasepool {
        buffer = nil;
    }
}


id<AVAudioMixing> AudioPCMBufferSound::load(AVAudioEngine *engine) {
    buffer = loadBuffer(engine);
    return AudioSourceNodeSound::load(engine);
}


void AudioPCMBufferSound::unload(AVAudioEngine *engine) {
    AudioSourceNodeSound::unload(engine);
    buffer = nil;
}


bool AudioPCMBufferSound::startPlaying(MWTime startTime) {
    currentLoop = loop->getValue().getBool();
    currentRepeats = repeats->getValue().getInteger();
    
    nextFrame = 0;
    remainingRepeats = currentRepeats;
    
    return AudioSourceNodeSound::startPlaying(startTime);
}


void AudioPCMBufferSound::setCurrentAnnounceData(Datum::dict_value_type &announceData) const {
    AudioSourceNodeSound::setCurrentAnnounceData(announceData);
    if (currentLoop) {
        announceData[LOOP] = true;
    } else if (currentRepeats > 0) {
        announceData[REPEATS] = currentRepeats;
    }
}


bool AudioPCMBufferSound::renderFrames(AVAudioTime *firstFrameTime,
                                       AVAudioFrameCount framesRequested,
                                       AVAudioFrameCount &framesProvided,
                                       std::size_t frameSize,
                                       AudioBufferList *outputData)
{
    auto sourceData = buffer.audioBufferList;
    
    //
    // Check that the output data and source data have the same layout
    //
    {
        const auto expectedNumBuffers = sourceData->mNumberBuffers;
        const auto actualNumBuffers = outputData->mNumberBuffers;
        if (expectedNumBuffers != actualNumBuffers) {
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Output data has wrong number of buffers: expected %u, got %u",
                   expectedNumBuffers,
                   actualNumBuffers);
            return false;
        }
    }
    for (std::size_t i = 0; i < outputData->mNumberBuffers; i++) {
        const auto expectedNumChannels = sourceData->mBuffers[i].mNumberChannels;
        const auto actualNumChannels = outputData->mBuffers[i].mNumberChannels;
        if (expectedNumChannels != actualNumChannels) {
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Output data buffer has wrong number of channels: expected %u, got %u",
                   expectedNumChannels,
                   actualNumChannels);
            return false;
        }
    }
    
    //
    // Provide as many of the requested frames as possible
    //
    do {
        auto framesToCopy = std::min(framesRequested - framesProvided, buffer.frameLength - nextFrame);
        if (framesToCopy > 0) {
            for (std::size_t i = 0; i < outputData->mNumberBuffers; i++) {
                std::memcpy(static_cast<std::uint8_t *>(outputData->mBuffers[i].mData) + framesProvided * frameSize,
                            static_cast<std::uint8_t *>(sourceData->mBuffers[i].mData) + nextFrame * frameSize,
                            framesToCopy * frameSize);
            }
            framesProvided += framesToCopy;
            nextFrame += framesToCopy;
        }
        if (framesProvided < framesRequested) {
            if (currentLoop) {
                nextFrame = 0;
                continue;
            } else if (remainingRepeats > 0) {
                nextFrame = 0;
                remainingRepeats--;
                continue;
            }
        }
    } while (false);
    
    return true;
}


END_NAMESPACE_MW
