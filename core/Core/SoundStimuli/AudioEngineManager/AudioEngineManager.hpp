//
//  AudioEngineManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 4/11/24.
//

#ifndef AudioEngineManager_hpp
#define AudioEngineManager_hpp

#include "AudioEngineSound.hpp"


BEGIN_NAMESPACE_MW


class AudioEngineSound::EngineManager : boost::noncopyable {
    
public:
    EngineManager();
    virtual ~EngineManager();
    
    AVAudioEngine * getEngine(unique_lock &lock) const {
        lock = unique_lock(mutex);
        return engine;
    }
    
protected:
    static constexpr auto targetBufferDuration = 0.001;  // 1ms
    
private:
    void stateSystemModeCallback(const Datum &data, MWorksTime time);
    
    AVAudioEngine *engine;
    AVAudioNode *dummyNode;
    boost::shared_ptr<VariableNotification> stateSystemModeNotification;
    mutable mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* AudioEngineManager_hpp */
