//
//  AudioPCMBufferSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/4/22.
//

#ifndef AudioPCMBufferSound_hpp
#define AudioPCMBufferSound_hpp

#include "AudioEngineSound.hpp"

#include <AVFoundation/AVAudioBuffer.h>
#include <AVFoundation/AVAudioPlayerNode.h>


BEGIN_NAMESPACE_MW


class AudioPCMBufferSound : public AudioEngineSound {
    
public:
    static const std::string LOOP;
    static const std::string REPEATS;
    static const std::string ENDED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioPCMBufferSound(const ParameterValueMap &parameters);
    ~AudioPCMBufferSound();
    
protected:
    id<AVAudioMixing> load(AVAudioEngine *engine) override;
    virtual AVAudioPCMBuffer * loadBuffer(AVAudioEngine *engine) = 0;
    void unload(AVAudioEngine *engine) override;
    
    bool startPlaying(MWTime startTime) override;
    bool stopPlaying() override;
    bool beginPause() override;
    bool endPause() override;
    
    void setCurrentAnnounceData(Datum::dict_value_type &announceData) const override;
    
private:
    void handlePlaybackCompleted();
    
    const VariablePtr loop;
    const VariablePtr repeats;
    const VariablePtr ended;
    
    const boost::shared_ptr<Clock> clock;
    
    AVAudioPlayerNode *playerNode;
    AVAudioPCMBuffer *buffer;
    bool currentLoop;
    long long currentRepeats;
    
    std::atomic_bool stopping;
    
};


END_NAMESPACE_MW


#endif /* AudioPCMBufferSound_hpp */
