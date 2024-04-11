//
//  MacOSAudioEngineManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 4/11/24.
//

#include "MacOSAudioEngineManager.hpp"

#include <AudioToolbox/AudioToolbox.h>


BEGIN_NAMESPACE_MW


MacOSAudioEngineManager::MacOSAudioEngineManager() :
    outputAudioUnit(nullptr)
{
    @autoreleasepool {
        std::unique_lock<std::mutex> engineLock;
        auto engine = getEngine(engineLock);
        
        //
        // The fact that engine.outputNode.audioUnit is the AUHAL Audio Unit doesn't
        // seem to be documented, but it's easy to confirm, as we do below.
        //
        if (auto audioUnit = engine.outputNode.audioUnit) {
            auto audioComponent = AudioComponentInstanceGetComponent(audioUnit);
            AudioComponentDescription audioComponentDescription;
            if (audioComponent &&
                noErr == AudioComponentGetDescription(audioComponent, &audioComponentDescription) &&
                kAudioUnitType_Output == audioComponentDescription.componentType &&
                kAudioUnitSubType_HALOutput == audioComponentDescription.componentSubType)
            {
                outputAudioUnit = audioUnit;
            }
        }
        
        if (outputAudioUnit &&
            noErr != AudioUnitAddPropertyListener(outputAudioUnit,
                                                  kAudioOutputUnitProperty_CurrentDevice,
                                                  outputDeviceChanged,
                                                  this))
        {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot add listener for audio output device changes");
        }
        
        setIOBufferDuration();
    }
}


MacOSAudioEngineManager::~MacOSAudioEngineManager() {
    if (outputAudioUnit &&
        noErr != AudioUnitRemovePropertyListenerWithUserData(outputAudioUnit,
                                                             kAudioOutputUnitProperty_CurrentDevice,
                                                             outputDeviceChanged,
                                                             this))
    {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot remove listener for audio output device changes");
    }
}


void MacOSAudioEngineManager::outputDeviceChanged(void *_engineManager,
                                                  AudioUnit unit,
                                                  AudioUnitPropertyID propertyID,
                                                  AudioUnitScope scope,
                                                  AudioUnitElement element)
{
    auto &engineManager = *(static_cast<MacOSAudioEngineManager *>(_engineManager));
    if (engineManager.outputAudioUnit == unit &&
        kAudioOutputUnitProperty_CurrentDevice == propertyID &&
        kAudioUnitScope_Global == scope &&
        0 == element)
    {
        engineManager.setIOBufferDuration();
    }
}


bool MacOSAudioEngineManager::setIOBufferDuration(AudioUnit outputAudioUnit) {
    //
    // The technique used here for adjusting the I/O buffer size is described at
    // https://developer.apple.com/library/archive/technotes/tn2321/_index.html
    //
    
    if (!outputAudioUnit) {
        return false;
    }
    
    Float64 nominalSampleRate;
    UInt32 nominalSampleRateSize = sizeof(nominalSampleRate);
    if (noErr != AudioUnitGetProperty(outputAudioUnit,
                                      kAudioDevicePropertyNominalSampleRate,
                                      kAudioUnitScope_Global,
                                      0,
                                      &nominalSampleRate,
                                      &nominalSampleRateSize))
    {
        return false;
    }
    
    AudioValueRange bufferFrameSizeRange;
    UInt32 bufferFrameSizeRangeSize = sizeof(bufferFrameSizeRange);
    if (noErr != AudioUnitGetProperty(outputAudioUnit,
                                      kAudioDevicePropertyBufferFrameSizeRange,
                                      kAudioUnitScope_Global,
                                      0,
                                      &bufferFrameSizeRange,
                                      &bufferFrameSizeRangeSize))
    {
        return false;
    }
    
    UInt32 currentBufferFrameSize;
    UInt32 currentBufferFrameSizeSize = sizeof(currentBufferFrameSize);
    if (noErr != AudioUnitGetProperty(outputAudioUnit,
                                      kAudioDevicePropertyBufferFrameSize,
                                      kAudioUnitScope_Global,
                                      0,
                                      &currentBufferFrameSize,
                                      &currentBufferFrameSizeSize))
    {
        return false;
    }
    
    // Set the buffer frame size to the smallest power of 2 that produces a
    // buffer duration less than or equal to the target duration, subject to
    // the minimum buffer frame size
    const UInt32 newBufferFrameSize = std::max(bufferFrameSizeRange.mMinimum,
                                               std::pow(2.0, std::floor(std::log2(targetBufferDuration * nominalSampleRate))));
    if (newBufferFrameSize != currentBufferFrameSize) {
        if (noErr != AudioUnitSetProperty(outputAudioUnit,
                                          kAudioDevicePropertyBufferFrameSize,
                                          kAudioUnitScope_Global,
                                          0,
                                          &newBufferFrameSize,
                                          sizeof(newBufferFrameSize)))
        {
            return false;
        }
    }
    
    return true;
}


void MacOSAudioEngineManager::setIOBufferDuration() {
    if (!setIOBufferDuration(outputAudioUnit)) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                 "Cannot set audio I/O buffer duration; sounds may start later than requested");
    }
}


END_NAMESPACE_MW
