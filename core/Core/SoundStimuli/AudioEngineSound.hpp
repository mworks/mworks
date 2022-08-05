//
//  AudioEngineSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#ifndef AudioEngineSound_hpp
#define AudioEngineSound_hpp

#include <AVFoundation/AVAudioEngine.h>
#include <AVFoundation/AVAudioMixerNode.h>

#include <boost/noncopyable.hpp>

#include "Sound.h"


BEGIN_NAMESPACE_MW


class AudioEngineSound : public Sound, boost::noncopyable {
    
public:
    static const std::string AMPLITUDE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioEngineSound(const ParameterValueMap &parameters);
    ~AudioEngineSound();
    
    void load() override;
    
    void play() override;
    void pause() override;
    void stop() override;
    
protected:
    using lock_guard = std::lock_guard<std::mutex>;
    
    lock_guard acquireLock() const { return lock_guard(mutex); }
    void didStopPlaying() { playing = paused = pausedWithStateSystem = false; }
    
    virtual void load(AVAudioEngine *engine, AVAudioMixerNode *mixerNode) { }
    
    virtual bool startPlaying() = 0;
    virtual bool stopPlaying() = 0;
    virtual bool beginPause() = 0;
    virtual bool endPause() = 0;
    
private:
    using mutex_type = lock_guard::mutex_type;
    using unique_lock = std::unique_lock<mutex_type>;
    
    void amplitudeCallback(const Datum &data, MWorksTime time);
    void setCurrentAmplitude(const Datum &data);
    void stateSystemModeCallback(const Datum &data, MWorksTime time);
    
    struct EngineManager : boost::noncopyable {
        EngineManager();
        ~EngineManager();
        AVAudioEngine * getEngine(unique_lock &lock) const {
            lock = unique_lock(mutex);
            return engine;
        }
    private:
        void stateSystemModeCallback(const Datum &data, MWorksTime time);
        AVAudioEngine *engine;
        boost::shared_ptr<VariableNotification> stateSystemModeNotification;
        mutable mutex_type mutex;
    };
    
    static boost::shared_ptr<EngineManager> getEngineManager();
    
    const VariablePtr amplitude;
    const boost::shared_ptr<EngineManager> engineManager;
    
    AVAudioMixerNode *mixerNode;
    bool loaded;
    bool running;
    bool playing;
    bool paused;
    bool pausedWithStateSystem;
    
    boost::shared_ptr<VariableNotification> amplitudeNotification;
    boost::shared_ptr<VariableNotification> stateSystemModeNotification;
    
    mutable mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* AudioEngineSound_hpp */
