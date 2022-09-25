//
//  AudioPCMBufferSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/4/22.
//

#ifndef AudioPCMBufferSound_hpp
#define AudioPCMBufferSound_hpp

#include "AudioSourceNodeSound.hpp"

#include <AVFAudio/AVAudioBuffer.h>


BEGIN_NAMESPACE_MW


class AudioPCMBufferSound : public AudioSourceNodeSound {
    
public:
    static const std::string LOOP;
    static const std::string REPEATS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioPCMBufferSound(const ParameterValueMap &parameters);
    ~AudioPCMBufferSound();
    
protected:
    id<AVAudioMixing> load(AVAudioEngine *engine) override;
    virtual AVAudioPCMBuffer * loadBuffer(AVAudioEngine *engine) = 0;  // Called by load()
    AVAudioFormat * getFormat() const override { return buffer.format; }
    void unload(AVAudioEngine *engine) override;
    
    bool startPlaying(MWTime startTime) override;
    
    void setCurrentAnnounceData(Datum::dict_value_type &announceData) const override;
    
    bool renderFrames(AVAudioTime *firstFrameTime,
                      AVAudioFrameCount framesRequested,
                      AVAudioFrameCount &framesProvided,
                      std::size_t frameSize,
                      AudioBufferList *outputData) override;
    
private:
    const VariablePtr loop;
    const VariablePtr repeats;
    
    AVAudioPCMBuffer *buffer;
    
    bool currentLoop;
    long long currentRepeats;
    
    AVAudioFrameCount nextFrame;
    long long remainingRepeats;
    
};


END_NAMESPACE_MW


#endif /* AudioPCMBufferSound_hpp */
