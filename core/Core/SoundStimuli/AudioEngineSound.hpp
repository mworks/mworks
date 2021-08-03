//
//  AudioEngineSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#ifndef AudioEngineSound_hpp
#define AudioEngineSound_hpp

#include <AVFoundation/AVAudioEngine.h>

#include <boost/noncopyable.hpp>

#include "Sound.h"


BEGIN_NAMESPACE_MW


class AudioEngineSound : public Sound, boost::noncopyable {
    
public:
    explicit AudioEngineSound(const ParameterValueMap &parameters);
    
protected:
    using mutex_type = std::mutex;
    using lock_guard = std::lock_guard<mutex_type>;
    using unique_lock = std::unique_lock<mutex_type>;
    
    AVAudioEngine * getEngine(unique_lock &lock) const { return engineManager->getEngine(lock); }
    
private:
    struct EngineManager : boost::noncopyable {
        EngineManager();
        ~EngineManager();
        AVAudioEngine * getEngine(unique_lock &lock) const {
            lock = unique_lock(mutex);
            return engine;
        }
    private:
        void stateSystemCallback(const Datum &data, MWorksTime time);
        AVAudioEngine *engine;
        boost::shared_ptr<VariableCallbackNotification> stateSystemCallbackNotification;
        mutable mutex_type mutex;
    };
    
    static boost::shared_ptr<EngineManager> getEngineManager();
    
    const boost::shared_ptr<EngineManager> engineManager;
    
};


END_NAMESPACE_MW


#endif /* AudioEngineSound_hpp */
