//
//  AudioSourceNodeSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/28/22.
//

#ifndef AudioSourceNodeSound_hpp
#define AudioSourceNodeSound_hpp

#include "AudioEngineSound.hpp"

#include <AVFAudio/AVAudioSourceNode.h>
#include <AVFAudio/AVAudioFormat.h>


BEGIN_NAMESPACE_MW


class AudioSourceNodeSound : public AudioEngineSound {
    
public:
    static const std::string ENDED;
    
    explicit AudioSourceNodeSound(const ParameterValueMap &parameters);
    ~AudioSourceNodeSound();
    
protected:
    id<AVAudioMixing> load(AVAudioEngine *engine) override;
    virtual AVAudioFormat * getFormat() const = 0;  // Called by load()
    void unload(AVAudioEngine *engine) override;
    
    bool startPlaying(MWTime startTime) override;
    bool stopPlaying() override;
    bool beginPause() override;
    bool endPause() override;
    
    virtual bool renderFrames(AVAudioTime *firstFrameTime,
                              AVAudioFrameCount framesRequested,
                              AVAudioFrameCount &framesProvided,
                              std::size_t frameSize,
                              AudioBufferList *outputData) = 0;
    
private:
    bool renderCallback(BOOL &isSilence,
                        const AudioTimeStamp *timestamp,
                        AVAudioFrameCount frameCount,
                        AudioBufferList *outputData);
    
    const VariablePtr ended;
    const boost::shared_ptr<Clock> clock;
    
    double sampleRate;
    std::size_t frameSize;
    AVAudioSourceNode *sourceNode;
    
    AVAudioTime *currentStartTime;
    bool active;
    
};


END_NAMESPACE_MW


#endif /* AudioSourceNodeSound_hpp */
