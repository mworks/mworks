//
//  ToneSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/10/22.
//

#include "ToneSound.hpp"

#include <Accelerate/Accelerate.h>


BEGIN_NAMESPACE_MW


const std::string ToneSound::FREQUENCY("frequency");
const std::string ToneSound::DURATION("duration");


void ToneSound::describeComponent(ComponentInfo &info) {
    AudioSourceNodeSound::describeComponent(info);
    info.setSignature("sound/tone");
    info.addParameter(FREQUENCY);
    info.addParameter(DURATION, "0");
    info.addParameter(ENDED, false);  // AudioSourceNodeSound doesn't add this
}


ToneSound::ToneSound(const ParameterValueMap &parameters) :
    AudioSourceNodeSound(parameters),
    frequency(parameters[FREQUENCY]),
    duration(parameters[DURATION]),
    format(nil),
    currentFrequency(0.0),
    currentDuration(0),
    currentPhase(0.0),
    currentPhaseIncrement(0.0)
{ }


ToneSound::~ToneSound() {
    @autoreleasepool {
        format = nil;
    }
}


id<AVAudioMixing> ToneSound::load(AVAudioEngine *engine) {
    const auto sampleRate = [engine.outputNode inputFormatForBus:0].sampleRate;
    format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:sampleRate channels:1];
    if (!format) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create format for tone audio samples");
    }
    return AudioSourceNodeSound::load(engine);
}


void ToneSound::unload(AVAudioEngine *engine) {
    AudioSourceNodeSound::unload(engine);
    format = nil;
}


bool ToneSound::startPlaying(MWTime startTime) {
    currentFrequency = frequency->getValue().getFloat();
    if (currentFrequency <= 0.0) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Tone frequency must be greater than zero");
        return false;
    }
    
    currentDuration = duration->getValue().getInteger();
    if (currentDuration > 0) {
        const auto numSamples = std::round(double(currentDuration) / 1e6 * format.sampleRate);
        if (numSamples > double(std::numeric_limits<decltype(framesRemaining)>::max())) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Requested tone duration requires too many audio samples");
            return false;
        }
        framesRemaining = numSamples;
    }
    
    currentPhase = 0.0;
    currentPhaseIncrement = 2.0 * M_PI * currentFrequency  / format.sampleRate;
    
    return AudioSourceNodeSound::startPlaying(startTime);
}


void ToneSound::setCurrentAnnounceData(Datum::dict_value_type &announceData) const {
    AudioSourceNodeSound::setCurrentAnnounceData(announceData);
    announceData[SOUND_TYPE] = "tone";
    announceData[FREQUENCY] = currentFrequency;
    if (haveCurrentDuration()) {
        announceData[DURATION] = currentDuration;
    }
}


bool ToneSound::renderFrames(AVAudioTime *firstFrameTime,
                             AVAudioFrameCount framesRequested,
                             AVAudioFrameCount &framesProvided,
                             std::size_t frameSize,
                             AudioBufferList *outputData)
{
    //
    // Check that the output data has the expected layout
    //
    {
        const auto actualNumBuffers = outputData->mNumberBuffers;
        if (actualNumBuffers != 1) {
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Output data has wrong number of buffers: expected 1, got %u",
                   actualNumBuffers);
            return false;
        }
    }
    {
        const auto actualNumChannels = outputData->mBuffers[0].mNumberChannels;
        if (actualNumChannels != 1) {
            merror(M_SYSTEM_MESSAGE_DOMAIN,
                   "Output data buffer has wrong number of channels: expected 1, got %u",
                   actualNumChannels);
            return false;
        }
    }
    
    //
    // Provide as many of the requested frames as possible
    //
    auto framesToCompute = framesRequested - framesProvided;
    if (haveCurrentDuration()) {
        framesToCompute = std::min(framesToCompute, framesRemaining);
    }
    if (framesToCompute > 0) {
        auto samples = static_cast<float *>(outputData->mBuffers[0].mData) + framesProvided;
        {
            const float initialValue = currentPhase;
            const float increment = currentPhaseIncrement;
            vDSP_vramp(&initialValue, &increment, samples, 1, framesToCompute);
        }
        {
            const int numElements = framesToCompute;
            vvsinf(samples, samples, &numElements);
        }
        
        framesProvided += framesToCompute;
        if (haveCurrentDuration()) {
            framesRemaining -= framesToCompute;
        }
        
        currentPhase = std::fmod(currentPhase + currentPhaseIncrement * double(framesToCompute), 2 * M_PI);
    }
    
    return true;
}


END_NAMESPACE_MW
