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
    virtual AVAudioPCMBuffer * loadBuffer() const = 0;
    
private:
    void load(AVAudioEngine *engine, AVAudioMixerNode *mixerNode) override final;
    
    bool startPlaying() override;
    bool stopPlaying() override;
    bool beginPause() override;
    bool endPause() override;
    
    void handlePlaybackCompleted();
    
    const VariablePtr loop;
    const VariablePtr repeats;
    const VariablePtr ended;
    
    AVAudioPlayerNode *playerNode;
    AVAudioPCMBuffer *buffer;
    
    std::atomic_bool stopping;
    
};


END_NAMESPACE_MW


#endif /* AudioPCMBufferSound_hpp */
