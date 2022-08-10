//
//  ToneSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/10/22.
//

#ifndef ToneSound_hpp
#define ToneSound_hpp

#include "AudioPCMBufferSound.hpp"


BEGIN_NAMESPACE_MW


class ToneSound : public AudioPCMBufferSound {
    
public:
    static const std::string FREQUENCY;
    static const std::string DURATION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ToneSound(const ParameterValueMap &parameters);
    
private:
    AVAudioPCMBuffer * loadBuffer(AVAudioEngine *engine) override;
    
    const VariablePtr frequency;
    const VariablePtr duration;
    
};


END_NAMESPACE_MW


#endif /* ToneSound_hpp */
