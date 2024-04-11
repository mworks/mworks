//
//  MacOSAudioEngineManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 4/11/24.
//

#ifndef MacOSAudioEngineManager_hpp
#define MacOSAudioEngineManager_hpp

#include "AudioEngineManager.hpp"


BEGIN_NAMESPACE_MW


class MacOSAudioEngineManager : public AudioEngineSound::EngineManager {
    
public:
    MacOSAudioEngineManager();
    ~MacOSAudioEngineManager();
    
private:
    static void outputDeviceChanged(void *_engineManager,
                                    AudioUnit unit,
                                    AudioUnitPropertyID propertyID,
                                    AudioUnitScope scope,
                                    AudioUnitElement element);
    
    static bool setIOBufferDuration(AudioUnit outputAudioUnit);
    
    void setIOBufferDuration();
    
    AudioUnit outputAudioUnit;
    
};


END_NAMESPACE_MW


#endif /* MacOSAudioEngineManager_hpp */
