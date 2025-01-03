//
//  AudioFileSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/21.
//

#ifndef AudioFileSound_hpp
#define AudioFileSound_hpp

#include "AudioPCMBufferSound.hpp"


BEGIN_NAMESPACE_MW


class AudioFileSound : public AudioPCMBufferSound {
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioFileSound(const ParameterValueMap &parameters);
    
private:
    AVAudioPCMBuffer * loadBuffer(AVAudioEngine *engine) override;
    void setCurrentAnnounceData(Datum::dict_value_type &announceData) const override;
    
    const VariablePtr path;
    std::string currentPath;
    std::string currentFileHash;
    
};


END_NAMESPACE_MW


#endif /* AudioFileSound_hpp */
