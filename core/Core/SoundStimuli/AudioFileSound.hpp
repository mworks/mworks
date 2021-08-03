//
//  AudioFileSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#ifndef AudioFileSound_hpp
#define AudioFileSound_hpp

#include "AudioEngineSound.hpp"

#include <AVFoundation/AVAudioFile.h>
#include <AVFoundation/AVAudioMixerNode.h>
#include <AVFoundation/AVAudioPlayerNode.h>


BEGIN_NAMESPACE_MW


class AudioFileSound : public AudioEngineSound {
    
public:
    static const std::string PATH;
    static const std::string AMPLITUDE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioFileSound(const ParameterValueMap &parameters);
    ~AudioFileSound();
    
    void play() override;
    void pause() override;
    void stop() override;
    
private:
    const boost::filesystem::path path;
    const VariablePtr amplitude;
    
    AVAudioFile *file;
    AVAudioPlayerNode *playerNode;
    AVAudioMixerNode *mixerNode;
    
    bool paused;
    
};


END_NAMESPACE_MW


#endif /* AudioFileSound_hpp */
