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
    ~AudioEngineSound();
    
    void play() override;
    void pause() override;
    void stop() override;
    
protected:
    using mutex_type = std::recursive_mutex;
    using lock_guard = std::lock_guard<mutex_type>;
    using unique_lock = std::unique_lock<mutex_type>;
    
    AVAudioEngine * getEngine(unique_lock &lock) const { return engineManager->getEngine(lock); }
    
    virtual bool startPlaying() = 0;
    virtual bool stopPlaying() = 0;
    virtual bool beginPause() = 0;
    virtual bool endPause() = 0;
    
    void didStopPlaying() { playing = paused = false; }
    
    mutex_type mutex;
    
private:
    void stateSystemCallback(const Datum &data, MWorksTime time);
    
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
    bool playing;
    bool paused;
    boost::shared_ptr<VariableCallbackNotification> stateSystemCallbackNotification;
    
};


END_NAMESPACE_MW


#endif /* AudioEngineSound_hpp */
