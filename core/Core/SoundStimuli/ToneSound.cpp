//
//  ToneSound.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/10/22.
//

#include "ToneSound.hpp"


BEGIN_NAMESPACE_MW


const std::string ToneSound::FREQUENCY("frequency");
const std::string ToneSound::DURATION("duration");


void ToneSound::describeComponent(ComponentInfo &info) {
    AudioPCMBufferSound::describeComponent(info);
    info.setSignature("sound/tone");
    info.addParameter(FREQUENCY);
    info.addParameter(DURATION);
}


ToneSound::ToneSound(const ParameterValueMap &parameters) :
    AudioPCMBufferSound(parameters),
    frequency(parameters[FREQUENCY]),
    duration(parameters[DURATION]),
    currentFrequency(0.0),
    currentDuration(0)
{ }


static inline AVAudioFrameCount getNumSamples(MWTime duration, double sampleRate) {
    const auto numSamples = std::round(double(duration) / 1e6 * sampleRate);
    if (numSamples > double(std::numeric_limits<AVAudioFrameCount>::max())) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Requested tone duration requires too many audio samples");
    }
    return AVAudioFrameCount(numSamples);
}


AVAudioPCMBuffer * ToneSound::loadBuffer(AVAudioEngine *engine) {
    currentFrequency = frequency->getValue().getFloat();
    if (currentFrequency <= 0.0) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Tone frequency must be greater than zero");
    }
    
    currentDuration = duration->getValue().getInteger();
    if (currentDuration <= 0) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Tone duration must be greater than zero");
    }
    
    const auto sampleRate = [engine.outputNode inputFormatForBus:0].sampleRate;
    const auto numSamples = getNumSamples(currentDuration, sampleRate);
    
    auto format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:sampleRate channels:1];
    if (!format) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create format for tone audio samples");
    }
    
    auto buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:numSamples];
    if (!buffer) {
        throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Cannot create buffer for tone audio samples");
    }
    buffer.frameLength = numSamples;
    
    auto bufferData = buffer.floatChannelData[0];
    for (AVAudioFrameCount sampleIndex = 0; sampleIndex < numSamples; sampleIndex++) {
        bufferData[sampleIndex] = std::sin(2 * M_PI * currentFrequency * double(sampleIndex) / sampleRate);
    }
    
    return buffer;
}


void ToneSound::setCurrentAnnounceData(Datum::dict_value_type &announceData) const {
    AudioPCMBufferSound::setCurrentAnnounceData(announceData);
    announceData[SOUND_TYPE] = "tone";
    announceData[FREQUENCY] = currentFrequency;
    announceData[DURATION] = currentDuration;
}


END_NAMESPACE_MW
