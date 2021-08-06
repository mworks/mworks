//
//  AudioFileSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#ifndef AudioFileSound_hpp
#define AudioFileSound_hpp

#include "AudioEngineSound.hpp"

#include <AVFoundation/AVAudioBuffer.h>
#include <AVFoundation/AVAudioPlayerNode.h>


BEGIN_NAMESPACE_MW


class AudioFileSound : public AudioEngineSound {
    
public:
    static const std::string PATH;
    static const std::string AMPLITUDE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioFileSound(const ParameterValueMap &parameters);
    ~AudioFileSound();
    
private:
    bool startPlaying() override;
    bool stopPlaying() override;
    bool beginPause() override;
    bool endPause() override;
    
    const boost::filesystem::path path;
    const VariablePtr amplitude;
    
    AVAudioPCMBuffer *buffer;
    AVAudioPlayerNode *playerNode;
    
};


END_NAMESPACE_MW


#endif /* AudioFileSound_hpp */
