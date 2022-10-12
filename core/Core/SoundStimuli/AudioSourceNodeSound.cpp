//
//  AudioSourceNodeSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/28/22.
//

#include "AudioSourceNodeSound.hpp"

#include "MachUtilities.h"


BEGIN_NAMESPACE_MW


const std::string AudioSourceNodeSound::ENDED("ended");


AudioSourceNodeSound::AudioSourceNodeSound(const ParameterValueMap &parameters) :
    AudioEngineSound(parameters),
    ended(optionalVariable(parameters[ENDED])),
    clock(Clock::instance()),
    sampleRate(0.0),
    frameSize(0),
    sourceNode(nil),
    currentStartTime(nil),
    active(false)
{ }


AudioSourceNodeSound::~AudioSourceNodeSound() {
    @autoreleasepool {
        currentStartTime = nil;
        sourceNode = nil;
    }
}


id<AVAudioMixing> AudioSourceNodeSound::load(AVAudioEngine *engine) {
    auto format = getFormat();
    
    sampleRate = format.sampleRate;
    
    switch (format.commonFormat) {
        case AVAudioPCMFormatFloat32:
            frameSize = sizeof(float);
            break;
        case AVAudioPCMFormatFloat64:
            frameSize = sizeof(double);
            break;
        case AVAudioPCMFormatInt16:
            frameSize = sizeof(std::int16_t);
            break;
        case AVAudioPCMFormatInt32:
            frameSize = sizeof(std::int32_t);
            break;
        default:
            throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Audio data has an unknown format");
    }
    if (format.interleaved) {
        frameSize *= format.channelCount;
    }
    
    auto renderBlock = [this](BOOL *isSilence,
                              const AudioTimeStamp *timestamp,
                              AVAudioFrameCount frameCount,
                              AudioBufferList *outputData)
    {
        static_assert(noErr == 0);
        return OSStatus(!renderCallback(*isSilence, timestamp, frameCount, outputData));
    };
    sourceNode = [[AVAudioSourceNode alloc] initWithFormat:format renderBlock:renderBlock];
    
    [engine attachNode:sourceNode];
    [engine connect:sourceNode to:engine.mainMixerNode format:format];
    
    return sourceNode;
}


void AudioSourceNodeSound::unload(AVAudioEngine *engine) {
    [engine disconnectNodeOutput:sourceNode];
    [engine detachNode:sourceNode];
    
    sourceNode = nil;
}


bool AudioSourceNodeSound::startPlaying(MWTime startTime) {
    if (startTime <= 0) {
        currentStartTime = nil;
    } else {
        static const MachTimebase timebase;
        auto startHostTime = timebase.nanosToAbsolute(startTime * 1000 /*us to ns*/ + clock->getSystemBaseTimeNS());
        auto startTimestamp = [AVAudioTime timeWithHostTime:startHostTime].audioTimeStamp;
        currentStartTime = [AVAudioTime timeWithAudioTimeStamp:&startTimestamp sampleRate:sampleRate];
    }
    active = true;
    return true;
}


bool AudioSourceNodeSound::stopPlaying() {
    active = false;
    return true;
}


bool AudioSourceNodeSound::beginPause() {
    active = false;
    return true;
}


bool AudioSourceNodeSound::endPause() {
    active = true;
    return true;
}


bool AudioSourceNodeSound::renderCallback(BOOL &isSilence,
                                          const AudioTimeStamp *timestamp,
                                          AVAudioFrameCount framesRequested,
                                          AudioBufferList *outputData)
{
    auto lock = acquireLock();
    
    //
    // Check that the output buffers have the expected size
    //
    for (std::size_t i = 0; i < outputData->mNumberBuffers; i++) {
        const auto expectedDataSize = framesRequested * frameSize;
        const auto actualDataSize = outputData->mBuffers[i].mDataByteSize;
        if (expectedDataSize != actualDataSize) {
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Output data buffer has incorrect size: expected %lu bytes, got %u bytes",
                   expectedDataSize,
                   actualDataSize);
            return false;
        }
    }
    
    AVAudioFrameCount framesProvided = 0;
    
    if (active) {
        auto firstFrameTime = [AVAudioTime timeWithAudioTimeStamp:timestamp sampleRate:sampleRate];
        if (!(firstFrameTime.hostTimeValid && firstFrameTime.sampleTimeValid)) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create valid audio time from output timestamp");
            return false;
        }
        
        //
        // Fill any frames preceding the start time with silence
        //
        if (currentStartTime) {
            auto extrapolatedStartTime = [currentStartTime extrapolateTimeFromAnchor:firstFrameTime];
            if (!extrapolatedStartTime) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot extrapolate start time from first frame time");
                return false;
            }
            
            AVAudioFrameCount framesToFill = 0;
            auto framesUntilStartTime = extrapolatedStartTime.sampleTime - firstFrameTime.sampleTime;
            if (framesUntilStartTime < 0) {
                mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                         "Sound %s is starting %g ms later than requested",
                         getTag().c_str(),
                         double(-framesUntilStartTime) / sampleRate * 1000.0);
            } else {
                framesToFill = std::min(framesRequested, AVAudioFrameCount(framesUntilStartTime));
            }
            
            if (framesToFill > 0) {
                for (std::size_t i = 0; i < outputData->mNumberBuffers; i++) {
                    std::memset(outputData->mBuffers[i].mData, 0, framesToFill * frameSize);
                }
                framesProvided += framesToFill;
            }
            
            if (framesProvided < framesRequested) {
                // The frame corresponding to the start time lies within the current request (or was part of
                // a previous request, which we missed because we were paused).  Set currentStartTime to nil,
                // so that we don't bother checking it again.
                currentStartTime = nil;
            } else {
                // The frame corresponding to the start time has not yet been requested.  We've filled the
                // full request with silence, so we're done.
                isSilence = YES;
                return true;
            }
        }
        
        //
        // Attempt to fill the remainder of the request with actual data
        //
        if (!renderFrames(firstFrameTime, framesRequested, framesProvided, frameSize, outputData)) {
            return false;
        }
    }
    
    //
    // Fill any remaining frames with silence
    //
    if (framesProvided < framesRequested) {
        for (std::size_t i = 0; i < outputData->mNumberBuffers; i++) {
            std::memset(static_cast<std::uint8_t *>(outputData->mBuffers[i].mData) + framesProvided * frameSize,
                        0,
                        (framesRequested - framesProvided) * frameSize);
        }
    }
    
    //
    // Handle the case where all frames are silence
    //
    if (framesProvided == 0) {
        isSilence = YES;
        if (active) {
            didStopPlaying();
            if (ended && !(ended->getValue().getBool())) {
                ended->setValue(Datum(true));
            }
            active = false;
        }
    }
    
    return true;
}


END_NAMESPACE_MW
