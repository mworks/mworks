//
//  ToneSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/10/22.
//

#ifndef ToneSound_hpp
#define ToneSound_hpp

#include "AudioSourceNodeSound.hpp"


BEGIN_NAMESPACE_MW


class ToneSound : public AudioSourceNodeSound {
    
public:
    static const std::string FREQUENCY;
    static const std::string DURATION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ToneSound(const ParameterValueMap &parameters);
    ~ToneSound();
    
private:
    id<AVAudioMixing> load(AVAudioEngine *engine) override;
    AVAudioFormat * getFormat() const override { return format; }
    void unload(AVAudioEngine *engine) override;
    
    bool startPlaying(MWTime startTime) override;
    
    void setCurrentAnnounceData(Datum::dict_value_type &announceData) const override;
    
    bool renderFrames(AVAudioTime *firstFrameTime,
                      AVAudioFrameCount framesRequested,
                      AVAudioFrameCount &framesProvided,
                      std::size_t frameSize,
                      AudioBufferList *outputData) override;
    
    bool haveCurrentDuration() const { return (currentDuration > 0); }
    
    const VariablePtr frequency;
    const VariablePtr duration;
    
    AVAudioFormat *format;
    
    double currentFrequency;
    long long currentDuration;
    AVAudioFrameCount framesRemaining;
    double currentPhase;
    double currentPhaseIncrement;
    
};


END_NAMESPACE_MW


#endif /* ToneSound_hpp */
